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
	*operators = "+-*/",
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
ByteAutomata::~ByteAutomata()
{
}
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



//////////////// TRANSITION FUNCTIONS



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
}
void ByteAutomata::fillTransition (BYTE state, void (* action)(ByteAutomata*))
{
	BYTE actionIndex = 0;
	if (action != 0) actionIndex = addAction(action);
	for (INT i=0; i<256; i++)
	{
		tr[(state * 256) + i] = actionIndex;
	}
}
BYTE ByteAutomata::addAction (void (* action)(ByteAutomata*))
{
	actionCounter++;
	actions[actionCounter] = action;
	return actionCounter;
}


//////////////// STATE MACHINE CONTROL


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
		CHECK(false, PARENTHESIS);
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
		LOGERROR("unexpected character: ");
		LOGCHAR(input);
		LOGLINE("");
		CHECK(false, UNEXPECTED_CHARACTER);
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
		LOG(tree.treeTypeName(tree.getType(treeStack[i]))<<" > ");
	}
	LOGLINE("");
}
void ByteAutomata::pushTree(INT subtreeType)
{
	CHECK(tree.isSubtreeTag(subtreeType), PARSE_ERROR);

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
	CHECK(lineType = LINE_UNDEFINED, PARSE_ERROR);
	lineType = LINE_ASSIGNMENT;
	tree.init(NODE_ASSIGNMENT);
	treeStack[0] = 0;
	treeStackTop = 0;
}
void ByteAutomata::startFunction ()
{
	LOGLINE("startFunction");
	CHECK(lineType = LINE_UNDEFINED, PARSE_ERROR);
	lineType = LINE_CALL;
	tree.init(NODE_COMMAND);
	treeStack[0] = 0;
	treeStackTop = 0;
}
INT ByteAutomata::parseInt(Array<BYTE>& src, INT i, INT lastByte)
{
	INT value = 0;
	INT zeros = 0;
	while(i < lastByte)
	{
		// TODO negative
		BYTE b = src[i];
		ASSERT(b >= '0' && b <= '9', "");
		if (value > 0) value *= 10;
		value += b - '0';
		i++;
	}
	return value;
}
void ByteAutomata::prepareAddToken()
{
	printTreeStack();

	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_SUBTREE)
	{
		// parent is a subtree, start a new expr
		LOGLINE("addToken: new expr");
		pushTree(NODE_EXPR);
	}
}
void ByteAutomata::addOperatorToken()
{
	LOGLINE("addOperatorToken");
	prepareAddToken();
	tree.addOperatorNode(currentParent(), (char)inputByte);
}
void ByteAutomata::addIntegerToken()
{
	LOGLINE("addIntegerToken");
	INT value = parseInt(buffer, lastStart, getIndex());
	prepareAddToken();
	tree.addInt(currentParent(), value);
}
void ByteAutomata::addNameToken()
{
	addLiteralToken(NODE_NAME);
}
void ByteAutomata::addTextToken()
{
	addLiteralToken(NODE_TEXT);
}
void ByteAutomata::addLiteralToken(INT nodeType)
{
	LOGLINE("addLiteralToken: "<<nodeType);
	prepareAddToken();
	tree.addText(currentParent(), buffer, lastStart, getIndex(), nodeType);
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
}
void ByteAutomata::startBlock()
{
	LOGLINE("addBlock");
	pushTree(NODE_SUBTREE);
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
	transition(stateFirstName, whiteSpace, [](ByteAutomata*ba)			{ ba->addNameToken(); ba->next(ba->stateSpace); });
	transition(stateFirstName, ":", [](ByteAutomata*ba)					{ ba->startAssignment(); ba->addNameToken(); ba->next(ba->stateSpace); });
	transition(stateFirstName, blockStart, [](ByteAutomata*ba)			{ ba->startFunction(); ba->addNameToken(); ba->stay(); ba->next(ba->stateSpace); });

	transition(stateName, letters, 0);
	transition(stateName, whiteSpace, [](ByteAutomata*ba)				{ ba->addNameToken(); ba->next(ba->stateSpace); });
	transition(stateNumber, operators, [](ByteAutomata*ba)				{ ba->addNameToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, blockStart, [](ByteAutomata*ba)				{ ba->addNameToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, blockEnd, [](ByteAutomata*ba)					{ ba->addNameToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, linebreak, [](ByteAutomata*ba)				{ ba->addNameToken(); ba->lineBreak(); });

	transition(stateSpace, whiteSpace, 0);
	transition(stateSpace, operators, [](ByteAutomata*ba)				{ ba->addOperatorToken(); });
	transition(stateSpace, letters, [](ByteAutomata*ba)					{ ba->next(ba->stateName); });
	transition(stateSpace, numbers, [](ByteAutomata*ba)					{ ba->next(ba->stateNumber); });
	transition(stateSpace, linebreak, [](ByteAutomata*ba)				{ ba->lineBreak(); });
	transition(stateSpace, blockStart, [](ByteAutomata*ba)				{ ba->startBlock(); });
	transition(stateSpace, blockEnd, [](ByteAutomata*ba)				{ ba->endBlock(); });
	transition(stateSpace, ",", [](ByteAutomata*ba)						{ ba->comma(); });

	transition(stateNumber, numbers, 0);
	transition(stateNumber, whiteSpace, [](ByteAutomata*ba)				{ ba->addIntegerToken(); ba->next(ba->stateSpace); });
	transition(stateNumber, operators, [](ByteAutomata*ba)				{ ba->addIntegerToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateNumber, ",", [](ByteAutomata*ba)					{ ba->addIntegerToken(); ba->comma(); ba->next(ba->stateSpace); });
	transition(stateNumber, blockEnd, [](ByteAutomata*ba)				{ ba->addIntegerToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateNumber, linebreak, [](ByteAutomata*ba)				{ ba->addIntegerToken(); ba->lineBreak(); });
}
}