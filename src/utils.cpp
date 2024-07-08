#include "utils.h"
#include "array.h"

#ifdef PAULA_EXCEPTIONS
#include <stdexcept>
#endif

// UTILS

void paula::assert(bool x, const char * msg, Error e)
{
	if (!x)
	{
#ifdef PAULA_EXCEPTIONS
		throw PaulaException(msg,e);
#else
		std::cout<<msg<<std::endl;
#endif
		HALT;
	}
}

void paula::logChar(CHAR c)
{
	if (c>=32 && c<127) std::cout<<c; // printable
	else std::cout<<'#'<<static_cast<unsigned int>(static_cast<unsigned char>(c)); // control char: print number
}

void paula::bytesToInts(const unsigned char * bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength)
{
	// order: 0x04030201

	// bytes:	b[3] b[2] b[1] b[0] b[7] b[6] b[5] b[4]...
	// ints:	_________i[0]______|_________i[1]______...

	int shift = 0;
	ints[intsOffset] = 0;
	for (int i = 0; i <= bytesLength;)
	{
		auto theByte = i == bytesLength ? '\0' : bytes[bytesOffset + i]; // add ending character to the end
		ints[(i / 4) + intsOffset] += (theByte & 0x000000FF) << shift;

		i++;
		if (i % 4 == 0)
		{
			shift = 0;
			if (i <= bytesLength)
			{
				ints[(i / 4) + intsOffset] = 0;
			}
		}
		else shift += 8;
	}
}