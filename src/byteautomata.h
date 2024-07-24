#pragma once
#include "defs.h"
#include "array.h"
#include "stream.h"
#include "tree.h"

#include <iostream>
namespace paula
{
	class Paula;

	class ByteAutomata
	{
	public:

		ByteAutomata(Paula&);
		~ByteAutomata();
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

		IInputStream* input;
		const Error* error;
		Array<BYTE> tr;
		BYTE currentInput;
		BYTE currentState;
		void (*actions[64])(ByteAutomata*);
		BYTE stateCounter;
		BYTE actionCounter; // 0 = end
		// running:
		INT readIndex = -1;
		INT bufferIndex = 0;
		INT lineStartIndex = 0;
		INT lineNumber = 0;
		bool stayNextStep = false;
		Array<BYTE> buffer;
		Array<BYTE> tmp;
		Array<INT> treeStack;
		Array<const CHAR *> stateNames;
		Tree tree;

		// declarations
		
		BYTE addState(const CHAR *);
		void transition(BYTE state, const CHAR *, void (* action)(ByteAutomata*));
		void fillTransition(BYTE state, void (* action)(ByteAutomata*));
		BYTE addAction(void (* action)(ByteAutomata*));
		void startAssignment();
		void startFunction();
		INT parseInt(Array<BYTE>& src, INT firstByte, INT lastByte);
        void addTokenAndTransitionToSpace();
		void prepareAddToken();
        void addOperatorToken();
		void addFirstNameAndTransit();
		void next(BYTE nextState);
		//void print();
		void printError();
        void printTreeStack();
		void pushTree(INT parent);
		void popTree();

		// state machine functions

		void stay();
		void addLiteralToken(INT tokenType);
		void lineBreak();
		void comma();
		void startBlock();
		void endBlock();
		void newLine();
		void startExpr(BYTE firstState);
		void defineTransitions();

		INT currentParent();
		Paula& paula;
		INT lastStart, indentation, lineType, treeStackTop;
		BYTE stateStart, stateSpace, stateName, stateFirstName, stateAfterFirstName, statePostName, stateNumber, stateText;
		
		// hide
		ByteAutomata() = delete;
		ByteAutomata& operator=(const ByteAutomata&) = delete;
	};
}