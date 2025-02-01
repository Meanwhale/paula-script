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

		constexpr INT
			NUM_COMMANDS = 8,
			MAX_USER_CALLBACKS = 16,
			MAX_SCRIPT_PROCEDURES = 16,
			MAX_BLOCK_DEPTH = 16;

		constexpr INT
			BLOCK_TYPE_LOOP = 1001,
			BLOCK_TYPE_CONDITIONAL = 1002,
			BLOCK_TYPE_PROCEDURE = 1003;

		struct Block
		{
			INT startBytecodeIndex, indentation, blockType;
			INT* argsBasePtr;
		};

		class Engine
		{
		public:

			static Engine one; // the one Paula object

			void startLoop();
			void startIf();
			void startProcedure();
			void skipBlock();
			ERROR_STATUS addParsedLine();
			void runSafe(IInputStream&);
			void runSafe(IInputStream&, const char **args, int numArgs);
			ERROR_STATUS runBytecode();
			ERROR_STATUS parse(IInputStream&);
			ERROR_STATUS run(IInputStream&);
			ERROR_STATUS run(IInputStream&, const char **args, int numArgs);
			ERROR_STATUS addCallback(const char* callbackName, const Error* (*_action)(Args&));
			ERROR_STATUS jump(INT bytecodeIndex);
			ERROR_STATUS returnProcedure();
			ERROR_STATUS callProcedure(INT address, Args&args);
			ERROR_STATUS addProcedure(char*name, INT address);
			void printInfo();
			const char * blockTypeName(INT);

			Tree vars;
			bool oneLiner, skipNextAfterJump;
			Args globalArgs; // args of current script procedure (function) = CLI args on base level

			INT currentIndentation, skipIndentation, blockStackSize, bytecodeIndex, numCallbacks, numProcedures, jumpIndex;

			friend class ByteAutomata;
			friend class Args;

		//private:
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
