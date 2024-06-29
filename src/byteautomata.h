#pragma once
#include "defs.h"
#include "array.h"
#include "stream.h"
#include "tree.h"

#include <iostream>
namespace paula
{
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
		Array<const CHAR *> stateNames;
		Tree tree;

		// declarations
		
		ByteAutomata();
		BYTE addState(const CHAR *);
		void transition(BYTE state, const CHAR *, void (* action)(ByteAutomata*));
		void fillTransition(BYTE state, void (* action)(ByteAutomata*));
		BYTE addAction(void (* action)(ByteAutomata*));
		void next(BYTE nextState);
		void print();
		void printError();
		//std::string getString(INT,INT);
		bool step(BYTE input);
		INT getIndex();
		BYTE getInputByte();
		void run(IInputStream & input);
		~ByteAutomata();

		// state machine functions
		
		void stay();
		void addExpr();
		void addToken(INT tokenType);
		void addTextToken();
		void exprBreak();
		void addBlock();
		void endBlock();
		void defineTransitions();

	private:
		INT treeParent;
		BYTE stateSpace, stateName, stateNumber;
		INT lastStart;
	};
}