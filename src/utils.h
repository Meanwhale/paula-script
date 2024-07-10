#pragma once
#include "defs.h"
//#include "array.h"
#include <iostream>
namespace paula
{
	template <class BYTE> class Array;
	template <class INT> class Array;

	void assert(bool, const char *, const Error&);
	void logChar(CHAR, std::ostream&);
    INT textDataSize(INT numBytes);
    void bytesToInts(const unsigned char *  bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength);
}