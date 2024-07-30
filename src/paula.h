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
		NUM_COMMANDS = 4,
		MAX_USER_CALLBACKS = 16,
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
		ERROR_STATUS pushArgListAndExecute(TreeIterator&, Command * cmd);
		ERROR_STATUS pushAtomicValue(TreeIterator& _it);
		ERROR_STATUS pushVariable(TreeIterator& name);
		INT findVariableIndex(TreeIterator& name, Tree& tree);
		//bool pushVariable(TreeIterator& name, Tree&tree);
		ERROR_STATUS pushExprArg(TreeIterator& _it);
		ERROR_STATUS pushExprSubtreeArg(TreeIterator&);
		ERROR_STATUS operatorPush(CHAR op, INT a, INT b);
        ERROR_STATUS addCallback(const char* callbackName, const Error* (*_action)(Paula&, Args&));
        ERROR_STATUS lineIndentationInit(INT indentation, bool& executeLine);
        ERROR_STATUS executeLine(INT indentation, INT lineStartIndex, INT lineType, Tree& tree);

		void startLoop();

		void startIf();

		void skipBlock();

	private:

		Paula();

		INT currentIndentation, skipIndentation, blockStackSize, lineStartIndex, numCallbacks;

		ByteAutomata automata;

		Block blockStack[MAX_BLOCK_DEPTH];

		Tree stack, constants;

		Args args;

		Command commands[NUM_COMMANDS];
		Command callbacks[MAX_USER_CALLBACKS];

		Command* findCommand(TreeIterator& it);

		// hide
		Paula& operator=(const Paula&) = delete;
	};
}