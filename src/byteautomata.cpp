#pragma once
#include "byteautomata.h"
#include "paula.h"

namespace paula
{
constexpr INT
	TREE_ARRAY_SIZE = 1024,
	MAX_STATES = 32,
	MAX_DEPTH = 32,
	BA_BUFFER_SIZE = 1024,
	CFG_MAX_NAME_LENGTH = 128,
	
	LINE_UNDEFINED = 1,
	LINE_ASSIGNMENT = 2,
	LINE_CALL = 3;


const CHAR
	*letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	*numbers = "1234567890",
	*whiteSpace = " \t", // "\n\r",
	*linebreak = "\n\r",
	//*expressionBreak = ",;",
	*blockStart = "(", // "([{",
	*blockEnd = ")"; //")]}";


ByteAutomata::ByteAutomata(Paula& p) :
	paula(p),
	tree(TREE_ARRAY_SIZE),
	tr(MAX_STATES * 256),
	buffer(BA_BUFFER_SIZE),
	tmp(BA_BUFFER_SIZE),
	treeStack(MAX_DEPTH),
	stateNames(MAX_STATES)
{
	ok = true;
	currentInput = 0;
	currentState = 0;
	stateCounter = 0;
	actionCounter = 0;
	inputByte = 0;
	index = 0;
	lineNumber = 0;
	stayNextStep = false;
	running = false;

	tr.fill((BYTE)0xff);

	defineTransitions();
}
ByteAutomata::~ByteAutomata() { }
void ByteAutomata::print ()
{
	for (INT i = 0; i <= stateCounter; i++)
	{
		//vrbout()<<("state: ")<<(i)<<std::endl;
		for (INT n = 0; n < 256; n++)
		{
			BYTE foo = tr[(i * 256) + n];
			if (foo == 0xff) std::cout<<(".");
			else std::cout<<(foo);
		}
		std::cout<<("")<<std::endl;
	}
}
BYTE ByteAutomata::addState (const CHAR * stateName)
{
	stateCounter++;
	stateNames[stateCounter] = stateName;
	return stateCounter;
}
void ByteAutomata::transition (BYTE state, const CHAR * input, void (* action)(ByteAutomata*))
{
	BYTE actionIndex = 0;
	if (action != 0)
	{
		actionIndex = addAction(action);
	}
	BYTE * bytes = (BYTE *)input;
	INT i = 0;
	while (bytes[i] != 0)
	{
		tr[(state * 256) + bytes[i]] = actionIndex;
		i++;
	}
	//DEBUG(VR("New Transition added: id ")X(actionIndex)XO);
}
void ByteAutomata::fillTransition (BYTE state, void (* action)(ByteAutomata*))
{
	BYTE actionIndex = 0;
	if (action != 0) actionIndex = addAction(action);
	for (INT i=0; i<256; i++)
	{
		tr[(state * 256) + i] = actionIndex;
	}
	//DEBUG(VR("New Transition filled: id ")X(actionIndex)XO);
}
BYTE ByteAutomata::addAction (void (* action)(ByteAutomata*))
{
	actionCounter++;
	actions[actionCounter] = action;
	return actionCounter;
}
void ByteAutomata::next (BYTE nextState)
{
	// transition to a next state
	lastStart = getIndex();
	currentState = nextState;
	std::cout<<("Next state: ")<<(stateNames[(INT)currentState])<<std::endl;
}
INT ByteAutomata::getIndex ()
{
	return index;
}
BYTE ByteAutomata::getInputByte ()
{
	return inputByte;
}
void ByteAutomata::stay ()
{
	// same input byte on next step
	ASSERT(!stayNextStep, "'stay' is called twice");
	stayNextStep = true;
}

void ByteAutomata::run (IInputStream & input)
{
	currentState = stateStart;

	inputByte = 0;
	index = 0;
	lineNumber = 1;
	stayNextStep = false;
	running = true;

	indentation = 0;

	newLine();

	while ((!input.end() || stayNextStep) && running && ok)
	{
		if (!stayNextStep)
		{
			index ++;
			inputByte = input.read();
			buffer[index % BA_BUFFER_SIZE] = inputByte;
			if (inputByte == '\n') lineNumber++;
		}
		else
		{
			stayNextStep = false;
		}
		LOG("[ ");
		LOGCHAR(inputByte);
		LOGLINE(" ]"<<" state: "<<stateNames[(INT)currentState]);
		running = step(inputByte);
	}
	printTreeStack();
	if (treeStackTop != 0)
	{
		CHECK(false, Error::PARENTHESIS);
	}
	if (!stayNextStep)
	{	index++;
	running = step('\n');
	}

	LOGLINE("end run");

	tree.print();
}
bool ByteAutomata::step (BYTE input)
{
	currentInput = input;
	INT index = (currentState * 256) + input;
	BYTE actionIndex = tr[index];
	if (actionIndex == 0) return true; // stay on same state and do nothing else
	if (actionIndex == 0xff||actionIndex < 0)
	{
		LOG("unexpected character: ");
		LOGCHAR(input);
		LOGLINE("");
		ASSERT(false, "parse error");
		ok = false;
		return false; // end
	}
	void (* act)(ByteAutomata*) = actions[actionIndex];
	if (act == 0)
	{
		ASSERT(false, "invalid action index");
	}
	act(this);
	return true;
}
void ByteAutomata::printError ()
{
	std::cout<<("ERROR: parser state [")<<(stateNames[(INT)currentState])<<("]")<<std::endl;
	std::cout<<("Line ")<<(lineNumber)<<(": \"");
	// print nearby code
	INT start = index-1;
	while (start > 0 && index - start < BA_BUFFER_SIZE && (BYTE)buffer[start % BA_BUFFER_SIZE] != '\n')
	{
		start --;
	}
	while (++start < index)
	{
		std::cout<<((BYTE)(buffer[start % BA_BUFFER_SIZE]));
	}
	std::cout<<("\"")<<std::endl;
}

//--------------------------------------------------------------
// state machine code
//--------------------------------------------------------------

INT ByteAutomata::currentParent()
{
	return treeStack[treeStackTop];
}
void ByteAutomata::printTreeStack()
{
	if (treeStackTop < 1) LOG(" tree stack, top: "<<treeStackTop);
	for(INT i=0; i<=treeStackTop; i++)
	{
		//LOG(" ["<<treeStack[i]<<":"<<(tree.getTag(treeStack[i]))<<"] ");
		LOG(tree.treeTypeName(tree.getTag(treeStack[i]))<<" > ");
	}
	LOGLINE("");
}
void ByteAutomata::pushTree(INT subtreeType)
{
	ASSERT(tree.isSubtreeTag(subtreeType),"");

	INT newParent = tree.addSubtree(currentParent(), subtreeType);
	treeStackTop++;
	treeStack[treeStackTop] = newParent;

	LOG("pushTree: ");
	printTreeStack();
}
void ByteAutomata::popTree()
{
	treeStackTop--;

	LOG("popTree [top="<<treeStackTop<<"]:");
	printTreeStack();
}
void ByteAutomata::startAssignment ()
{
	LOGLINE("startAssignment");
	ASSERT(lineType = LINE_UNDEFINED, "");
	lineType = LINE_ASSIGNMENT;
	tree.init(NODE_ASSIGNMENT);
	treeStack[0] = 0;
	treeStackTop = 0;
}
void ByteAutomata::startFunction ()
{
	LOGLINE("startFunction");
	ASSERT(lineType = LINE_UNDEFINED, "");
	lineType = LINE_CALL;
	tree.init(NODE_COMMAND);
	treeStack[0] = 0;
	treeStackTop = 0;
}
void ByteAutomata::addTextToken()
{
	addToken(NODE_TEXT);
}
void ByteAutomata::addToken(INT tokenType)
{
	LOGLINE("addToken, type: "<<tokenType);
	printTreeStack();
	// if parent is subtree, start a new expr

	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_SUBTREE)
	{
		LOGLINE("addToken: new expr");
		pushTree(NODE_EXPR);
	}

	// TODO: parse and save number

	tree.addText(currentParent(), buffer, lastStart, getIndex());
}
void ByteAutomata::comma()
{
	printTreeStack();
	LOGLINE("comma");
	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_EXPR)
	{
		// pop from expr first

		popTree();
	}
}
void ByteAutomata::lineBreak()
{
	LOGLINE("lineBreak: execute command");
	tree.print();
	paula.execute(indentation, tree);
	newLine();
	//if (currentBlock != 0) (*currentBlock).numChildren ++;
	//lastStart = -1;
	//addExpr();
}
void ByteAutomata::startBlock()
{
	LOGLINE("addBlock");
	pushTree(NODE_SUBTREE);
	//BYTE inputByte = (*automata).getInputByte();
	//INT blockType = 0;
	//if (inputByte == '(') blockType = NODE_PARENTHESIS;
	//else if (inputByte == '[') blockType = NODE_SQUARE_BRACKETS;
	//else if (inputByte == '{') blockType = NODE_CURLY_BRACKETS;
	//else { {if (!(false)) EXIT("unhandled block start: " << inputByte)}; }
	//lastStart = -1;
	//std::string tmp123("<BLOCK>");
	//MNode* block = new MNode(currentExpr, blockType, tmp123);
	//if (currentToken == 0) (*currentExpr).child = block;
	//else (*currentToken).next = block;
	//(*currentExpr).numChildren++;
	//currentBlock = block;
	//MNode* expr = new MNode(currentBlock, 0, "<EXPR>");
	//(*currentBlock).child = expr;
	//currentExpr = expr;
	//currentToken = 0;
}
void ByteAutomata::endBlock()
{
	LOGLINE("endBlock");
	tree.print();
	
	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_EXPR)
	{
		// pop from expr first
		LOGLINE("pop expr");
		printTreeStack();
		popTree();
	}
	
	popTree();
	//{if (!((currentBlock != 0))) EXIT("unexpected block end")};
	//BYTE inputByte = (*automata).getInputByte();
	//// Check that block-end BYTEacter is the right one.
	//// The 'type' is block start/end BYTEacter's ASCII code.
	//if ((*currentBlock).type == NODE_PARENTHESIS ) { {if (!(inputByte == ')')) EXIT("invalid block end; parenthesis was expected")};}
	//else if ((*currentBlock).type == NODE_SQUARE_BRACKETS ) { {if (!(inputByte == ']')) EXIT("invalid block end; square bracket was expected")};}
	//else if ((*currentBlock).type == NODE_CURLY_BRACKETS ) { {if (!(inputByte == '}')) EXIT("invalid block end; curly bracket was expected")};}
	//else { {if (!(false)) EXIT("unhandled block end: " << inputByte)}; }
	//lastStart = -1;
	//currentToken = currentBlock;
	//currentExpr = (*currentToken).parent;
	//currentBlock = (*currentExpr).parent;
}
void ByteAutomata::newLine()
{
	indentation = 0;
	lineType = LINE_UNDEFINED;
	tree.clear();
	
	// tree node to the top of the stack
	
	treeStackTop = 0;
	tree.clear();
	next(stateStart);
}
void ByteAutomata::startExpr(BYTE firstState)
{
	LOGLINE("startExpr: indentation="<<indentation);
	stay();
	next(firstState);
}
void ByteAutomata::defineTransitions()
{
	/*
			states: start (ind.) → name [first name] → [post name]
				1) assign → read expr
				2) function → read args ()
	*/

	stateStart = addState("start");
	stateSpace = addState("space");
	stateName = addState("name");
	stateFirstName = addState("first name");
	statePostName = addState("post name");
	stateName = addState("name");
	stateNumber = addState("number");
	
	transition(stateStart, "\t", [](ByteAutomata*ba)					{ ba->indentation++; });
	transition(stateStart, letters, [](ByteAutomata*ba)					{ ba->startExpr(ba->stateFirstName); });
	transition(stateSpace, linebreak, [](ByteAutomata*ba)				{ ba->newLine(); });

	transition(stateFirstName, letters, 0);
	transition(stateFirstName, whiteSpace, [](ByteAutomata*ba)			{ ba->addTextToken(); ba->next(ba->stateSpace); });
	transition(stateFirstName, ":", [](ByteAutomata*ba)					{ ba->startAssignment(); ba->addTextToken(); ba->next(ba->stateSpace); });
	transition(stateFirstName, blockStart, [](ByteAutomata*ba)			{ ba->startFunction(); ba->addTextToken(); ba->stay(); ba->next(ba->stateSpace); });

	transition(stateName, letters, 0);
	transition(stateName, whiteSpace, [](ByteAutomata*ba)				{ ba->addTextToken(); ba->next(ba->stateSpace); });
	transition(stateName, blockStart, [](ByteAutomata*ba)				{ ba->addTextToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, blockEnd, [](ByteAutomata*ba)					{ ba->addTextToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, linebreak, [](ByteAutomata*ba)				{ ba->addTextToken(); ba->lineBreak(); });

	transition(stateSpace, whiteSpace, 0);
	transition(stateSpace, letters, [](ByteAutomata*ba)					{ ba->next(ba->stateName); });
	transition(stateSpace, numbers, [](ByteAutomata*ba)					{ ba->next(ba->stateNumber); });
	transition(stateSpace, linebreak, [](ByteAutomata*ba)				{ ba->lineBreak(); });
	transition(stateSpace, blockStart, [](ByteAutomata*ba)				{ ba->startBlock();});
	transition(stateSpace, blockEnd, [](ByteAutomata*ba)				{ ba->endBlock();});

	transition(stateNumber, numbers, 0);
	transition(stateNumber, whiteSpace, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->next(ba->stateSpace); });
	transition(stateNumber, ",", [](ByteAutomata*ba)					{ ba->addToken(NODE_INTEGER); ba->comma(); ba->next(ba->stateSpace); });
	transition(stateNumber, blockEnd, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateNumber, linebreak, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->lineBreak(); });
}
}