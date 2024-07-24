#include "utils.h"
#include "array.h"


// UTILS

void paula::trap(const char* expr, const char* file, int line, const char* msg)
{
	paula::err.print("\nFAIL: \"").print(expr).print("\"\nFILE: ").print(file).print("\nLINE: ").print(line).print("\nMSG:  ").print(msg).endl(); 
	HALT;
}

//void paula::assert(bool x, const char * msg)
//{
//	if (!x)
//	{
//		//TODO
//	}
//}


paula::INT paula::charToInt(CHAR c)
{
	return static_cast<unsigned int>(static_cast<unsigned char>(c));
}

paula::INT paula::textDataSize(INT numBytes)
{
	return (numBytes / 4) + 2; // size int + ints with 4 char in one int and \0 at the end
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