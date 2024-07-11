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
	extern const POut& log; // log output
	extern const POut& err; // error output
	extern const POut& user; // print output

	class Paula
	{
	public:
		static Paula one; // the one Paula object

		void run(IInputStream&, bool handleException);
		void readCommandArgs(Tree& tree);
		void pushArgList(TreeIterator&);
		void pushSingleValue(TreeIterator& _it);
		void pushOneArg(TreeIterator&);
		INT operate(CHAR op, INT a, INT b);
		void pushOneSubtreeArg(TreeIterator& _it);
		void execute(INT indentation, Tree& tree);

	private:

		Paula();

		ByteAutomata automata;

		Tree args, vars;

		ArgDef commandArgDef, singleArgDef, OperatorArgDef;

		Command commands[NUM_COMMANDS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}