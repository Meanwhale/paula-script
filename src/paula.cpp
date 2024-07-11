#include "paula.h"
#include "stream.h"

#ifdef PAULA_EXCEPTIONS
#include <stdexcept>
#endif

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
	},
	commandArgDef(2),
	singleArgDef(1),
	OperatorArgDef(3)
{
	LOG.println("---------------- NEW PAULA ----------------");
	log.print("toimii!").endl();

	args.init(NODE_STACK);

	commandArgDef.types[0] = NODE_NAME;
	commandArgDef.types[1] = NODE_SUBTREE;

	singleArgDef.types[0] = NODE_ANY_DATA;

	OperatorArgDef.types[0] = NODE_ANY_DATA;
	OperatorArgDef.types[1] = NODE_OPERATOR;
	OperatorArgDef.types[2] = NODE_ANY_DATA;
}

const int
	STATE_INDENTATION = 0,
	STATE_EXPRESSION = 1;

void Paula::run(IInputStream& input, bool handleException)
{
	LOG.println("RUN STRING: ");

#ifdef PAULA_EXCEPTIONS
	if (handleException)
	{
		try
		{
			automata.run(input);
		}
		catch (const PaulaException& e)
		{
			ERR
				.print("Caught an exception: ")
				.print(e.error.name)
				.print(" (id=")
				.print(e.error.id)
				.print(")")
				.endl();
		}
	}
	else
#endif
	{
		automata.run(input);
	}
}


void paula::Paula::execute(INT indentation, Tree& tree)
{
	if (tree.getType(0) == NODE_ASSIGNMENT)
	{
		LOG.print("execute ASSIGNMENT: indentation=").print(indentation).endl();
	}
	else if (tree.getType(0) == NODE_COMMAND)
	{
		LOG.print("execute COMMAND: indentation=").print(indentation).endl();

		CHECK(commandArgDef.match(tree), SYNTAX_ERROR); // check that it's <name> <subtree>, eg. "foo(1)"

		TreeIterator it(tree);
		it.toChild(); // points to command name

		auto cmd = findCommand(it);
		if (cmd)
		{
			readCommandArgs(tree);
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
}

void Paula::readCommandArgs(Tree& tree)
{
	LOG.println("readCommandArgs");
	args.init(NODE_STACK);
	TreeIterator it(tree);
	it.toChild();
	it.next(); // skip command name
	ASSERT(it.isType(NODE_SUBTREE),"");
	if (!it.hasChild())
	{
		return;
	}
	it.toChild();
	// iterate children

	pushArgList(it);

}

void paula::Paula::pushArgList(TreeIterator& _it)
{
	TreeIterator it(_it);
	do
	{
		CHECK(it.isType(NODE_EXPR), SYNTAX_ERROR);
		CHECK(it.hasChild(), SYNTAX_ERROR);
		it.toChild();
		pushOneArg(it);
		it.toParent();
	}
	while(it.next());
}

void paula::Paula::pushSingleValue(TreeIterator&_it)
{
	// push a value in expression,
	// eg. "1" in "f(1)" or "2" in "f(2+3)" or "(4+5)" in "f((4+5)+6)"

	LOG.println("push single value");
	INT stackSizeBefore = args.stackSize(0);
	TreeIterator it(_it);

	if (it.isType(NODE_INTEGER))
	{
		args.pushData(0,it);
	}
	else if(it.isType(NODE_SUBTREE))
	{
		pushOneSubtreeArg(it);
	}
	else
	{
		LOG.print("unhandled value node: ").print(it.type()).endl();
	}

	INT stackSizeAfter = args.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, VALUE_MISSING);
}

void paula::Paula::pushOneArg(TreeIterator&_it)
{
	INT stackSizeBefore = args.stackSize(0);

	TreeIterator it(_it);

	if (singleArgDef.match(it))
	{
		pushSingleValue(it);
	}
	else if (commandArgDef.match(it))
	{
		LOG.println("push function return value");
		auto cmd = findCommand(it);
		if (cmd)
		{
			TreeIterator argIt(it); // iterator to point to argument list, eg. '1', if args. are (1,2)
			argIt.next();
			argIt.toChild();
			pushArgList(argIt);
			cmd->execute(*this, args);
		}
		else
		{
			CHECK(false, UNKNOWN_COMMAND);
		}
	}
	else if (OperatorArgDef.match(it))
	{
		LOG.println("int [op] int operator"); // eg. "a + b"
		
		// get the first value

		pushSingleValue(it);
		INT a = args.popInt(0);

		// read the operator

		it.next();
		ASSERT(it.isType(NODE_OPERATOR),"");
		CHAR op = it.getOp();

		// get the second value

		it.next();
		pushSingleValue(it);
		INT b = args.popInt(0);

		LOG.print("a=").print(a).print(" b=").print(b).endl();
		args.pushInt(0, operate(op, a, b));
	}
	//else if (IntSubtreeOperatorArgDef.match(it))
	//{
	//	LOG.println("int [op] subtree operator");
	//	INT a = it.getInt();
	//	it.next();
	//	ASSERT(it.isType(NODE_OPERATOR),"");
	//	it.next();
	//	pushOneSubtreeArg(it);
	//	INT b = args.popInt(0);
	//	LOG.println("a="<<a<<" b="<<b);
	//	args.pushInt(0, a + b);
	//}
	else
	{
		LOG.println("expression type not found:");
		it.toParent();
		it.printTree(true);
		CHECK(false, UNKNOWN_EXPRESSION);
	}
	INT stackSizeAfter = args.stackSize(0);
	CHECK(stackSizeBefore + 1 == stackSizeAfter, VALUE_MISSING);
}
INT paula::Paula::operate(CHAR op, INT a, INT b)
{
	switch(op)
	{
	case '+': return a + b;
	case '-': return a - b;
	case '*': return a * b;
	case '/': return a / b;
	}
	CHECK(false,INVALID_OPERATOR);
}
void paula::Paula::pushOneSubtreeArg(TreeIterator&_it)
{
	// arg. in (), eg. "(2+3)" in "1 + (2+3)"
	TreeIterator it(_it);
	ASSERT(it.isType(NODE_SUBTREE), "");
	it.toChild(); // 1 + ( to here <EXPR> )
	CHECK(it.isType(NODE_EXPR), SYNTAX_ERROR);
	CHECK(!it.hasNext(), SYNTAX_ERROR);
	CHECK(it.hasChild(), SYNTAX_ERROR);
	it.toChild(); // 1 + ( < to here EXPR> )
	pushOneArg(it);
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
