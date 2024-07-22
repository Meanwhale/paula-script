#pragma once
#include "byteautomata.h"
#include "paula.h"

#define BA_CHECK(x,e) { if (!(x)) { error = &e; return; } }

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
	input(nullptr),
	error(NO_ERROR),
	tree(TREE_ARRAY_SIZE),
	tr(MAX_STATES * 256),
	buffer(BA_BUFFER_SIZE),
	tmp(BA_BUFFER_SIZE),
	treeStack(MAX_DEPTH),
	stateNames(MAX_STATES)
{
	currentInput = 0;
	currentState = 0;
	stateCounter = 0;
	actionCounter = 0;
	bufferIndex = 0;
	lineNumber = 0;
	stayNextStep = false;

	tr.fill((BYTE)0xff);

	defineTransitions();
}
ByteAutomata::~ByteAutomata()
{
}
//void ByteAutomata::print ()
//{
//	for (INT i = 0; i <= stateCounter; i++)
//	{
//		//vrbout()<<("state: ")<<(i)<<std::endl;
//		for (INT n = 0; n < 256; n++)
//		{
//			BYTE foo = tr[(i * 256) + n];
//			if (foo == 0xff) std::cout<<(".");
//			else std::cout<<(foo);
//		}
//		std::cout<<("")<<std::endl;
//	}
//}



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

const Error* ByteAutomata::getError()
{
	return error;
}
void ByteAutomata::printError ()
{
	ERR.print("ERROR: parser state [").print(stateNames[(INT)currentState]).print("]").endl();
	ERR.print("Line ").print(lineNumber).print(": \"");
	// print nearby code
	INT start = bufferIndex-1;
	while (start > 0 && bufferIndex - start < BA_BUFFER_SIZE && (BYTE)buffer[start] != '\n')
	{
		start --;
	}
	while (++start < bufferIndex)
	{
		ERR.print((BYTE)(buffer[start]));
	}
	ERR.print("\"").endl();
}

//////////////// STATE MACHINE CONTROL


void ByteAutomata::next (BYTE nextState)
{
	// transition to a next state
	lastStart = getIndex();
	currentState = nextState;
	LOG.print("Next state: ").print(stateNames[(INT)currentState]).endl();
}
INT ByteAutomata::getIndex ()
{
	return bufferIndex;
}
void ByteAutomata::stay ()
{
	// same input byte on next step
	ASSERT(!stayNextStep, "'stay' is called twice");
	stayNextStep = true;
}
void ByteAutomata::init (IInputStream * _input)
{
	input = _input;
	currentState = stateStart;
	bufferIndex = 0;
	lineNumber = 1;
	stayNextStep = false;
	indentation = 0;
	error = NO_ERROR;
	newLine();
}
bool ByteAutomata::running()
{
	if (error != NO_ERROR)
	{
		ERR.print("ByteAutomata: ERROR ").print(error).endl();
		closeInput();
		return false;
	}
	return input != nullptr;
}
void ByteAutomata::closeInput()
{
	if (input != nullptr)
	{
		input->close();
		input = nullptr;
	}
}
void ByteAutomata::step()
{
	// get input byte
	BYTE inputByte;

	// TODO: read from buffer if loop

	if (!stayNextStep)
	{
		// check end here in case input was shut down between steps.
		if (input->end())
		{
			input->close();
			input = nullptr;
			return;
		}
		bufferIndex ++;
		inputByte = input->read();
		buffer[bufferIndex] = inputByte;
	}
	else
	{
		inputByte = currentInput;
		stayNextStep = false;
	}
	LOG.print("[ ").printCharSymbol(inputByte).print(" ] state: ").print(stateNames[(INT)currentState]).endl();

	step(inputByte);
}
void ByteAutomata::step(BYTE b)
{
	// handle input byte

	currentInput = b;
	INT functionIndex = (currentState * 256) + currentInput;
	BYTE actionIndex = tr[functionIndex];
	if (actionIndex == 0)
	{
		return; // stay on same state and do nothing else
	}
	if (actionIndex == 0xff)
	{
		ERR.print("unexpected character: ").printCharSymbol(currentInput).endl();
		error = &UNEXPECTED_CHARACTER;
		return;
	}

	void (* act)(ByteAutomata*) = actions[actionIndex];

	act(this);
}
void ByteAutomata::run (IInputStream * _input)
{
	init(_input);

	//while ((!input->end() || stayNextStep) && error == NO_ERROR)
	while(running())
	{
		step();
	}
	if (error == nullptr) uninit();
	else closeInput();
}
void ByteAutomata::uninit()
{
	LOG.println("end run");
	closeInput();
	printTreeStack();
	if (treeStackTop != 0)
	{
		error = &PARENTHESIS;
		return;
	}
	if (!stayNextStep)
	{	
		bufferIndex++;
		step('\n');
		if (stayNextStep) step('\n');
	}
}
void ByteAutomata::clearBuffer()
{
	LOG.println("clearBuffer");
	bufferIndex = 0;
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
	if (treeStackTop < 1) LOG.print(" tree stack, top: ").print(treeStackTop).endl();
	for(INT i=0; i<=treeStackTop; i++)
	{
		//LOG.print(" ["<<treeStack[i]<<":"<<(tree.getTag(treeStack[i]))<<"] ");
		LOG.print(tree.treeTypeName(tree.getType(treeStack[i]))).print(" > ");
	}
	LOG.println("");
}
void ByteAutomata::pushTree(INT subtreeType)
{
	ASSERT(tree.isSubtreeTag(subtreeType), "");

	INT newParent = tree.addSubtree(currentParent(), subtreeType);
	treeStackTop++;
	treeStack[treeStackTop] = newParent;

	LOG.print("pushTree: ");
	printTreeStack();
}
void ByteAutomata::popTree()
{
	treeStackTop--;

	LOG.print("popTree [top=").print(treeStackTop).print("]:");
	printTreeStack();
}
void ByteAutomata::startAssignment ()
{
	LOG.println("startAssignment");
	ASSERT(lineType == LINE_UNDEFINED, "");
	lineType = LINE_ASSIGNMENT;
	tree.init(NODE_ASSIGNMENT);
	treeStack[0] = 0;
	treeStackTop = 0;

	addLiteralToken(NODE_NAME);
	next(stateSpace);
}
void ByteAutomata::startFunction ()
{
	LOG.println("startFunction");
	ASSERT(lineType == LINE_UNDEFINED, "");
	lineType = LINE_CALL;
	tree.init(NODE_COMMAND);
	treeStack[0] = 0;
	treeStackTop = 0;

	addLiteralToken(NODE_NAME);
	stay();
	next(stateSpace);
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
	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_SUBTREE)
	{
		// parent is a subtree, start a new expr
		LOG.println("addToken: new expr");
		pushTree(NODE_EXPR);
	}
}
void ByteAutomata::addOperatorToken()
{
	LOG.println("addOperatorToken");
	prepareAddToken();
	tree.addOperatorNode(currentParent(), (char)currentInput);
}
void ByteAutomata::addTokenAndTransitionToSpace()
{
	if (currentState == stateName)
	{
		addLiteralToken(NODE_NAME);
	}
	else if (currentState == stateText)
	{
		addLiteralToken(NODE_TEXT);
	}
	else if (currentState == stateNumber)
	{
		LOG.println("addIntegerToken");
		INT value = parseInt(buffer, lastStart, getIndex());
		prepareAddToken();
		tree.addInt(currentParent(), value);
	}
	
	// continue at space state

	stay();
	next(stateSpace);
}
void ByteAutomata::addLiteralToken(INT nodeType)
{
	LOG.print("addLiteralToken: ").printHex(nodeType).endl();
	prepareAddToken();
	tree.addText(currentParent(), buffer.get(), lastStart, getIndex(), nodeType);
}
void ByteAutomata::comma()
{
	printTreeStack();
	LOG.println("comma");
	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_EXPR)
	{
		// pop from expr first

		popTree();
	}
}
void ByteAutomata::lineBreak()
{
	ASSERT(error == NO_ERROR, "");
	LOG.println("lineBreak: execute command");
	tree.print();
	error = paula.executeLine(indentation, tree);
	if (error != NO_ERROR) return;
	newLine();
}
void ByteAutomata::startBlock()
{
	LOG.println("addBlock");
	pushTree(NODE_SUBTREE);
}
void ByteAutomata::endBlock()
{
	LOG.println("endBlock");
	tree.print();
	
	if (tree.maskNodeTag(tree.get(currentParent())) == NODE_EXPR)
	{
		// pop from expr first
		LOG.println("pop expr");
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
	LOG.print("startExpr: indentation=").print(indentation).endl();
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
	stateText = addState("text"); // TODO
	
	transition(stateStart, "\t", [](ByteAutomata*ba)					{ ba->indentation++; });
	transition(stateStart, letters, [](ByteAutomata*ba)					{ ba->startExpr(ba->stateFirstName); });
	transition(stateStart, linebreak, [](ByteAutomata*ba)				{ ba->newLine(); });

	transition(stateFirstName, letters, 0);
	transition(stateFirstName, whiteSpace, [](ByteAutomata*ba)			{ ba->addTokenAndTransitionToSpace(); });
	transition(stateFirstName, ":", [](ByteAutomata*ba)					{ ba->startAssignment(); });
	transition(stateFirstName, blockStart, [](ByteAutomata*ba)			{ ba->startFunction(); });

	transition(stateSpace, whiteSpace, 0);
	transition(stateSpace, operators, [](ByteAutomata*ba)				{ ba->addOperatorToken(); });
	transition(stateSpace, letters, [](ByteAutomata*ba)					{ ba->next(ba->stateName); });
	transition(stateSpace, numbers, [](ByteAutomata*ba)					{ ba->next(ba->stateNumber); });
	transition(stateSpace, linebreak, [](ByteAutomata*ba)				{ ba->lineBreak(); });
	transition(stateSpace, blockStart, [](ByteAutomata*ba)				{ ba->startBlock(); });
	transition(stateSpace, blockEnd, [](ByteAutomata*ba)				{ ba->endBlock(); });
	transition(stateSpace, ",", [](ByteAutomata*ba)						{ ba->comma(); });

	transition(stateName, letters, 0);
	transition(stateName, whiteSpace, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateName, operators, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateName, blockStart, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateName, blockEnd, [](ByteAutomata*ba)					{ ba->addTokenAndTransitionToSpace(); });
	transition(stateName, linebreak, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });

	transition(stateNumber, numbers, 0);
	transition(stateNumber, whiteSpace, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateNumber, ",", [](ByteAutomata*ba)					{ ba->addTokenAndTransitionToSpace(); });
	transition(stateNumber, operators, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateNumber, blockEnd, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
	transition(stateNumber, linebreak, [](ByteAutomata*ba)				{ ba->addTokenAndTransitionToSpace(); });
}
}