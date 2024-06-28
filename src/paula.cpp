#include "paula.h"
#include "stream.h"
using namespace paula;

void paula::assert(bool x, const char * msg)
{
	if (!x)
	{
		std::cout<<msg<<std::endl;
		HALT;
	}
}

Paula::Paula() : buffer(BUFFER_SIZE), index(0)
{
}

const int
	STATE_INDENTATION = 0,
	STATE_EXPRESSION = 1;

void Paula::run(IInputStream& input)
{
	LOGLINE("RUN STRING: ");
	
	index = 0;
	lineStart = 0;
	state = STATE_INDENTATION;
	
	while(!input.end())
	{
		scan(input.read());
	}
	if (index > lineStart)
	{
		endExpression();
	}
}

void paula::Paula::scan(CHAR c)
{
	LOGLINE("SCAN "<<c<<" state:"<<state<<" ind:"<<indentation);

	switch(state)
	{
	case STATE_INDENTATION:

		if (c == '\t') indentation ++;
		else if (IS_CHAR(c)) startExpression();
		else if (c == '\n') indentation = 0;
		else CHECK(false, "invalid character at the beginning of a line: '"<<c<<"'");
		break;

	case STATE_EXPRESSION:

		if (c == '\n') endExpression();
		buffer[index++] = c;
		break;

	default:
		ASSERT(false, "scan: unhandled state");
	};
}

void paula::Paula::startExpression()
{
	state = STATE_EXPRESSION;
	index = 0;
	lineStart = 0;
}

void paula::Paula::endExpression()
{
	ASSERT(index > lineStart, "");

	execute();

	state = STATE_INDENTATION;
}

void paula::Paula::execute()
{
	LOGLINE("execute: "<<lineStart<<"..."<<index);

	BufferInputStream input(buffer, lineStart, index);

	automata.run(input);
}
