#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"
#include "command.h"

namespace paula
{
	class IInputStream;
	class Tree;

	const int NUM_COMMANDS = 3;

	class Paula
	{
	public:
		static Paula one; // the one Paula object

		Tree vars;

		ERROR_STATUS run(IInputStream&, bool handleException);
		ERROR_STATUS pushArgList(TreeIterator&);
		ERROR_STATUS pushAtomicValue(TreeIterator& _it);
		ERROR_STATUS pushVariable(TreeIterator& name);
		INT findVariableIndex(TreeIterator& name, Tree& tree);
		//bool pushVariable(TreeIterator& name, Tree&tree);
		ERROR_STATUS pushExprArg(TreeIterator& _it);
		ERROR_STATUS pushExprSubtreeArg(TreeIterator&);
		ERROR_STATUS operate(CHAR op, INT a, INT b, INT& out);
		ERROR_STATUS executeLine(INT indentation, INT lineType, Tree& tree);

	private:

		Paula();

		ByteAutomata automata;

		Tree args, constants;

		Command commands[NUM_COMMANDS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}