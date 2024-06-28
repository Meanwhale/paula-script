#pragma once

#include "defs.h"
#include "array.h"
#include "byteautomata.h"

namespace paula
{
	class IInputStream;

	const int BUFFER_SIZE = 1024;

	class Paula
	{
	public:
		Paula();
		void run(IInputStream&);

	private:

		ByteAutomata automata;
		Array<CHAR> buffer;
		INT index, lineStart, state, indentation;

		void scan(CHAR);
		void startExpression();
		void endExpression();
		void execute();
	};
}