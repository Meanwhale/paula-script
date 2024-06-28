#pragma once

#include <iostream>

#define LOG(x) std::cout<<x;
#define LOGLINE(x) std::cout<<x<<std::endl;
#define LOGERROR(x) std::cout<<"\nERROR: "<<x;

#define HALT std::exit(0) 
#define STR(x) #x

// assert: internal error, check: user error

#define ASSERT(x,msg) { if (!(x)) { printf("FAIL: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); assert(false, msg); }}
#define CHECK(x,msg) { if (!(x)) { LOGERROR(msg); }}
#define EXIT(msg) { LOGERROR(msg); }

#define IS_CHAR(c) (c>='a' && c<='z')

namespace paula
{
	typedef int32_t INT;
	typedef char CHAR;
	typedef uint8_t BYTE;

	void assert(bool, const char *);
}