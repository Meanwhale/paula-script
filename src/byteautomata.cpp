#include "byteautomata.h"
#include "engine.h"

#define BA_CHECK(x,e) { if (!(x)) { error = &e; return; } }

namespace paula { namespace core {
constexpr INT
	TREE_ARRAY_SIZE = 1024,
	MAX_STATES = 32,
	MAX_DEPTH = 32,
	BA_BUFFER_SIZE = 1024;


const CHAR
	*letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",
	*numbers = "1234567890",
	*hexNumbers = "1234567890abcdefABCDEF",
	*operators = "+-*/<>=",
	*whiteSpace = " \t", // "\n\r",
	*lineBreak = "\n\r",
	*commandBreak = "\n\r;",
	*blockStart = "(", // "([{",
	*blockEnd = ")"; //")]}";


ByteAutomata::ByteAutomata(Engine& p) :
	paula(p),
	input(nullptr),
	error(NO_ERROR),
	tree(TREE_ARRAY_SIZE),
	tr(MAX_STATES * 256),
	buffer(BA_BUFFER_SIZE),
	quoteBuffer(MAX_TEXT_SIZE),
	treeStack(MAX_DEPTH),
	stateNames(MAX_STATES)
{
	currentInput = 0;
	currentState = 0;
	stateCounter = 0;
	actionCounter = 0;
	bufferIndex = -1;
	quoteIndex = -1;
	lineNumber = 0;
	stayNextStep = false;

	tr.fill((BYTE)0xff);

	defineTransitions();
}

//////////////// TRANSITION FUNCTIONS



BYTE ByteAutomata::addState (const CHAR * stateName)
{
	stateCounter++;
	stateNames[stateCounter] = stateName;
	return stateCounter;
}
//BYTE ByteAutomata::transition (BYTE state, std::vector<std::string>list, void (* action)(ByteAutomata*))
//{
//	BYTE actionIndex = 0;
//	if (action != nullptr) actionIndex = addAction(action);
//	for (const auto& str : list)
//	{
//		fillTransition(state, str.c_str(), actionIndex);
//	}
//	return actionIndex;
//}
void ByteAutomata::fillTransition (BYTE state, const CHAR * input, BYTE actionIndex)
{
	BYTE * bytes = (BYTE *)input;
	for(INT i = 0;bytes[i] != 0; i++)
	{
		tr[(state * 256) + bytes[i]] = actionIndex;
	}
}
BYTE ByteAutomata::transition (BYTE state, const CHAR * input, void (* action)(ByteAutomata*))
{
	BYTE actionIndex = 0;
	if (action != nullptr) actionIndex = addAction(action);
	fillTransition(state, input, actionIndex);
	return actionIndex;
}
BYTE ByteAutomata::fillTransition (BYTE state, void (* action)(ByteAutomata*))
{
	BYTE actionIndex = 0;
	if (action != 0) actionIndex = addAction(action);
	for (INT i=0; i<256; i++)
	{
		tr[(state * 256) + i] = actionIndex;
	}
	return actionIndex;
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
	lastStart = bufferIndex;
	currentState = nextState;
	VRB(LOG.print("Next state: ").print(stateNames[(INT)currentState]).print(" start: ").print(lastStart).print(" stay: ").print(stayNextStep).endl();)
}
void ByteAutomata::nextCont (BYTE nextState)
{
	// continue with same token, so don't reset the start index
	currentState = nextState;
	VRB(LOG.print("Next cont.: ").print(stateNames[(INT)currentState]).print(" start: ").print(lastStart).print(" stay: ").print(stayNextStep).endl();)
}
void ByteAutomata::stay ()
{
	// same input byte on next step
	ASSERT(!stayNextStep);
	stayNextStep = true;
}
void ByteAutomata::init (IInputStream * _input)
{
	input = _input;
	currentState = stateNewLine;
	bufferIndex = -1;
	lineNumber = 1;
	stayNextStep = false;
	indentation = 0;
	error = NO_ERROR;
	resetCommand();
}
//bool ByteAutomata::running()
//{
//	if (error != NO_ERROR)
//	{
//		ERR.print("ERROR in ByteAutomata: ").print(error).endl();
//		closeInput();
//		return false;
//	}
//	return input != nullptr || stayNextStep;
//}
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
	ASSERT(error == NO_ERROR);

	// get input byte
	BYTE inputByte;

	if (!stayNextStep)
	{
		if (input != nullptr)
		{
			// check end here in case input was shut down between steps.
			if (!input->read(inputByte))
			{
				LOG.println("input end");
				closeInput();

				// handle eof: add expr (line) break and end char to close open code blocks

				//                                  bufferIndex
				//						            |
				//              ... 'e'  'x'  'p'  'r'  ~ ~ ~ ~ ~ ~ ~ ~
				//
				//              ... 'e'  'x'  'p'  'r'  '\n' '\0' ~ ~ ~
				//                                       |   
				//                                       |   
				//                                       |
				//                                       bufferIndex

				bufferIndex++;
				buffer[bufferIndex]     = '\n';
				inputByte = '\n';
			}
			else
			{
				bufferIndex ++;
				buffer[bufferIndex] = inputByte;
			}
		}
		else
		{
			ASSERT(false);
			return;
		}
	}
	else
	{
		inputByte = currentInput;
		stayNextStep = false;
	}
	VRB(LOG.print("[ ").printCharSymbol(inputByte).print(" ] state: ").print(stateNames[(INT)currentState]));
	VRB(if (readIndex >= 0) LOG.print(" r: ").print(readIndex).print("/").print(bufferIndex););
	VRB(LOG.endl(););

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
		ERR.print("unexpected character: '").printCharSymbol(currentInput).print("' state: ").print(stateNames[(INT)currentState]).endl();
		error = &UNEXPECTED_CHARACTER;
		return;
	}

	void (* act)(ByteAutomata*) = actions[actionIndex];

	act(this);
}
bool ByteAutomata::parseLine (IInputStream * _input)
{
	// parse until a command break. return true if keep parsing.

	commandReady = false;

	//while ((!input->end() || stayNextStep) && error == NO_ERROR)
	while(!commandReady && error == NO_ERROR)
	{
		step();
	}
	return input != nullptr; // end wasn't reached so keep parsing
}
void ByteAutomata::uninit()
{
	LOG.println("end run");
	closeInput();
	printTreeStack();
	ASSERT (!stayNextStep);
}
void ByteAutomata::clearBuffer()
{
	LOG.println("clearBuffer");
	bufferIndex = -1;
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
#ifdef VERBOSE
	if (treeStackTop < 1) LOG.print(" tree stack, top: ").print(treeStackTop).endl();
	for(INT i=0; i<=treeStackTop; i++)
	{
		//LOG.print(" ["<<treeStack[i]<<":"<<(tree.getTag(treeStack[i]))<<"] ");
		LOG.print(treeTypeName(tree.getType(treeStack[i]))).print(" > ");
	}
	LOG.println("");
#endif
}
void ByteAutomata::pushTree(INT subtreeType)
{
	// add a new subtree and push its index to the stack

	ASSERT(tree.isSubtreeTag(subtreeType));

	INT newParent = tree.addSubtree(currentParent(), subtreeType);
	treeStackTop++;
	treeStack[treeStackTop] = newParent;

	VRB(LOG.print("pushTree: ");)
	printTreeStack();
}
void ByteAutomata::popTree()
{
	treeStackTop--;

	VRB(LOG.print("popTree [top=").print(treeStackTop).print("]:");)
	printTreeStack();
}
void ByteAutomata::startAssignment ()
{
	VRB(LOG.println("startAssignment");)
	ASSERT(commandType == LINE_UNDEFINED);
	commandType = LINE_ASSIGNMENT;
	next(stateSpace);
}
void ByteAutomata::startFunction ()
{
	VRB(LOG.println("startFunction");)
	ASSERT(commandType == LINE_UNDEFINED);
	commandType = LINE_CALL;
	stay();
	next(stateSpace);
}
INT ByteAutomata::parseInt(Array<BYTE>& src, INT i, INT lastByte)
{
	INT value = 0;
	while(i < lastByte)
	{
		// TODO negative
		BYTE b = src[i];
		ASSERT(b >= '0' && b <= '9');
		if (value > 0) value *= 10;
		value += b - '0';
		i++;
	}
	return value;
}
double ByteAutomata::parseDouble(Array<BYTE>& src, INT i, INT lastByte)
{
	double value;
	int numBytes = lastByte - i;
	if (sscanf((const char *)src.ptr(i), "%lf%n", &value, &numBytes) == 1)
	{
		return value;
	}
	ASSERT(false);
	return 0.0;
}
void ByteAutomata::addQuoteByte(BYTE b)
{
	quoteBuffer[quoteIndex++] = b; // TODO: check bounds
}
void ByteAutomata::addQuote()
{
	prepareAddToken();
	tree.addText(currentParent(), quoteBuffer.ptr(), 0, quoteIndex, NODE_TEXT);
}
void ByteAutomata::prepareAddToken()
{
	if (tree.getType(currentParent()) == NODE_SUBTREE)
	{
		// parent is a subtree "(...)", start a new expr after "(" or ","
		VRB(LOG.println("addToken: new expr");)
		pushTree(NODE_EXPR);
	}
}
void ByteAutomata::addOperatorToken()
{
	VRB(LOG.println("addOperatorToken");)
	prepareAddToken();
	tree.addOperatorNode(currentParent(), (char)currentInput);
}
void ByteAutomata::addFirstNameAndTransit()
{
	addLiteralToken(NODE_NAME);
	stay();
	next(stateAfterFirstName);
}
void ByteAutomata::addTokenAndTransitionToSpace()
{

	if (currentState == stateName)
	{
		addLiteralToken(NODE_NAME);
	}
	else if (currentState == stateQuote)
	{
		addLiteralToken(NODE_TEXT);
	}
	else if (currentState == stateNumber)
	{
		VRB(LOG.print("add integer token: ").print(lastStart).print(" -> ").print(readIndex).endl();)
		INT value = parseInt(buffer, lastStart, bufferIndex);
		prepareAddToken();
		tree.addInt(currentParent(), value);
	}
	else if (currentState == stateDecimal)
	{
		VRB(LOG.print("add decimal token: ").print(lastStart).print(" -> ").print(readIndex).endl();)
		double value = parseDouble(buffer, lastStart, bufferIndex);
		prepareAddToken();
		tree.addDouble(currentParent(), value);
	}
	else ASSERT(false);
	
	// continue at space state

	stay();
	next(stateSpace);
}
void ByteAutomata::addLiteralToken(INT nodeType)
{
	if (nodeType == NODE_NAME && bufferIndex - lastStart >= MAX_VAR_NAME_LENGTH)
	{
		error = &VARIABLE_NAME_TOO_LONG;
		return;
	}
	if (nodeType == NODE_TEXT && bufferIndex - lastStart >= MAX_TEXT_SIZE)
	{
		error = &TEXT_TOO_LONG;
		return;
	}
	VRB(LOG.print("add token: ").print(lastStart).print(" -> ").print(readIndex).endl();)
	VRB(LOG.print("addLiteralToken: ").printHex(nodeType).endl();)
	prepareAddToken();
	tree.addText(currentParent(), buffer.ptr(), lastStart, bufferIndex, nodeType);
}
void ByteAutomata::addHexByte()
{
	// eg. "\xF4"
	//       ^lastStart
	BYTE high = buffer[lastStart + 1];
	BYTE low = buffer[lastStart + 2];
	BYTE b = (BYTE)(((hexCharToByte(high) << 4) & 0xf0) | hexCharToByte(low));
	addQuoteByte(b);
}
void ByteAutomata::escapeChar()
{
	// standard escape character literals: https://en.cppreference.com/w/cpp/language/escape
	char c = (char)currentInput;

	     if (c == '\'') addQuoteByte(0x27);
	else if (c == '\"') addQuoteByte(0x22);
	else if (c == '?')  addQuoteByte(0x3f);
	else if (c == '\\') addQuoteByte(0x5c);
	else if (c == 'a')  addQuoteByte(0x07);
	else if (c == 'b')  addQuoteByte(0x08);
	else if (c == 'f')  addQuoteByte(0x0c);
	else if (c == 'n')  addQuoteByte(0x0a);
	else if (c == 'r')  addQuoteByte(0x0d);
	else if (c == 't')  addQuoteByte(0x09);
	else if (c == 'v')  addQuoteByte(0x0b);
	else if (c == 'x')
	{
		next(stateHexChar);
		return;
	}
	else error = &QUOTE_ERROR;

	next(stateQuote);
}
void ByteAutomata::comma()
{
	printTreeStack();
	VRB(LOG.println("comma");)
	if (tree.getType(currentParent()) == NODE_EXPR)
	{
		// pop from expr first

		popTree();
	}
}
void ByteAutomata::startBlock()
{
	VRB(LOG.println("addBlock");)
		pushTree(NODE_SUBTREE);
}
void ByteAutomata::endBlock()
{
	VRB(LOG.println("endBlock");)
		tree.print();

	if (tree.getType(currentParent()) == NODE_EXPR)
	{
		// pop from expr first
		VRB(LOG.println("pop expr");)
			printTreeStack();
		popTree();
	}

	popTree();
}
void ByteAutomata::breakCommand()
{
	LOG.println("breakCommand: finish command");

	oneLiner = false;
	finishCommand();
	next(stateNewCommand);
}
void ByteAutomata::breakLine()
{
	LOG.println("breakLine: finish command");

	finishCommand();
	startNewLine();
}
void ByteAutomata::startNewLine()
{
	next(stateNewLine);
}
void ByteAutomata::eof()
{
	LOG.println("---------------- EOF ----------------");
}
void ByteAutomata::finishCommand()
{
	// command is parsed and ready to being added to command list

	if (commandType == LINE_UNDEFINED)
	{
		error = &SYNTAX_ERROR;
		return;
	}
	if (treeStackTop != 0)
	{
		error = &PARENTHESIS;
		return;
	}
	commandReady = true; // tell the loop to break and add command to list
	//error = paula.executeLine(indentation, oneLiner, commandStartIndex, commandType, tree);
}
void ByteAutomata::resetNewLine()
{
	// call from engine
	oneLiner = true;
	indentation = 0;
}
void ByteAutomata::resetCommand()
{
	commandType = LINE_UNDEFINED;
	commandStartIndex = bufferIndex;
	
	// tree node to the top of the stack

	tree.init(NODE_STATEMENT);
	treeStack[0] = 0;
	treeStackTop = 0;
}
void ByteAutomata::startExpr(BYTE firstState)
{
	VRB(LOG.print("startExpr: indentation=").print(indentation).endl();)
	stay();
	next(firstState);
}

#define TRANSITION(state,chars,callback) ai = transition(state, chars, [](ByteAutomata*ba) callback);
#define SAME_TRANS(state,chars) fillTransition(state, chars, ai);
#define FILL_TRANSITION(state,callback) ai = fillTransition(state, [](ByteAutomata*ba) callback);

void ByteAutomata::defineTransitions()
{
	/*
			states: start (ind.) → name [first name] → [post name]
				1) assign → read expr
				2) function → read args ()
	*/

	stateNewLine = addState("new line");
	stateNewCommand = addState("new command");
	stateSpace = addState("space");
	stateName = addState("name");
	stateFirstName = addState("first name");
	stateAfterFirstName = addState("after first name");
	statePostName = addState("post name");
	stateName = addState("name");
	stateNumber = addState("number");
	stateDecimal = addState("decimal");
	stateQuote = addState("quote");
	stateEscapeChar = addState("excape character");
	stateHexChar = addState("hex character");

	BYTE ai; // action index

	TRANSITION(stateNewLine, "\t",										{ ba->indentation++; });
	TRANSITION(stateNewLine, letters,									{ ba->startExpr(ba->stateFirstName); });
	TRANSITION(stateNewLine, lineBreak,									{ ba->startNewLine(); });

	transition(stateNewCommand, whiteSpace,	nullptr);
	TRANSITION(stateNewCommand, letters,								{ ba->startExpr(ba->stateFirstName); });
	TRANSITION(stateNewCommand, lineBreak,								{ ba->startNewLine(); });

	transition(stateFirstName, letters, nullptr);
	TRANSITION(stateFirstName, whiteSpace,								{ ba->addFirstNameAndTransit(); });
	SAME_TRANS(stateFirstName, ":");
	SAME_TRANS(stateFirstName, blockStart);

	transition(stateAfterFirstName, whiteSpace, nullptr);
	TRANSITION(stateAfterFirstName, ":",								{ ba->startAssignment(); });
	TRANSITION(stateAfterFirstName, blockStart,							{ ba->startFunction(); });

	transition(stateSpace, whiteSpace, nullptr);
	TRANSITION(stateSpace, operators,									{ ba->addOperatorToken(); });
	TRANSITION(stateSpace, letters,										{ ba->next(ba->stateName); });
	TRANSITION(stateSpace, numbers,										{ ba->next(ba->stateNumber); });
	TRANSITION(stateSpace, lineBreak,									{ ba->breakLine(); });
	TRANSITION(stateSpace, ";",											{ ba->breakCommand(); });
	TRANSITION(stateSpace, blockStart,									{ ba->startBlock(); });
	TRANSITION(stateSpace, blockEnd,									{ ba->endBlock(); });
	TRANSITION(stateSpace, ",",											{ ba->comma(); });
	TRANSITION(stateSpace, "\"",										{ ba->next(ba->stateQuote); ba->quoteIndex = 0; });

	FILL_TRANSITION(stateQuote,											{ ba->addQuoteByte(ba->currentInput); });
	TRANSITION(stateQuote, lineBreak,									{ ba->error = &QUOTE_ERROR; });
	TRANSITION(stateQuote, "\"",										{ ba->lastStart++; ba->addQuote(); ba->next(ba->stateSpace); });
	TRANSITION(stateQuote, "\\",										{ ba->next(ba->stateEscapeChar); });

	FILL_TRANSITION(stateEscapeChar,									{ ba->escapeChar(); });

	FILL_TRANSITION(stateHexChar,										{ ba->error = &INVALID_HEXADECIMAL_CHARACTER; });
	TRANSITION(stateHexChar, hexNumbers,								{ if (ba->bufferIndex - ba->lastStart >= 2) { ba->addHexByte(); ba->next(ba->stateQuote); }; });


	FILL_TRANSITION(stateName,											{ ba->addTokenAndTransitionToSpace(); });
	transition(stateName, letters, nullptr);
	fillTransition(stateName, numbers, -1);

	FILL_TRANSITION(stateNumber,										{ ba->addTokenAndTransitionToSpace(); });
	transition(stateNumber, numbers, nullptr);
	TRANSITION(stateNumber, ".", 										{ ba->nextCont(ba->stateDecimal); });
	fillTransition(stateNumber, letters, -1);

	FILL_TRANSITION(stateDecimal,										{ ba->addTokenAndTransitionToSpace(); });
	transition(stateDecimal, numbers, nullptr);
	fillTransition(stateDecimal, letters, -1);

	/*LOG.print("constexpr int maxStates=").print(ai+1).print(";").endl();
	LOG.println("constexpr BYTE* transitionTable = {");

	for (INT i=0; i<256; i++) LOG.print("0, ");
	for (BYTE state = 1; state <= ai; state++)
	{
		LOG.endl();
		for (INT i=0; i<256; i++)
		{
			LOG.print(tr[(state*256) + i]).print(",");
		}
	}
	LOG.endl().print("};").endl();*/
}}}