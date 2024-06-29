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

void paula::logChar(CHAR c)
{
	if (c>=32 && c<127) std::cout<<c; // printable
	else std::cout<<'#'<<static_cast<unsigned int>(static_cast<unsigned char>(c)); // control char: print number
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
		index--; // index would else point to the line break
		endExpression();
	}
}

void paula::Paula::scan(CHAR c)
{
	LOG("SCAN ");
	LOGCHAR(c);
	LOGLINE(" state:"<<state<<" ind:"<<indentation);

	switch(state)
	{
	case STATE_INDENTATION:

		if (c == '\t') indentation ++;
		else if (IS_CHAR(c)) startExpression(c);
		else if (c == '\n') indentation = 0;
		else
		{
			LOGERROR("invalid character at the beginning of a line:");
			LOGCHAR(c);
			CHECK(false, "");
		}
		break;

	case STATE_EXPRESSION:

		if (c == '\n')
		{
			index--; // index would else point to the line break
			endExpression();
		}
		else
		{
			buffer[index++] = c;
		}
		break;

	default:
		ASSERT(false, "scan: unhandled state");
	};
}

void paula::Paula::startExpression(CHAR c)
{
	state = STATE_EXPRESSION;
	buffer[0] = c; // set the first character
	index = 1;
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
