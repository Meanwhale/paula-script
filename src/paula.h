#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"
#include "command.h"

namespace paula
{
	class IInputStream;
	class Tree;

	constexpr int
		NUM_COMMANDS = 3,
		MAX_BLOCK_DEPTH = 16;

	struct Block
	{
		INT startAddress, indentation;
		bool loop;
	};

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
        ERROR_STATUS lineIndentationInit(INT indentation, bool& outLoop);
        ERROR_STATUS executeLine(INT indentation, INT lineStartIndex, INT lineType, Tree& tree);

		void startLoop();

		void skipBlock();

	private:

		Paula();

		INT currentIndentation, loopIndentation, blockStackSize, lineStartIndex;

		ByteAutomata automata;

		Block blockStack[MAX_BLOCK_DEPTH];

		Tree args, constants;

		Command commands[NUM_COMMANDS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}