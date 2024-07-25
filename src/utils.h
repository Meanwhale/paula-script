#pragma once
#include "defs.h"
//#include "array.h"
#include <iostream>
namespace paula
{
	template <class BYTE> class Array;
	template <class INT> class Array;

	void trap(const char *, const char *, int, const char *);
    LONG doubleToLongFormat(DOUBLE value);
	DOUBLE longToDoubleFormat(LONG value);
	INT longHighBits(LONG value);
	INT longLowBits(LONG value);
	LONG intsToLong(INT high, INT low);
    //void assert(bool, const char *);
	INT charToInt(CHAR c);
    INT textDataSize(INT numBytes);
    void bytesToInts(const unsigned char *  bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength);
}