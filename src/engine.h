#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"
#include "command.h"


namespace paula
{
	class IInputStream;

	namespace core
	{
		class Tree;

		constexpr int
			NUM_COMMANDS = 4,
			MAX_USER_CALLBACKS = 16,
			MAX_BLOCK_DEPTH = 16;

		struct Block
		{
			INT startBytecodeIndex, indentation;
			bool loop;
		};

		class Engine
		{
		public:

			static Engine one; // the one Paula object

			void startLoop();
			void startIf();
			void skipBlock();
			ERROR_STATUS addParsedLine();
			ERROR_STATUS run(IInputStream&, bool handleError);
			ERROR_STATUS returnHandleError(const Error* error, bool handleErrors);
			ERROR_STATUS addCallback(const char* callbackName, const Error* (*_action)(Args&));
			ERROR_STATUS jump(INT bytecodeIndex);
			Tree vars;
			bool oneLiner;

			friend class ByteAutomata;

		private:
			Engine();

			ERROR_STATUS pushArgListAndExecute(TreeIterator&, ICallback * cmd);
			ERROR_STATUS pushAtomicValue(TreeIterator& _it);
			ERROR_STATUS pushVariable(TreeIterator& name);
			INT findVariableIndex(INT* nameData, Tree& tree);
			//bool pushVariable(TreeIterator& name, Tree&tree);
			ERROR_STATUS pushExprArg(TreeIterator& _it);
			ERROR_STATUS pushExprSubtreeArg(TreeIterator&);
			ERROR_STATUS operatorPush(CHAR op, INT a, INT b);
			ERROR_STATUS lineIndentationInit(INT indentation, bool& executeLine);
			ERROR_STATUS executeLine(INT indentation, bool oneLiner, INT _bytecodeIndex, INT lineType, Tree& tree);

			INT currentIndentation, skipIndentation, blockStackSize, bytecodeIndex, numCallbacks, jumpIndex;

			ByteAutomata automata;

			Block blockStack[MAX_BLOCK_DEPTH];

			Tree stack, constants, bytecode;

			Args args;

			Command commands[NUM_COMMANDS];
			Callback callbacks[MAX_USER_CALLBACKS];

			ICallback* findCommand(INT* textData);

			bool isReservedName(INT* textData);

			// hide
			Engine& operator=(const Engine&) = delete;
		};
	}
}