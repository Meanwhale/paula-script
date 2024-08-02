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

	// text functions

	INT charToInt(CHAR c);
    INT textDataSize(INT numBytes);
    void bytesToInts(const unsigned char *  bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength);
    bool matchTextData(INT* a, INT* b);
	void charsToNameData(const char* str, Array<INT>& trg);

	const char* treeTypeName(INT tag);

	// data access
	bool match(const INT* ptr, INT tag);
	bool readInt(INT& out, const INT* node);
	bool readDouble(DOUBLE& out, const INT* node);
	bool readBool(bool& out, const INT* node);
	bool readOp(char& out, const INT* node);
	bool readChars(char*&out, const INT* node);
}