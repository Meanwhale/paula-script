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
			NUM_COMMANDS = 7,
			MAX_USER_CALLBACKS = 16,
			MAX_SCRIPT_PROCEDURES = 16,
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
			ERROR_STATUS run(IInputStream&, const char **args, int numArgs, bool handleError);
			ERROR_STATUS returnHandleError(const Error* error, bool handleErrors);
			ERROR_STATUS addCallback(const char* callbackName, const Error* (*_action)(Args&));
			ERROR_STATUS jump(INT bytecodeIndex);
			ERROR_STATUS callProcedure(INT address, Args&args);
			ERROR_STATUS addProcedure(char*name, INT address);

			Tree vars;
			bool oneLiner;
			Args globalArgs;

			INT currentIndentation, skipIndentation, blockStackSize, bytecodeIndex, numCallbacks, numProcedures, jumpIndex;

			friend class ByteAutomata;
			friend class Args;

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
			ERROR_STATUS executeLine(INT indentation, INT _bytecodeIndex, INT lineType, Tree& tree);


			ByteAutomata automata;

			Block blockStack[MAX_BLOCK_DEPTH];

			Stack stack;
			Tree constants, bytecode;

			Stack returnValue;
			Command commands[NUM_COMMANDS];
			Callback callbacks[MAX_USER_CALLBACKS];
			ProcedureCallback procedures[MAX_SCRIPT_PROCEDURES];

			ICallback* findCommand(INT* textData);

			bool isReservedName(INT* textData);

			// hide
			Engine& operator=(const Engine&) = delete;
		};
	}
}
