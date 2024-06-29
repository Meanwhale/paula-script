#pragma once
#include "byteautomata.h"

namespace paula
{
constexpr INT
	TREE_ARRAY_SIZE = 10242,
	MAX_STATES = 32,
	MAX_STATES = 32,
	BA_BUFFER_SIZE = 512,
	CFG_MAX_NAME_LENGTH = 128;


const CHAR
	*letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	*numbers = "1234567890",
	*whiteSpace = " \t\n\r",
	*linebreak = "\n\r",
	*expressionBreak = ",;",
	*blockStart = "([{",
	*blockEnd = ")]}";


ByteAutomata::ByteAutomata() :
	tree(TREE_ARRAY_SIZE),
	tr(MAX_STATES * 256),
	buffer(BA_BUFFER_SIZE),
	tmp(BA_BUFFER_SIZE),
	stateNames(MAX_STATES)
{
	ok = true;
	currentInput = 0;
	currentState = 0;
	stateCounter = 0;
	actionCounter = 0;
	for (INT i=0; i<MAX_STATES * 256; i++) tr[i] = (BYTE)0xff;
	inputByte = 0;
	index = 0;
	lineNumber = 0;
	stayNextStep = false;
	running = false;

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
bool ByteAutomata::step (BYTE input)
{
	currentInput = input;
	INT index = (currentState * 256) + input;
	BYTE actionIndex = tr[index];
	if (actionIndex == 0) return true; // stay on same state and do nothing else
	if (actionIndex == 0xff||actionIndex < 0)
	{
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
	currentState = stateSpace;

	inputByte = 0;
	index = 0;
	lineNumber = 1;
	stayNextStep = false;
	running = true;

	treeParent = 0; // points to root node
	tree.clear();

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
	index++;
	running = step('\n');
	//if (!stayNextStep) index++;

	LOGLINE("end run");
	tree.print();
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


// state machine code

void ByteAutomata::addExpr()
{
	//MNode* expr = new MNode(currentBlock, NODE_EXPR, "<EXPR>");
	//(*currentExpr).next = expr;
	//currentExpr = expr;
	//currentToken = 0;
}
void ByteAutomata::addTextToken()
{
	LOG("addTextToken");
	tree.addText(treeParent, buffer, lastStart, getIndex());
}
void ByteAutomata::addToken(INT tokenType)
{
	
	//std::string data = (*automata).getString(lastStart, (*automata).getIndex() - lastStart);
	//vrbout()<<("NEW TOKEN: ")<<(data)<<std::endl;
	//MNode* token = new MNode(currentExpr, tokenType, data);
	//if (currentToken == 0) (*currentExpr).child = token;
	//else (*currentToken).next = token;
	//(*currentExpr).numChildren++;
	//currentToken = token;
	//lastStart = (*automata).getIndex();
}
void ByteAutomata::exprBreak()
{
	//if (currentBlock != 0) (*currentBlock).numChildren ++;
	//lastStart = -1;
	//addExpr();
}
void ByteAutomata::addBlock()
{
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
void ByteAutomata::defineTransitions()
{

	stateSpace = addState("space");
	stateName = addState("name");
	stateNumber = addState("number");
	
	transition(stateSpace, whiteSpace, 0);
	transition(stateSpace, letters, [](ByteAutomata*ba)					{ ba->next(ba->stateName); });
	transition(stateSpace, numbers, [](ByteAutomata*ba)					{ ba->next(ba->stateNumber); });
	transition(stateSpace, expressionBreak, [](ByteAutomata*ba)			{ ba->exprBreak(); });
	transition(stateSpace, blockStart, [](ByteAutomata*ba)				{ ba->addBlock();});
	transition(stateSpace, blockEnd, [](ByteAutomata*ba)				{ ba->endBlock();});
	
	transition(stateName, letters, 0);
	transition(stateName, whiteSpace, [](ByteAutomata*ba)				{ ba->addTextToken(); ba->next(ba->stateSpace); });
	transition(stateName, blockStart, [](ByteAutomata*ba)				{ ba->addTextToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, blockEnd, [](ByteAutomata*ba)					{ ba->addTextToken(); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateName, expressionBreak, [](ByteAutomata*ba)			{ ba->addTextToken(); ba->exprBreak(); ba->next(ba->stateSpace); });

	transition(stateNumber, numbers, 0);
	transition(stateNumber, whiteSpace, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->next(ba->stateSpace); });
	transition(stateNumber, blockStart, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateNumber, blockEnd, [](ByteAutomata*ba)				{ ba->addToken(NODE_INTEGER); ba->stay(); ba->next(ba->stateSpace); });
	transition(stateNumber, expressionBreak, [](ByteAutomata*ba)		{ ba->addToken(NODE_INTEGER); ba->exprBreak(); ba->next(ba->stateSpace); });
}
}