#include "paula.h"
#include "stream.h"

#ifdef PAULA_EXCEPTIONS
#include <stdexcept>
#endif

using namespace paula;

constexpr INT ARG_STACK_SIZE = 1024;

// PAULA

void printAction (Paula&,Tree&args)
{
	LOGLINE("printAction");
	TreeIterator it(args);
	if (!it.hasChild()) { LOGLINE("<empty>"); return; }
	args.printCompact(it);
	LOGLINE("");
}
void fooAction (Paula&p,Tree&args)
{
	LOGLINE("fooAction");
	args.pop(0);
	args.pop(0);
	args.pushInt(0,555);
}

Paula Paula::one = Paula();
STDOut stdOut = STDOut();
const POut& Paula::log = stdOut;

Paula::Paula() : //buffer(BUFFER_SIZE), index(0)
	automata(*this),
	args(ARG_STACK_SIZE),
	commands
	{
		Command("print", printAction),
		Command("foo", fooAction),
	},
	commandArgDef(2),
	functionArgDef(2),
	intArgDef(1)
{
	LOGLINE("---------------- NEW PAULA ----------------");
	log.print("toimii!").endl();

	args.init(NODE_STACK);

	commandArgDef.types[0] = NODE_NAME;
	commandArgDef.types[1] = NODE_SUBTREE;

	functionArgDef.types[0] = NODE_TEXT;
	functionArgDef.types[1] = NODE_SUBTREE;

	intArgDef.types[0] = NODE_INTEGER;
}

const int
	STATE_INDENTATION = 0,
	STATE_EXPRESSION = 1;

void Paula::run(IInputStream& input, bool handleException)
{
	LOGLINE("RUN STRING: ");

#ifdef PAULA_EXCEPTIONS
	if (handleException)
	{
		try
		{
			automata.run(input);
		}
		catch (const PaulaException& e)
		{
			LOGLINE("Caught an exception: " << e.error.name<<" (id="<<e.error.id<<")");
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
		LOGLINE("execute ASSIGNMENT: indentation="<<indentation);
	}
	else if (tree.getType(0) == NODE_COMMAND)
	{
		LOGLINE("execute COMMAND: indentation="<<indentation);

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
	LOGLINE("readCommandArgs");
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
		if (intArgDef.match(it))
		{
			LOGLINE("push integer");
			args.pushData(0,it);
		}
		else if (functionArgDef.match(it))
		{
			LOGLINE("push function return value");
			auto cmd = findCommand(it);
			if (cmd)
			{
				TreeIterator argIt(it); // iterator to point to argument list, eg. '1', if args. are (1,2)
				argIt.next();
				argIt.toChild();

				INT stackSizeBefore = args.stackSize(0);

				pushArgList(argIt);
				cmd->execute(*this, args);

				INT stackSizeAfter = args.stackSize(0);

				CHECK(stackSizeBefore + 1 == stackSizeAfter, FUNCTION_DID_NOT_RETURN_A_VALUE);
			}
			else
			{
				CHECK(false, UNKNOWN_COMMAND);
			}
		}
		else
		{
			CHECK(false, UNKNOWN_EXPRESSION);
		}
		it.toParent();
	}
	while(it.next());
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
