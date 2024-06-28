#pragma once
#include "byteautomata.h"
#include "stream.h"
#include "defs.h"

namespace paula
{

void defineTransitions(ByteAutomata & ba);

constexpr INT MAX_STATES = 32;
constexpr INT BA_BUFFER_SIZE = 512;
constexpr INT CFG_MAX_NAME_LENGTH = 128;

BYTE stateSpace, stateName, stateNumber;
ByteAutomata* automata;
INT lastStart;

const CHAR * letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const CHAR * numbers = "1234567890";
const CHAR * whiteSpace = " \t\n\r";
const CHAR * linebreak = "\n\r";
const CHAR * expressionBreak = ",;";
const CHAR * blockStart = "([{";
const CHAR * blockEnd = ")]}";
constexpr INT NODE_EXPR = 101;
constexpr INT NODE_PARENTHESIS = 102;
constexpr INT NODE_SQUARE_BRACKETS = 103;
constexpr INT NODE_CURLY_BRACKETS = 104;
constexpr INT NODE_TEXT = 105;
constexpr INT NODE_NUMBER = 106;


ByteAutomata::ByteAutomata()
{
	ok = true;
	currentInput = 0;
	currentState = 0;
	stateCounter = 0;
	actionCounter = 0;
	tr.reset(MAX_STATES * 256);
	for (INT i=0; i<MAX_STATES * 256; i++) tr[i] = (BYTE)0xff;
	inputByte = 0;
	index = 0;
	lineNumber = 0;
	stayNextStep = false;
	running = false;
	buffer.reset(BA_BUFFER_SIZE);
	tmp.reset(BA_BUFFER_SIZE);
	stateNames.reset(MAX_STATES);

	defineTransitions(*this);
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
void ByteAutomata::transition (BYTE state, const CHAR * input, void (* action)())
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
void ByteAutomata::fillTransition (BYTE state, void (* action)())
{
	BYTE actionIndex = 0;
	if (action != 0) actionIndex = addAction(action);
	for (INT i=0; i<256; i++)
	{
		tr[(state * 256) + i] = actionIndex;
	}
	//DEBUG(VR("New Transition filled: id ")X(actionIndex)XO);
}
BYTE ByteAutomata::addAction (void (* action)())
{
	actionCounter++;
	actions[actionCounter] = action;
	return actionCounter;
}
void ByteAutomata::next (BYTE nextState)
{
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
	void (* act)() = actions[actionIndex];
	if (act == 0)
	{
		ASSERT(false, "invalid action index");
	}
	act();
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
//std::string ByteAutomata::getString (INT start, INT length)
//{
//	CHECK(length < CFG_MAX_NAME_LENGTH, "name is too long");
//	INT i = 0;
//	for (; i < length; i++)
//	{
//		tmp[i] = buffer[start++ % BA_BUFFER_SIZE];
//	}
//	tmp[i] = '\0';
//	return std::string((BYTE*)tmp.get());
//}
void ByteAutomata::run (IInputStream & input)
{
	currentState = stateSpace;

	inputByte = 0;
	index = 0;
	lineNumber = 1;
	stayNextStep = false;
	running = true;
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
		std::cout<<("[ ")<<((BYTE)(inputByte))<<(" ]")<<(" state: ")<<(stateNames[(INT)currentState])<<std::endl;
		running = step(inputByte);
	}
	if (!stayNextStep) index++;
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


// state machine definition


//MNode* root;
//MNode* currentBlock;
//MNode* currentExpr;
//MNode* currentToken;

void next(BYTE state)
{
	// transition to a next state
	lastStart = (*automata).getIndex();
	(*automata).next(state);
}
void stay()
{
	(*automata).stay();
}
void addExpr()
{
	//MNode* expr = new MNode(currentBlock, NODE_EXPR, "<EXPR>");
	//(*currentExpr).next = expr;
	//currentExpr = expr;
	//currentToken = 0;
}
void addToken(INT tokenType)
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
void exprBreak()
{
	//if (currentBlock != 0) (*currentBlock).numChildren ++;
	//lastStart = -1;
	//addExpr();
}
void addBlock()
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
void endBlock()
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
void defineTransitions(ByteAutomata & ba)
{
	automata = &ba;

	stateSpace = ba.addState("space");
	stateName = ba.addState("name");
	stateNumber = ba.addState("number");
	ba.transition(stateSpace, whiteSpace, 0);
	ba.transition(stateSpace, letters, []() { next(stateName); });
	ba.transition(stateSpace, numbers, []() { next(stateNumber); });
	ba.transition(stateSpace, expressionBreak, []() { exprBreak(); });
	ba.transition(stateSpace, blockStart, []() { addBlock();});
	ba.transition(stateSpace, blockEnd, []() { endBlock();});
	ba.transition(stateName, letters, 0);
	ba.transition(stateName, whiteSpace, []() { addToken(NODE_TEXT); next(stateSpace); });
	ba.transition(stateName, blockStart, []() { addToken(NODE_TEXT); stay(); next(stateSpace); });
	ba.transition(stateName, blockEnd, []() { addToken(NODE_TEXT); stay(); next(stateSpace); });
	ba.transition(stateName, expressionBreak, []() { addToken(NODE_TEXT); exprBreak(); next(stateSpace); });
	ba.transition(stateNumber, numbers, 0);
	ba.transition(stateNumber, whiteSpace, []() { addToken(NODE_NUMBER); next(stateSpace); });
	ba.transition(stateNumber, blockStart, []() { addToken(NODE_NUMBER); stay(); next(stateSpace); });
	ba.transition(stateNumber, blockEnd, []() { addToken(NODE_NUMBER); stay(); next(stateSpace); });
	ba.transition(stateNumber, expressionBreak, []() { addToken(NODE_NUMBER); exprBreak(); next(stateSpace); });
}
}