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

		ERROR_STATUS run(IInputStream&, bool handleException);
		ERROR_STATUS pushArgList(TreeIterator&);
		ERROR_STATUS pushAtomicValue(TreeIterator& _it);
		ERROR_STATUS pushVariable(TreeIterator& name);
		ERROR_STATUS pushExprArg(TreeIterator&);
		ERROR_STATUS operate(CHAR op, INT a, INT b, INT& out);
		ERROR_STATUS executeLine(INT indentation, Tree& tree);

	private:

		Paula();

		ByteAutomata automata;

		Tree args, vars;

		//ArgDef commandArgDef, singleArgDef, OperatorArgDef;

		Command commands[NUM_COMMANDS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}