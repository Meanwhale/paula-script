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
		void run(IInputStream & input);

	private:

		bool ok;
		Array<BYTE> tr;
		BYTE currentInput;
		BYTE currentState;
		void (*actions[64])(ByteAutomata*);
		BYTE stateCounter;
		BYTE actionCounter; // 0 = end
		// running:
		BYTE inputByte = 0;
		INT index = 0;
		INT lineNumber = 0;
		bool stayNextStep = false;
		bool running = false;
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
		void addIntegerToken();
		void addNameToken();
		void prepareAddToken();
        void addOperatorToken();
		void next(BYTE nextState);
		void print();
		void printError();
        void printTreeStack();
        void pushTree(INT parent);
		void popTree();
		//std::string getString(INT,INT);
		bool step(BYTE input);
		INT getIndex();

		// state machine functions
		
		void stay();
		void addLiteralToken(INT tokenType);
		void addTextToken();
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
		BYTE stateStart, stateSpace, stateName, stateFirstName, statePostName, stateNumber;
		
		// hide
		ByteAutomata() = delete;
		ByteAutomata& operator=(const ByteAutomata&) = delete;
	};
}