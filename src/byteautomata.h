#pragma once
#include "defs.h"
#include "array.h"
#include "stream.h"
#include "tree.h"

#include <iostream>
//#include <vector>

namespace paula { namespace core
{
	class Engine;

	class ByteAutomata
	{
	public:

		ByteAutomata(Engine&);
		void init(IInputStream* _input);
		bool running();
		void step();
		void run(IInputStream* input);
		void clearBuffer();
		void jump(INT address);
		const Error* getError();

	private:

		void step(BYTE);
		void closeInput();
		void uninit();

		Engine& paula;
		IInputStream* input;
		const Error* error;
		Tree tree;
		
		Array<BYTE> tr;
		Array<BYTE> buffer;
		Array<BYTE> quoteBuffer;
		Array<INT> treeStack;
		Array<const CHAR *> stateNames;

		BYTE currentInput;
		BYTE currentState;
		void (*actions[64])(ByteAutomata*);
		BYTE stateCounter;
		BYTE actionCounter; // 0 = end
		// running:
		INT readIndex = -1;
		INT bufferIndex = 0;
		INT commandStartIndex = 0;
		INT lineNumber = 0;
		bool stayNextStep = false;
		bool oneLiner = true;

		// declarations
		
		BYTE addState(const CHAR *);
		//BYTE transition(BYTE state, std::vector<std::string>, void(*action)(ByteAutomata*));
		void fillTransition(BYTE state, const CHAR* input, BYTE actionIndex);
		BYTE transition(BYTE state, const CHAR *, void (* action)(ByteAutomata*));
		BYTE fillTransition(BYTE state, void (* action)(ByteAutomata*));
		BYTE addAction(void (* action)(ByteAutomata*));
		void startAssignment();
		void startFunction();
		INT parseInt(Array<BYTE>& src, INT firstByte, INT lastByte);
		double parseDouble(Array<BYTE>& src, INT i, INT lastByte);
		void addQuoteByte(BYTE);
		void addQuote();
        void addTokenAndTransitionToSpace();
		void prepareAddToken();
        void addOperatorToken();
		void addFirstNameAndTransit();
		void next(BYTE nextState);
        void nextCont(BYTE nextState);
		//void print();
		void printError();
        void printTreeStack();
		void pushTree(INT parent);
		void popTree();

		// state machine functions

		void stay();
		void addLiteralToken(INT tokenType);
		void breakCommand();
		void startNewLine();
		void breakLine();
		void comma();
		void startBlock();
		void endBlock();
        void eof();
		void executeCommand();
		void resetCommand();
		void startExpr(BYTE firstState);
		void defineTransitions();

		INT currentParent();
		INT lastStart, indentation, commandType, treeStackTop, quoteIndex;
		BYTE
			stateNewLine,
			stateNewCommand,
			stateSpace,
			stateName,
			stateFirstName,
			stateAfterFirstName,
			statePostName,
			stateNumber,
			stateDecimal,
			stateQuote;
		
		// hide
		ByteAutomata() = delete;
		ByteAutomata& operator=(const ByteAutomata&) = delete;
	};
}}