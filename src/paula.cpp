#include "paula.h"
#include "stream.h"

#ifdef PAULA_EXCEPTIONS
#include <stdexcept>
#endif

using namespace paula;



// PAULA

Paula::Paula() : //buffer(BUFFER_SIZE), index(0)
	automata(*this),
	commands
	{
		Command("print"),
		Command("exit"),
	}
{
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
			LOGLINE("Caught an exception: " << e.what()<<" (id="<<e.id<<")");
		}
	}
	else
#endif
	{
		automata.run(input);
	}
}

Command * paula::Paula::findCommand(Tree& tree)
{
	// get the first child (name)

	// TODO

	for (INT i=0; i<NUM_COMMANDS; i++)
	{
		return &commands[i];
	}
	return 0;
}

void paula::Paula::execute(INT indentation, Tree& tree)
{
	LOGLINE("execute: indentation="<<indentation);

	auto cmd = findCommand(tree);
	if (cmd)
	{
		cmd->execute(*this, tree);
	}

	//BufferInputStream input(buffer, lineStart, index);

	//automata.run(input);
}
