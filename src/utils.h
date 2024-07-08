#pragma once
#include "defs.h"
//#include "array.h"
#include <iostream>
namespace paula
{
	template <class BYTE> class Array;
	template <class INT> class Array;

	void assert(bool, const char *, Error = Error::UNDEFINED);
	void logChar(CHAR);
	void bytesToInts(const unsigned char *  bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength);
}