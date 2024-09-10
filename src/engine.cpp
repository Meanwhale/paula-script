#include "engine.h"
#include "stream.h"
#include "args.h"

using namespace paula;
using namespace paula::core;

// PAULA

ERROR_STATUS printAction (Engine&,Args&args)
{
	LOG.println("-------- PRINT ACTION --------");
	for(INT i=0; i<args.count(); i++)
	{
		pout.print(args.get(i));
	}
	LOG.endl().println("------------------------------");
	return NO_ERROR;
}
ERROR_STATUS notAction (Engine&p,Args&args)
{
	LOG.println("-------- NOT ACTION --------");
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		args.returnBool(!value);
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS whileAction (Engine&p,Args&args)
{
	LOG.println("-------- WHILE ACTION --------");
	CHECK(p.oneLiner, CONDITION_LINE_WITH_SEMICOLON);
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		if (value) p.startLoop();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS ifAction (Engine&p,Args&args)
{
	LOG.println("-------- IF ACTION --------");
	CHECK(p.oneLiner, CONDITION_LINE_WITH_SEMICOLON);
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	bool value = false;
	if(args.get(0).getBool(value))
	{
		if (value) p.startIf();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}

Engine Engine::one = Engine();


Engine::Engine() : //buffer(BUFFER_SIZE), index(0)
	vars(VARS_SIZE),
	oneLiner(true),
	currentIndentation(0),
	skipIndentation(-1),
	blockStackSize(0),
	bytecodeIndex(0),
	numCallbacks(0),
	jumpIndex(-1),
	automata(*this),
	stack(ARG_STACK_SIZE),
	constants(CONSTANTS_SIZE),
	bytecode(BYTECODE_SIZE),
	args(stack),
	commands
	{
		Command("print", printAction),
		Command("not", notAction),
		Command("while", whileAction),
		Command("if", ifAction)
	}
{
	constants.init(NODE_SUBTREE);

	LOG.println("---------------- NEW PAULA ----------------");
	log.print("toimii!").endl();


	INT kvIndex = constants.addSubtree(0, NODE_KV);
	constants.addText(kvIndex, "true");
	constants.addBool(kvIndex, true);
	
	kvIndex = constants.addSubtree(0, NODE_KV);
	constants.addText(kvIndex, "false");
	constants.addBool(kvIndex, false);
}


ERROR_STATUS Engine::addParsedLine()
{
	// add line info and parsed tree
	INT nodeIndex = bytecode.addSubtree(0, NODE_SUBTREE);
	bytecode.addInt(nodeIndex, automata.lineNumber);
	bytecode.addInt(nodeIndex, automata.indentation);
	bytecode.addInt(nodeIndex, automata.commandType);
	bytecode.addBool(nodeIndex, automata.oneLiner);
	bytecode.addRawTree(nodeIndex, automata.tree);
	LOG.print("PARSED TREE ADDED:\n");
	automata.tree.print();
	LOG.endl();
	return NO_ERROR;
}


ERROR_STATUS Engine::run(IInputStream& input, bool handleErrors)
{
	LOG.println("Paula::newRun");

	vars.init(NODE_SUBTREE);
	bytecode.init(NODE_SUBTREE);

	currentIndentation = 0;
	skipIndentation = -1;
	blockStackSize = 0;
	const Error* error = nullptr; // NO_ERROR

	// parse lines and add them to the bytecode list

	automata.init(&input);
	bool running = true;
	while(running)
	{
		running = automata.parseLine(&input);
		error = automata.getError();
		if (error != nullptr) return returnHandleError(error, handleErrors);
		CHECK_CALL(addParsedLine());	// add command to bytecode list
		automata.resetCommand();		// prepare to another command

		// reset line here so that addParsedLine has correct values
		if (automata.currentState == automata.stateNewLine) automata.resetNewLine();
	}
	
	// all lines parsed --> execute bytecode

	bytecode.print();

	TreeIterator it(bytecode);
	if (!it.hasChild())	return NO_ERROR;
	it.toChild();

	bool hasNextLine;
	do
	{
		INT bytecodeIndex = it.index;

		it.toChild();
		// read line data
		INT lineNumber, indentation, commandType;
		bool oneLiner;

		it.var().getInt(lineNumber);  it.next();
		it.var().getInt(indentation); it.next();
		it.var().getInt(commandType); it.next();
		it.var().getBool(oneLiner); it.next();

		// assign parsed tree from bytecode
		INT* rawTreeDataPtr  = it.tree.data.ptr(it.index + 3);
		INT  rawTreeDataSize = it.var().size() - 2;
		Tree parsedCommand(rawTreeDataPtr, rawTreeDataSize);

		VRB(LOG.print("\nEXECUTE LINE\n"));
		VRB(parsedCommand.print());

		CHECK_CALL(executeLine(indentation, oneLiner, bytecodeIndex, commandType, parsedCommand));

		it.toParent();

		if (!it.hasNext())
		{
			// end of file. make a call to end if's and loops. possibly jump back by moving bytecode index.
			bool executeLine = false;
			CHECK_CALL(lineIndentationInit(0, executeLine));
		}
		if (jumpIndex >= 0)
		{
			it.jumpTo(jumpIndex);
			ASSERT(it.isType(NODE_SUBTREE));
			jumpIndex = -1;
			hasNextLine = true;
		}
		else
		{
			hasNextLine = it.next();
		}
	}
	while(hasNextLine);

	return NO_ERROR;
}

ERROR_STATUS Engine::returnHandleError(const Error* error, bool handleErrors)
{
	error = automata.getError();
	if (handleErrors && error != NO_ERROR)
	{
#ifndef PAULA_MINI
		ERR
			.print("Caught an exception: ")
			.print(error->name)
			.print(" (id=")
			.print(error->id)
			.print(")")
			.endl();
		ERR.flush();
#else
		log.endl().print("ERROR: L").print(error->id).endl(); // L = line
#endif
		error = NO_ERROR;
	}
	log.flush();
	return error;
}

ERROR_STATUS core::Engine::addCallback(const char* callbackName, const Error * (* _action)(Args&))
{
	INT tmp[MAX_VAR_NAME_DATA_LENGTH];
	Array<INT> nameData (tmp, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(callbackName, nameData);

	if (isReservedName(nameData.ptr())) return &RESERVED_NAME;
	
	if (numCallbacks >= MAX_USER_CALLBACKS) return &CALLBACK_ERROR;
	callbacks[numCallbacks].setup(nameData, _action);
	numCallbacks++;
	return NO_ERROR;
}

ERROR_STATUS core::Engine::jump(INT bytecodeIndex)
{
	// move bytecode iterator
	ASSERT(jumpIndex < 0);
	jumpIndex = bytecodeIndex;
	return NO_ERROR;
}

ERROR_STATUS core::Engine::lineIndentationInit(INT indentation, bool& executeLine)
{
	executeLine = true;
	currentIndentation = indentation;

	if (skipIndentation > 0)
	{
		LOG.println("-------- SKIP INDENTATION --------");
		if (indentation >= skipIndentation)
		{
			LOG.println("-------- SKIP LINE --------");
			// inside a block
			executeLine = false;
			return NO_ERROR;
		}
		LOG.println("-------- END SKIP --------");
		skipIndentation = -1;
	}

	if (blockStackSize == 0)
	{
		CHECK(indentation == blockStackSize, INDENTATION_ERROR);
	}
	else
	{
		Block& block = blockStack[blockStackSize-1];
		if (indentation == block.indentation)
		{
			LOG.println("stay inside the block");
		}
		else while (indentation < block.indentation)
		{
			LOG.println("end of the block");
			if (block.loop)
			{
				LOG.println("-------- JUMP BACK --------");
				LOG.print("address: ").print(block.startBytecodeIndex).endl();
				LOG.print("indentation: ").print(block.indentation).endl();
				LOG.print("current blockStackSize: ").print(blockStackSize).endl();
				CHECK_CALL(jump(block.startBytecodeIndex));
				blockStackSize--;
				LOG.print("jump back, pop stack, stack size: ").print(blockStackSize).endl();
				executeLine = false;
				return NO_ERROR;
			}
			else
			{
				LOG.println("-------- END IF BLOCK --------");
				blockStackSize--;
				if (blockStackSize > 0)
				{
					// continue as several blocks might have ended
					block = blockStack[blockStackSize-1];
				}
				else
				{
					return NO_ERROR;
				}
			}
		}
	}
	return NO_ERROR;
}

ERROR_STATUS core::Engine::executeLine(INT indentation, bool _oneLiner, INT _bytecodeIndex, INT lineType, Tree& tree)
{
	oneLiner = _oneLiner;
	bool executeLine = false;
	CHECK_CALL(lineIndentationInit(indentation, executeLine));

	if (!executeLine) return NO_ERROR;

	bytecodeIndex = _bytecodeIndex;

	stack.clear();

	if (lineType == LINE_ASSIGNMENT)
	{
		// TRG : SRC

		LOG.print("execute ASSIGNMENT: indentation=").print(indentation).endl();
		TreeIterator it(tree);
		it.toChild(); // points to variable name
		LOG.print("variable name: ").print(it).endl();

		// new or override?

		VRB(vars.print();)

		INT index = findVariableIndex(it.getTextData(), vars);

		if (index >= 0)
		{
			LOG.println("-------- OVERWRITE VAR --------");
			// variable already exists
			TreeIterator data(vars, index); // points to the data
			LOG.print("old value: ").print(data).endl();

			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			StackIterator src(stack);
			LOG.print("overwrite value. new value: ").print(src.var()).endl();
			if (src.type() == NODE_TEXT || data.type() == NODE_TEXT) return &TEXT_VARIABLE_OVERWRITE;
			vars.print();
			vars.printData();
			data.overwrite(src.var());
		}
		else
		{
			LOG.println("-------- NEW VAR --------");
			if (isReservedName(it.getTextData())) return &RESERVED_NAME;
			INT kvIndex = vars.addSubtree(0, NODE_KV);
			// new
			vars.addData(kvIndex, it); // add variable name to KV
			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			StackIterator src(stack);
			LOG.print("assign value: ").print(src.var()).endl();
			vars.addData(kvIndex, src.var()); // add value to KV
			stack.pop();
		}

		VRB(vars.print();)
	}
	else if (lineType == LINE_CALL)
	{
		// COMMAND { funcName ( args ) }

		LOG.print("execute COMMAND: indentation=").print(indentation).endl();
		TreeIterator it(tree);
		it.toChild(); // points to command name

		auto cmd = findCommand(it.getTextData());
		if (cmd)
		{
			it.next();
			CHECK(!it.hasNext(), SYNTAX_ERROR); // extra tokens after ()

			CHECK_CALL(pushArgListAndExecute(it, cmd));
		}
		else
		{
			CHECK(false, UNKNOWN_COMMAND);
		}
	}
	else
	{
		ASSERT(false);
	}
	if (blockStackSize == 0 && skipIndentation <= 0)
	{
		automata.clearBuffer();
	}
	return NO_ERROR;
}

void core::Engine::startLoop()
{
	LOG.println("-------- START LOOP --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startBytecodeIndex = bytecodeIndex;
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].loop = true;
	blockStackSize++;
}
void core::Engine::startIf()
{
	LOG.println("-------- START IF --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startBytecodeIndex = -123456; // not needed
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].loop = false;
	blockStackSize++;
}

void core::Engine::skipBlock()
{
	LOG.println("-------- SKIP BLOCK --------");
	
	// skip lines with indentation equal or greater than skipIndentation
	// eg. code in 'if' block.

	skipIndentation = currentIndentation + 1;
}

ERROR_STATUS core::Engine::pushArgListAndExecute(TreeIterator& _it, ICallback * cmd)
{
	// push list of expressions, eg. ( 1, f(x), y )
	// --> pushExprArg("1"), pushExprArg("f(x)"), pushExprArg("y")

	INT numArgs = 0;

	TreeIterator it(_it);
	CHECK(it.isType(NODE_SUBTREE), SYNTAX_ERROR);

	if (!it.hasChild())
	{
		args.reset(0);
		return NO_ERROR; // empty ()
	}
	it.toChild();
	do
	{
		CHECK_CALL(pushExprSubtreeArg(it));
		numArgs ++;
	}
	while(it.next());

	// print args

	if (stack.itemCount() > 0)
	{
		StackIterator argIt(stack);
		LOG.print("args stack");
		do
		{
			LOG.print("\n - ").print(argIt.var());
		}
		while(argIt.next());
		LOG.endl();
	}
	else LOG.println("empty stack");

	args.reset(numArgs);

	CHECK_CALL(cmd->execute(*this, args));

	// pop function arguments after call and push return value
	
	while (numArgs-- > 0) stack.pop();

	if (args.hasReturnValue())
	{
		stack.pushData( args.returnValue.ptr());
	}

	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushAtomicValue(TreeIterator&_it)
{
	// push a value in expression,
	// eg. "1" in "f(1)" or "2" in "f(2+3)" or "(4+5)" in "f((4+5)+6)"

	INT stackSizeBefore = stack.itemCount();
	TreeIterator it(_it);

	LOG.print("push atomic value: ").print(it).endl();

	if (it.isType(NODE_INTEGER) || it.isType(NODE_DOUBLE) || it.isType(NODE_BOOL) || it.isType(NODE_TEXT))
	{
		stack.pushData(it);
	}
	else if(it.isType(NODE_SUBTREE))
	{
		// arg. in (), eg. "(2+3)" in "1 + (2+3)"
		TreeIterator it(_it);
		ASSERT(it.isType(NODE_SUBTREE));
		it.toChild(); // 1 + ( to here <EXPR> )
		CHECK(!it.hasNext(), SYNTAX_ERROR);
		CHECK_CALL(pushExprSubtreeArg(it));
	}
	else if(it.isType(NODE_NAME))
	{
		LOG.print("find variable: ").print(it).endl();

		CHECK_CALL(pushVariable(it));

	}
	else
	{
		ASSERT(false);
	}

	INT stackSizeAfter = stack.itemCount();
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushVariable(TreeIterator& name)
{
	INT index = findVariableIndex(name.getTextData(), constants);
	if (index >= 0)
	{
		TreeIterator out(constants, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	index = findVariableIndex(name.getTextData(), vars);
	if (index >= 0)
	{
		TreeIterator out(vars, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	
	return &VARIABLE_NOT_FOUND;
}

INT core::Engine::findVariableIndex(INT* nameData, Tree& variableMap)
{

	// iterate variables and find name. return true if found.

	TreeIterator it(variableMap);
	if (!it.hasChild())	return -1;
	it.toChild();
	do
	{
		it.toChild(); // first child is the name
		if (matchTextData(it.getTextData(), nameData))
		{
			it.next(); // found! move to data
			return it.index;
		}
		it.toParent();
	}
	while(it.next());
	return -1; // variable not found in the tree
}

ERROR_STATUS core::Engine::pushExprArg(TreeIterator& it)
{
	INT stackSizeBefore = stack.itemCount();
	// 'it' now points to first element of the expression, eg. "x" in "x + 1"

	if (!it.hasNext())
	{
		// eg. "y" in "f(x,y,z)"
		CHECK_CALL(pushAtomicValue(it));
	}
	else
	{
		// next can be
		// a) () -> function call, eg. "f(x,y)"
		// b) operator, eg. "x+y"

		if (it.isNextType(NODE_SUBTREE))
		{
			LOG.println("push function return value");
			auto cmd = findCommand(it.getTextData());
			if (cmd)
			{
				it.next(); // it points to "(...)" in "f(...)"
				CHECK(!it.hasNext(), SYNTAX_ERROR);
				CHECK_CALL(pushArgListAndExecute(it, cmd));
			}
			else
			{
				CHECK(false, UNKNOWN_COMMAND);
			}
		}
		else if (it.isNextType(NODE_OPERATOR))
		{
			LOG.println("int [op] int operator"); // eg. "a + b"

			// get the first value

			CHECK_CALL(pushAtomicValue(it));
			INT a = stack.popInt();

			// read the operator

			it.next();
			CHAR op = '\0';
			it.var().getOp(op);

			// get the second value

			it.next();
			CHECK(!it.hasNext(), SYNTAX_ERROR);
			CHECK_CALL(pushAtomicValue(it));
			INT b = stack.popInt();

			LOG.print("a=").print(a).print(" ").print(op).print(" b=").print(b).endl();
			CHECK_CALL(operatorPush(op, a, b));
		}
		else
		{
			CHECK(false, SYNTAX_ERROR);
		}
	}

	// check that one argument has actually been pushed

	INT stackSizeAfter = stack.itemCount();
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS core::Engine::pushExprSubtreeArg(TreeIterator& _it)
{
	// push an argument that is wrapped in an expression

	TreeIterator it(_it);

	CHECK(it.isType(NODE_EXPR), SYNTAX_ERROR);
	CHECK(it.hasChild(), SYNTAX_ERROR);
	it.toChild(); // 'it' now points to first element of the expression, eg. "x" in "x + 1"
	CHECK_CALL(pushExprArg(it));

	return NO_ERROR;
}
ERROR_STATUS core::Engine::operatorPush(CHAR op, INT a, INT b)
{
	switch(op)
	{
	case '<': stack.pushBool(a <  b); return NO_ERROR;
	case '>': stack.pushBool(a >  b); return NO_ERROR;
	case '=': stack.pushBool(a == b); return NO_ERROR;

	case '+': stack.pushInt (a +  b); return NO_ERROR;
	case '-': stack.pushInt (a -  b); return NO_ERROR;
	case '*': stack.pushInt (a *  b); return NO_ERROR;
	case '/': CHECK(b!=0, DIV_ZERO); stack.pushInt(a / b); return NO_ERROR;
	}
	ERR.printCharSymbol(op);
	return &INVALID_OPERATOR;
}
ICallback * core::Engine::findCommand(INT * textData)
{
	// 'it' points to command name
	INT i;
	for (i=0; i<NUM_COMMANDS; i++)
	{
		if (matchTextData(textData, commands[i].name)) return &commands[i];
	}
	for (i=0; i<numCallbacks; i++)
	{
		if (matchTextData(textData, callbacks[i].name)) return &callbacks[i];
	}
	return 0;
}
bool core::Engine::isReservedName(INT * textData)
{
	if (findCommand(textData) != nullptr) return true;
	if (findVariableIndex(textData, constants) >= 0) return true;
	if (findVariableIndex(textData, vars) >= 0) return true;
	return false;
}
