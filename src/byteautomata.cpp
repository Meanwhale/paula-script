#include "byteautomata.h"
#include "paula.h"

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
	*operators = "+-*/<>=",
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
	lastStart = readIndex;
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
	currentState = stateStart;
	bufferIndex = -1;
	readIndex = -1;
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
	return input != nullptr || readIndex < bufferIndex || stayNextStep;
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

	if (!stayNextStep)
	{
		if (readIndex < bufferIndex)
		{
			// read from buffer (loop)
			readIndex++;
			inputByte = buffer[readIndex];
		}
		else if (input != nullptr)
		{
			ASSERT(readIndex == bufferIndex);
			// read from input to buffer
			// check end here in case input was shut down between steps.
			if (!input->read(inputByte))
			{
				LOG.println("input end");
				closeInput();

				// handle eof: add expr (line) break and end char to close open code blocks

				//                                  read|bufferIndex
				//						            |
				//              ... 'e'  'x'  'p'  'r'  ~ ~ ~ ~ ~ ~ ~ ~
				//
				//              ... 'e'  'x'  'p'  'r'  '\n' '\0' ~ ~ ~
				//                                       |    |  
				//                                       |    bufferIndex
				//                                       |
				//                                       readIndex

				bufferIndex++;
				buffer[bufferIndex] = '\n';
				bufferIndex++;
				buffer[bufferIndex] = '\x4'; // end of transmission
				readIndex++;
				inputByte = '\n';
			}
			else
			{
				bufferIndex ++;
				readIndex ++;
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
	ASSERT (!stayNextStep);
}
void ByteAutomata::clearBuffer()
{
	LOG.println("clearBuffer");
	bufferIndex = -1;
	readIndex = -1;
}

void ByteAutomata::jump(INT address)
{
	lineStartIndex = readIndex = address;
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
		LOG.print(treeTypeName(tree.getType(treeStack[i]))).print(" > ");
	}
	LOG.println("");
}
void ByteAutomata::pushTree(INT subtreeType)
{
	ASSERT(tree.isSubtreeTag(subtreeType));

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
	ASSERT(lineType == LINE_UNDEFINED);
	lineType = LINE_ASSIGNMENT;
	next(stateSpace);
}
void ByteAutomata::startFunction ()
{
	LOG.println("startFunction");
	ASSERT(lineType == LINE_UNDEFINED);
	lineType = LINE_CALL;
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
		LOG.print("add integer token: ").print(lastStart).print(" -> ").print(readIndex).endl();
		INT value = parseInt(buffer, lastStart, readIndex);
		prepareAddToken();
		tree.addInt(currentParent(), value);
	}
	else if (currentState == stateDecimal)
	{
		LOG.print("add decimal token: ").print(lastStart).print(" -> ").print(readIndex).endl();
		double value = parseDouble(buffer, lastStart, readIndex);
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
	if (nodeType == NODE_NAME && readIndex - lastStart >= MAX_VAR_NAME_LENGTH)
	{
		error = &VARIABLE_NAME_TOO_LONG;
		return;
	}
	if (nodeType == NODE_TEXT && readIndex - lastStart >= MAX_TEXT_SIZE)
	{
		error = &TEXT_TOO_LONG;
		return;
	}
	LOG.print("add token: ").print(lastStart).print(" -> ").print(readIndex).endl();
	LOG.print("addLiteralToken: ").printHex(nodeType).endl();
	prepareAddToken();
	tree.addText(currentParent(), buffer.ptr(), lastStart, readIndex, nodeType);
}
void ByteAutomata::comma()
{
	printTreeStack();
	LOG.println("comma");
	if (tree.getType(currentParent()) == NODE_EXPR)
	{
		// pop from expr first

		popTree();
	}
}
void ByteAutomata::lineBreak()
{
	ASSERT(error == NO_ERROR);
	LOG.println("lineBreak: execute command");
	tree.print();
	if (lineType == LINE_UNDEFINED)
	{
		error = &SYNTAX_ERROR;
		return;
	}
	if (treeStackTop != 0)
	{
		error = &PARENTHESIS;
		return;
	}
	error = paula.executeLine(indentation, lineStartIndex, lineType, tree);
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
	
	if (tree.getType(currentParent()) == NODE_EXPR)
	{
		// pop from expr first
		LOG.println("pop expr");
		printTreeStack();
		popTree();
	}
	
	popTree();
}
void ByteAutomata::eof()
{
	LOG.println("---------------- EOF ----------------");
	bool executeLine = false;
	error = paula.lineIndentationInit(0, executeLine);
}
void ByteAutomata::newLine()
{
	indentation = 0;
	lineType = LINE_UNDEFINED;
	lineStartIndex = readIndex;
	
	// tree node to the top of the stack

	tree.init(NODE_STATEMENT);
	treeStack[0] = 0;
	treeStackTop = 0;
	next(stateStart);
}
void ByteAutomata::startExpr(BYTE firstState)
{
	LOG.print("startExpr: indentation=").print(indentation).endl();
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

	stateStart = addState("start");
	stateSpace = addState("space");
	stateName = addState("name");
	stateFirstName = addState("first name");
	stateAfterFirstName = addState("after first name");
	statePostName = addState("post name");
	stateName = addState("name");
	stateNumber = addState("number");
	stateDecimal = addState("decimal");
	stateQuote = addState("quote"); // TODO

	BYTE ai; // action index

	TRANSITION(stateStart, "\x4",										{ ba->eof(); })
	TRANSITION(stateStart, "\t",										{ ba->indentation++; });
	TRANSITION(stateStart, letters,										{ ba->startExpr(ba->stateFirstName); });
	TRANSITION(stateStart, linebreak,									{ ba->newLine(); });

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
	TRANSITION(stateSpace, linebreak,									{ ba->lineBreak(); });
	TRANSITION(stateSpace, blockStart,									{ ba->startBlock(); });
	TRANSITION(stateSpace, blockEnd,									{ ba->endBlock(); });
	TRANSITION(stateSpace, ",",											{ ba->comma(); });
	TRANSITION(stateSpace, "\"",										{ ba->next(ba->stateQuote); ba->quoteIndex = 0; });

	FILL_TRANSITION(stateQuote,											{ ba->addQuoteByte(ba->currentInput); });
	TRANSITION(stateQuote, linebreak,									{ ba->error = &QUOTE_ERROR; });
	TRANSITION(stateQuote, "\"",										{ ba->lastStart++; ba->addQuote(); ba->next(ba->stateSpace); });
	TRANSITION(stateQuote, "\\",										{ ba->error = &QUOTE_ERROR; });

	transition(stateName, letters, nullptr);
	TRANSITION(stateName, whiteSpace,									{ ba->addTokenAndTransitionToSpace(); });
	SAME_TRANS(stateName, operators);
	SAME_TRANS(stateName, blockStart);
	SAME_TRANS(stateName, blockEnd);
	SAME_TRANS(stateName, linebreak);

	transition(stateNumber, numbers, nullptr);
	TRANSITION(stateNumber, ".", 										{ ba->nextCont(ba->stateDecimal); });
	TRANSITION(stateNumber, whiteSpace,									{ ba->addTokenAndTransitionToSpace(); });
	SAME_TRANS(stateNumber, ",");
	SAME_TRANS(stateNumber, operators);
	SAME_TRANS(stateNumber, blockEnd);
	SAME_TRANS(stateNumber, linebreak);

	transition(stateDecimal, numbers, nullptr);
	TRANSITION(stateDecimal, whiteSpace,								{ ba->addTokenAndTransitionToSpace(); });
	SAME_TRANS(stateDecimal, operators);
	SAME_TRANS(stateDecimal, blockStart);
	SAME_TRANS(stateDecimal, blockEnd);
	SAME_TRANS(stateDecimal, linebreak);

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