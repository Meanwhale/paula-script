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
		
		ByteAutomata(Paula&);
		BYTE addState(const CHAR *);
		void transition(BYTE state, const CHAR *, void (* action)(ByteAutomata*));
		void fillTransition(BYTE state, void (* action)(ByteAutomata*));
		BYTE addAction(void (* action)(ByteAutomata*));
		void startAssignment();
		void startFunction();
		void next(BYTE nextState);
		void print();
		void printError();
        void printTreeStack();
        void pushTree(INT parent);
		void popTree();
		//std::string getString(INT,INT);
		bool step(BYTE input);
		INT getIndex();
		BYTE getInputByte();
		void run(IInputStream & input);
		~ByteAutomata();

		// state machine functions
		
		void stay();
		void addToken(INT tokenType);
		void addTextToken();
		void lineBreak();
		void comma();
		void startBlock();
		void endBlock();
		void newLine();
		void startExpr(BYTE firstState);
		void defineTransitions();

	private:
		INT currentParent();
		Paula& paula;
		INT lastStart, indentation, lineType, treeStackTop;
		BYTE stateStart, stateSpace, stateName, stateFirstName, statePostName, stateNumber;
	};
}