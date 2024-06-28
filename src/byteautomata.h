#pragma once
#include "defs.h"
#include "array.h"
#include "stream.h"

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
		void (*actions[64])();
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
		// declarations
		ByteAutomata();
		BYTE addState(const CHAR *);
		void transition(BYTE state, const CHAR *, void (* action)());
		void fillTransition(BYTE state, void (* action)());
		BYTE addAction(void (* action)());
		void next(BYTE nextState);
		void print();
		void stay();
		void printError();
		//std::string getString(INT,INT);
		bool step(BYTE input);
		INT getIndex();
		BYTE getInputByte();
		void run(IInputStream & input);
		~ByteAutomata();
	};
}