#include "paula.h"
#include "stream.h"

using namespace paula;

constexpr INT ARG_STACK_SIZE = 1024;
constexpr INT VARS_SIZE = 1024;

// PAULA

void printAction (Paula&,Tree&args)
{
	LOG.println("printAction");
	TreeIterator it(args);
	if (!it.hasChild()) { LOG.println("<empty>"); return; }
	args.printCompact(it);
	LOG.println("");
}
void fooAction (Paula&p,Tree&args)
{
	LOG.println("fooAction");
	args.pop(0);
	args.pop(0);
	args.pushInt(0,555);
}

Paula Paula::one = Paula();

// outputs
STDOut stdOut = STDOut();
STDErr stdErr = STDErr();
STDOut stdPrint = STDOut();
const POut& paula::log = stdOut;
const POut& paula::err = stdErr;
const POut& paula::user = stdPrint;

Paula::Paula() : //buffer(BUFFER_SIZE), index(0)
	automata(*this),
	args(ARG_STACK_SIZE),
	vars(VARS_SIZE),
	commands
	{
		Command("print", printAction),
		Command("foo", fooAction),
	}
	//commandArgDef(2),
	//singleArgDef(1),
	//OperatorArgDef(3)
{
	LOG.println("---------------- NEW PAULA ----------------");
	log.print("toimii!").endl();

	args.init(NODE_STACK);
	vars.init(NODE_SUBTREE);

	//commandArgDef.types[0] = NODE_NAME;
	//commandArgDef.types[1] = NODE_SUBTREE;

	//singleArgDef.types[0] = NODE_ANY_DATA;

	//OperatorArgDef.types[0] = NODE_ANY_DATA;
	//OperatorArgDef.types[1] = NODE_OPERATOR;
	//OperatorArgDef.types[2] = NODE_ANY_DATA;
}

const int
	STATE_INDENTATION = 0,
	STATE_EXPRESSION = 1;

ERROR_STATUS Paula::run(IInputStream& input, bool handleErrors)
{
	LOG.println("RUN STRING: ");

	auto error = automata.run(input);

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


ERROR_STATUS paula::Paula::executeLine(INT indentation, Tree& tree)
{
	args.init(NODE_STACK);

	if (tree.getType(0) == NODE_ASSIGNMENT)
	{
		// TRG : SRC

		LOG.print("execute ASSIGNMENT: indentation=").print(indentation).endl();
		INT kvIndex = vars.addSubtree(0, NODE_KV);
		TreeIterator it(tree);
		it.toChild(); // points to variable name
		LOG.print("variable name: "); it.print(true); LOG.endl();
		vars.addData(kvIndex, it); // add variable name to KV
		it.next(); // move to SRC
		LOG.print("start: "); it.print(true); LOG.endl();
		CHECK_CALL(pushAtomicValue(it)); // TODO: assign expression
		auto src = TreeIterator(args, args.stackTopIndex(0));
		LOG.print("assign value: "); src.print(true); LOG.endl();
		vars.addData(kvIndex, src); // add value to KV
		args.pop(0);
		vars.print();
	}
	else if (tree.getType(0) == NODE_COMMAND)
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

			// read command arguments

			CHECK_CALL(pushArgList(it));

			cmd->execute(*this, args);
		}
		else
		{
			CHECK(false, UNKNOWN_COMMAND);
		}
	}
	else
	{
		ASSERT(false, "Paula::execute: not an executable tree");
	}
	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushArgList(TreeIterator& _it)
{
	// push list of expressions, eg. ( 1, f(x), y )
	// --> pushExprArg("1"), pushExprArg("f(x)"), pushExprArg("y")

	TreeIterator it(_it);
	CHECK(it.isType(NODE_SUBTREE), SYNTAX_ERROR);

	if (!it.hasChild()) return NO_ERROR; // empty ()
	it.toChild();
	do
	{
		CHECK_CALL(pushExprArg(it));
	}
	while(it.next());

	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushAtomicValue(TreeIterator&_it)
{
	// push a value in expression,
	// eg. "1" in "f(1)" or "2" in "f(2+3)" or "(4+5)" in "f((4+5)+6)"

	INT stackSizeBefore = args.stackSize(0);
	TreeIterator it(_it);

	LOG.print("push atomic value: "); it.print(true); LOG.endl();

	if (it.isType(NODE_INTEGER))
	{
		args.pushData(0,it);
	}
	else if(it.isType(NODE_SUBTREE))
	{
		// arg. in (), eg. "(2+3)" in "1 + (2+3)"
		TreeIterator it(_it);
		ASSERT(it.isType(NODE_SUBTREE), "");
		it.toChild(); // 1 + ( to here <EXPR> )
		CHECK(!it.hasNext(), SYNTAX_ERROR);
		CHECK_CALL(pushExprArg(it));
	}
	else if(it.isType(NODE_NAME))
	{
		LOG.print("find variable: "); it.print(true); LOG.endl();

		CHECK_CALL(pushVariable(it));

	}
	else
	{
		ASSERT(false, "unhandled value node");
	}

	INT stackSizeAfter = args.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}

ERROR_STATUS paula::Paula::pushVariable(TreeIterator& name)
{
	// iterate variables and find
	TreeIterator it(vars);
	if (!it.hasChild())
	{
		return &VARIABLE_NOT_FOUND;
	}
	it.toChild();

	do
	{
		it.toChild(); // first child is the name
		if (it.matchTextData(name.getTextData()))
		{
			// found!
			it.next();
			pushAtomicValue(it);
			return NO_ERROR;
		}
		it.toParent();
	}
	while(it.next());

	return &VARIABLE_NOT_FOUND;
}

ERROR_STATUS paula::Paula::pushExprArg(TreeIterator& _it)
{
	INT stackSizeBefore = args.stackSize(0);

	TreeIterator it(_it);

	CHECK(it.isType(NODE_EXPR), SYNTAX_ERROR);
	CHECK(it.hasChild(), SYNTAX_ERROR);
	it.toChild();

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
				CHECK_CALL(pushArgList(it));
				cmd->execute(*this, args);
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
			INT a = args.popInt(0);

			// read the operator

			it.next();
			ASSERT(it.isType(NODE_OPERATOR),"");
			CHAR op = it.getOp();

			// get the second value

			it.next();
			CHECK_CALL(pushAtomicValue(it));
			INT b = args.popInt(0);

			LOG.print("a=").print(a).print(" b=").print(b).endl();
			INT result;
			CHECK_CALL(operate(op, a, b, result));
			args.pushInt(0, result);
		}
		else
		{
			CHECK(false, SYNTAX_ERROR);
		}
	}

	// check that one argument has actually been pushed

	INT stackSizeAfter = args.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, EMPTY_ARGUMENT_VALUE);
	return NO_ERROR;
}
ERROR_STATUS paula::Paula::operate(CHAR op, INT a, INT b, INT&out)
{
	switch(op)
	{
	case '+': out = a + b; return NO_ERROR;
	case '-': out = a - b; return NO_ERROR;
	case '*': out = a * b; return NO_ERROR;
	case '/': CHECK(b!=0, DIV_ZERO); out = a / b; return NO_ERROR;
	}
	ERR.printCharSymbol(op);
	return &INVALID_OPERATOR;
}
Command * paula::Paula::findCommand(TreeIterator& it)
{
	// 'it' points to command name

	for (INT i=0; i<NUM_COMMANDS; i++)
	{
		if (it.matchTextData(commands[i].name)) return &commands[i];
	}
	return 0;
}
