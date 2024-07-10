#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"
#include "command.h"

namespace paula
{
	class IInputStream;
	class Tree;

	const int NUM_COMMANDS = 2;

	class Paula
	{
	public:
		static Paula one; // the one Paula object
		static const POut& log; // log output

		void run(IInputStream&, bool handleException);
		void readCommandArgs(Tree& tree);
		void pushArgList(TreeIterator&);
		void execute(INT indentation, Tree& tree);

	private:

		Paula();

		ByteAutomata automata;

		Tree args;

		ArgDef commandArgDef, functionArgDef, intArgDef;

		Command commands[NUM_COMMANDS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}