#include "paula.h"
#include "stream.h"

using namespace paula;

// PAULA

ERROR_STATUS printAction (Paula&,Args&args)
{
	LOG.println("-------- PRINT ACTION --------");
	//TreeIterator it(args);
	//if (!it.hasChild()) { LOG.println("<empty>"); return NO_ERROR; }
	//args.printCompact(it);
	//LOG.println("");
	return NO_ERROR;
}
ERROR_STATUS notAction (Paula&p,Args&args)
{
	LOG.println("-------- NOT ACTION --------");
	CHECK(args.argCount() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	Data data;
	args.get(0, data);
	bool value = false;
	if(data.getBool(value))
	{
		args.returnBool(!value);
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS whileAction (Paula&p,Args&args)
{
	LOG.println("-------- WHILE ACTION --------");
	CHECK(args.argCount() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	Data data;
	args.get(0, data);
	bool value = false;
	if(data.getBool(value))
	{
		if (value) p.startLoop();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}
ERROR_STATUS ifAction (Paula&p,Args&args)
{
	LOG.println("-------- IF ACTION --------");

	CHECK(args.argCount() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	Data data;
	args.get(0, data);
	bool value = false;
	if(data.getBool(value))
	{
		if (value) p.startIf();
		else p.skipBlock();
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}

Paula Paula::one = Paula();


Paula::Paula() : //buffer(BUFFER_SIZE), index(0)
	currentIndentation(0),
	skipIndentation(-1),
	blockStackSize(0),
	lineStartIndex(0),
	numCallbacks(0),
	automata(*this),
	stack(ARG_STACK_SIZE),
	vars(VARS_SIZE),
	constants(CONSTANTS_SIZE),
	commands
	{
		Command("print", printAction),
		Command("not", notAction),
		Command("while", whileAction),
		Command("if", ifAction)
	},
	args(stack)
	//commandArgDef(2),
	//singleArgDef(1),
	//OperatorArgDef(3)
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

	//commandArgDef.types[0] = NODE_NAME;
	//commandArgDef.types[1] = NODE_SUBTREE;

	//singleArgDef.types[0] = NODE_ANY_DATA;

	//OperatorArgDef.types[0] = NODE_ANY_DATA;
	//OperatorArgDef.types[1] = NODE_OPERATOR;
	//OperatorArgDef.types[2] = NODE_ANY_DATA;
}


ERROR_STATUS Paula::run(IInputStream& input, bool handleErrors)
{
	LOG.println("Paula::run");

	stack.init(NODE_STACK);
	vars.init(NODE_SUBTREE);

	currentIndentation = 0;
	skipIndentation = -1;
	blockStackSize = 0;

	automata.run(&input);
	auto error = automata.getError();
	if (handleErrors && error != NO_ERROR)
	{
		ERR
			.print("Caught an exception: ")
			.print(error->name)
			.print(" (id=")
			.print(error->id)
			.print(")")
			.endl();

		return NO_ERROR;
	}
	else
	{
		return error;
	}
}

ERROR_STATUS paula::Paula::addCallback(const char* callbackName, const Error * (* _action)(Paula&,Args&))
{
	if (numCallbacks >= MAX_USER_CALLBACKS) return &CALLBACK_ERROR;
	callbacks[numCallbacks].setup(callbackName, _action);
	numCallbacks++;
	return NO_ERROR;
}

ERROR_STATUS paula::Paula::lineIndentationInit(INT indentation, bool& executeLine)
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
				LOG.print("address: ").print(block.startAddress).endl();
				automata.jump(block.startAddress);
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

ERROR_STATUS paula::Paula::executeLine(INT indentation, INT _lineStartIndex, INT lineType, Tree& tree)
{
	bool executeLine = false;
	CHECK_CALL(lineIndentationInit(indentation, executeLine));

	if (!executeLine) return NO_ERROR;

	lineStartIndex = _lineStartIndex;

	stack.init(NODE_STACK);

	if (lineType == LINE_ASSIGNMENT)
	{
		// TRG : SRC

		LOG.print("execute ASSIGNMENT: indentation=").print(indentation).endl();
		TreeIterator it(tree);
		it.toChild(); // points to variable name
		LOG.print("variable name: "); it.print(true); LOG.endl();

		// new or override?

		vars.print();

		INT index = findVariableIndex(it, vars);

		if (index >= 0)
		{
			LOG.println("-------- OVERWRITE VAR --------");
			// variable already exists
			TreeIterator data(vars, index); // points to the data
			LOG.print("old value: "); data.print(true); LOG.endl();

			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			TreeIterator src(stack, stack.stackTopIndex(0));
			LOG.print("overwrite value: "); src.print(true); LOG.endl();
			if (src.type() == NODE_TEXT || data.type() == NODE_TEXT) return &TEXT_VARIABLE_OVERWRITE;
			data.overwrite(src);
		}
		else
		{
			LOG.println("-------- NEW VAR --------");
			INT kvIndex = vars.addSubtree(0, NODE_KV);
			// new
			vars.addData(kvIndex, it); // add variable name to KV
			it.next(); // move to SRC
			CHECK_CALL(pushExprArg(it));
			TreeIterator src(stack, stack.stackTopIndex(0));
			LOG.print("assign value: "); src.print(true); LOG.endl();
			vars.addData(kvIndex, src); // add value to KV
			stack.pop(0);
		}

		vars.print();
	}
	else if (lineType == LINE_CALL)
	{
		// COMMAND { funcName ( args ) }

		LOG.print("execute COMMAND: indentation=").print(indentation).endl();
		TreeIterator it(tree);
		it.toChild(); // points to command name

		auto cmd = findCommand(it);
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

void paula::Paula::startLoop()
{
	LOG.println("-------- START LOOP --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startAddress = lineStartIndex;
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].loop = true;
	blockStackSize++;
}
void paula::Paula::startIf()
{
	LOG.println("-------- START IF --------");
	ASSERT(blockStackSize>=0 && blockStackSize<MAX_BLOCK_DEPTH);
	blockStack[blockStackSize].startAddress = -123456; // not needed
	blockStack[blockStackSize].indentation = currentIndentation+1;
	blockStack[blockStackSize].loop = false;
	blockStackSize++;
}

void paula::Paula::skipBlock()
{
	LOG.println("-------- SKIP BLOCK (TODO) --------");
	
	// skip lines with indentation equal or greater than skipIndentation
	// eg. code in 'if' block.

	skipIndentation = currentIndentation + 1;
}

ERROR_STATUS paula::Paula::pushArgListAndExecute(TreeIterator& _it, Command * cmd)
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
	if (stack.stackTopIndex(0) > 0)
	{
		TreeIterator argIt(stack, stack.stackTopIndex(0));
		LOG.print("args stack");
		do
		{
			LOG.print("\n - ");
			argIt.print(true);
		}
		while(argIt.next());
		LOG.endl();
	}
	else LOG.println("empty stack");

	args.reset(numArgs);

	CHECK_CALL(cmd->execute(*this, args));

	// pop function arguments after call and push return value
	
	while (numArgs-- > 0) stack.pop(0);

	if (args.hasReturnValue())
	{
		stack.pushData(0, args.returnValue.ptr());
	}

	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushAtomicValue(TreeIterator&_it)
{
	// push a value in expression,
	// eg. "1" in "f(1)" or "2" in "f(2+3)" or "(4+5)" in "f((4+5)+6)"

	INT stackSizeBefore = stack.stackSize(0);
	TreeIterator it(_it);

	LOG.print("push atomic value: "); it.print(true); LOG.endl();

	if (it.isType(NODE_INTEGER) || it.isType(NODE_DOUBLE) || it.isType(NODE_BOOL) || it.isType(NODE_TEXT))
	{
		stack.pushData(0,it);
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
		LOG.print("find variable: "); it.print(true); LOG.endl();

		CHECK_CALL(pushVariable(it));

	}
	else
	{
		ASSERT(false);
	}

	INT stackSizeAfter = stack.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushVariable(TreeIterator& name)
{
	INT index = findVariableIndex(name, constants);
	if (index >= 0)
	{
		TreeIterator out(constants, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	index = findVariableIndex(name, vars);
	if (index >= 0)
	{
		TreeIterator out(vars, index);
		CHECK_CALL(pushAtomicValue(out));
		return NO_ERROR;
	}
	
	return &VARIABLE_NOT_FOUND;
}

INT paula::Paula::findVariableIndex(TreeIterator& name, Tree& variableMap)
{

	// iterate variables and find name. return true if found.

	TreeIterator it(variableMap);
	if (!it.hasChild())	return -1;
	it.toChild();
	do
	{
		it.toChild(); // first child is the name
		if (it.matchTextData(name.getTextData()))
		{
			it.next(); // found! move to data
			return it.getIndex();
		}
		it.toParent();
	}
	while(it.next());
	return -1; // variable not found in the tree
}

ERROR_STATUS paula::Paula::pushExprArg(TreeIterator& it)
{
	INT stackSizeBefore = stack.stackSize(0);
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
			auto cmd = findCommand(it);
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
			INT a = stack.popInt(0);

			// read the operator

			it.next();
			CHAR op = it.getOp();

			// get the second value

			it.next();
			CHECK(!it.hasNext(), SYNTAX_ERROR);
			CHECK_CALL(pushAtomicValue(it));
			INT b = stack.popInt(0);

			LOG.print("a=").print(a).print(" ").print(op).print(" b=").print(b).endl();
			CHECK_CALL(operatorPush(op, a, b));
		}
		else
		{
			CHECK(false, SYNTAX_ERROR);
		}
	}

	// check that one argument has actually been pushed

	INT stackSizeAfter = stack.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushExprSubtreeArg(TreeIterator& _it)
{
	// push an argument that is wrapped in an expression

	TreeIterator it(_it);

	CHECK(it.isType(NODE_EXPR), SYNTAX_ERROR);
	CHECK(it.hasChild(), SYNTAX_ERROR);
	it.toChild(); // 'it' now points to first element of the expression, eg. "x" in "x + 1"
	CHECK_CALL(pushExprArg(it));

	return NO_ERROR;
}
ERROR_STATUS paula::Paula::operatorPush(CHAR op, INT a, INT b)
{
	switch(op)
	{
	case '<': stack.pushBool(0, a <  b); return NO_ERROR;
	case '>': stack.pushBool(0, a >  b); return NO_ERROR;
	case '=': stack.pushBool(0, a == b); return NO_ERROR;

	case '+': stack.pushInt (0, a +  b); return NO_ERROR;
	case '-': stack.pushInt (0, a -  b); return NO_ERROR;
	case '*': stack.pushInt (0, a *  b); return NO_ERROR;
	case '/': CHECK(b!=0, DIV_ZERO); stack.pushInt(0, a / b); return NO_ERROR;
	}
	ERR.printCharSymbol(op);
	return &INVALID_OPERATOR;
}
Command * paula::Paula::findCommand(TreeIterator& it)
{
	// 'it' points to command name
	INT i;
	for (i=0; i<NUM_COMMANDS; i++)
	{
		if (it.matchTextData(commands[i].name)) return &commands[i];
	}
	for (i=0; i<numCallbacks; i++)
	{
		if (it.matchTextData(callbacks[i].name)) return &callbacks[i];
	}
	return 0;
}
