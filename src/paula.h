#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"
#include "command.h"

namespace paula
{
	class IInputStream;
	class Tree;

	const int BUFFER_SIZE = 1024;
	const int NUM_COMMANDS = 2;

	class Paula
	{
	public:
		Paula();
		void run(IInputStream&, bool handleException);
		void execute(INT indentation, Tree& tree);

	private:

		ByteAutomata automata;
		//Array<CHAR> buffer;
		//INT index, lineStart, state, indentation;

		Command commands[NUM_COMMANDS];

		Command* findCommand(Tree& tree);
	};
}