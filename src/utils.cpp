#include "utils.h"
#include "array.h"
#include "tree.h"

using namespace paula;

// UTILS

void paula::trap(const char* expr, const char* file, int line, const char* msg)
{
	paula::err.print("\nFAIL: \"").print(expr).print("\"\nFILE: ").print(file).print("\nLINE: ").print(line).print("\nMSG:  ").print(msg).endl(); 
	HALT;
}

LONG paula::doubleToLongFormat(double value)
{
	// NOTE: these conversions give 'strict-aliasing' warnings. Not even reinterpret_cast help...

	return ((LONG&)(*(&value)));
}
DOUBLE paula::longToDoubleFormat(LONG value)
{
	return ((DOUBLE&)(*(&value)));
}
INT paula::longHighBits(LONG x)
{
	return (INT)(x>>32);
}
INT paula::longLowBits(LONG x)
{
	return (INT)x;
}
LONG paula::intsToLong(INT high, INT low)
{
	LONG x = ((LONG)high) << 32;
	x |= ((LONG)low) & 0x00000000ffffffffL;
	return x;
}

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
const char* paula::treeTypeName(INT tag)
{
	switch(tag)
	{
	case NODE_SUBTREE: return "<subtree>";
	case NODE_EXPR: return "<expr>";
	case NODE_STATEMENT: return "<statement>";

	case NODE_STACK: return "<stack>";
	case NODE_MAP: return "<map>";
	case NODE_KV: return "<key-value>";
	}
	return "<! ! ! error ! ! !>";
}

// data access

bool paula::match(const INT* ptr, INT tag)
{
	// check data type match
	return (tag & TAG_MASK) == (*ptr & TAG_MASK);
}
bool paula::readInt(INT& out, const INT* node)
{
	if (!match(node, NODE_INTEGER)) return false;
	out = *(node + 3);
	return true;
}

bool paula::readDouble(DOUBLE& out, const INT* node)
{
	if (!match(node, NODE_DOUBLE)) return false;
	INT a = *(node + 3);
	INT b = *(node + 4);
	LONG bits = intsToLong(a, b);
	out = longToDoubleFormat(bits);
	return true;
}

bool paula::readBool(bool& out, const INT* node)
{
	if (!match(node, NODE_BOOL)) return false;
	out = (*(node + 3)) != 0;
	return true;
}
bool paula::readOp(char& out, const INT* node)
{
	if (!match(node, NODE_OPERATOR)) return false;
	out = (char)(*(node + 3));
	return true;
}

bool paula::readChars(char*& out, const INT* node)
{
	if (!match(node, NODE_TEXT)) return false;
	out = (char*)(node + 4);
	return true;
}
