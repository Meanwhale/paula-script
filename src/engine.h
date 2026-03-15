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
			NUM_COMMANDS = 10,
			MAX_USER_CALLBACKS = 16,
			MAX_SCRIPT_PROCEDURES = 16,
			MAX_BLOCK_DEPTH = 16;

		constexpr INT
			BLOCK_TYPE_LOOP = 1001,
			BLOCK_TYPE_CONDITIONAL = 1002,
			BLOCK_TYPE_PROCEDURE = 1003,
			BLOCK_TYPE_FUNCTION = 1004;

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
			void startProcedureOrFunction(bool function);
			void skipBlock();
#ifndef PAULA_MINI
			ERROR_STATUS compile(IInputStream&, BinaryOutputStream&);
			ERROR_STATUS parse(IInputStream&);
			ERROR_STATUS addParsedLine();
			ERROR_STATUS runScript(IInputStream&);
			ERROR_STATUS runScript(IInputStream&, const char **args, int numArgs);
			void runSafe(IInputStream&);
			void runSafe(IInputStream&, const char **args, int numArgs);
#endif
            void reset();
			ERROR_STATUS runBytecode(IInputStream&, const char **args, int numArgs);
			ERROR_STATUS runBytecode(INT startIndex);
			ERROR_STATUS addCallback(const char* callbackName, std::function<const Error*(Args&)> _action);
			ERROR_STATUS jump(INT bytecodeIndex);
			ERROR_STATUS backFromProcedureOrFunction(bool function);
			ERROR_STATUS callProcedure(INT address, Args&args, bool function);
			ERROR_STATUS addProcedure(char*name, INT address, bool function);
			void printInfo();
			const char * blockTypeName(INT);

			Tree vars;
			bool oneLiner, skipNextAfterJump, returnCalled;
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

#ifndef PAULA_MINI
			ByteAutomata automata;
#endif

			Block blockStack[MAX_BLOCK_DEPTH];

			Stack stack;
			Tree constants, bytecode;

			Stack returnValue;
			Command commands[NUM_COMMANDS];
			Callback callbacks[MAX_USER_CALLBACKS];
			ProcedureCallback procedures[MAX_SCRIPT_PROCEDURES];

			INT andKeywordData[MAX_VAR_NAME_DATA_LENGTH],
				 orKeywordData[MAX_VAR_NAME_DATA_LENGTH],
				xorKeywordData[MAX_VAR_NAME_DATA_LENGTH];

			ICallback* findCommand(INT* textData);

			bool isReservedName(INT* textData);

			// hide
			Engine& operator=(const Engine&) = delete;
		};
	}
}
