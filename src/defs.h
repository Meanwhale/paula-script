#pragma once

#include <iostream>


#define LOG(x) std::cout<<x;
#define LOGCHAR(x) logChar(x);
#define LOGLINE(x) std::cout<<x<<std::endl;
#define LOGERROR(x) std::cout<<"\nERROR: "<<x;

#define HALT std::exit(0) 
#define STR(x) #x

// assert: internal error, check: user error

//#define ASSERT(x,msg) { if (!(x)) { printf("FAIL: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); assert(false, msg); }}
#define ASSERT(x,msg) { if (!(x)) { std::cout<<"FAIL: ("<<STR(x)<<"), file "<<__FILE__<<", line "<<__LINE__<<std::endl; assert(false, msg); }}
#define EXIT(msg) { LOGERROR(msg); }

#define IS_CHAR(c) (c>='a' && c<='z')

namespace paula
{
	typedef int32_t INT;
	typedef char CHAR;
	typedef uint8_t BYTE;

	enum Error
	{
		UNDEFINED,

		PARENTHESIS,
		UNEXPECTED_CHARACTER
	};
}

#ifdef PAULA_EXCEPTIONS
#include <exception>
namespace paula
{
	class PaulaException : public std::exception {
	public:
		PaulaException(Error _id) : std::exception(), id(_id) {}
		PaulaException(const char* msg, Error _id) : std::exception(msg), id(_id) {}
		const Error id;
	};
}
#define CHECK(x,e) { if (!(x)) { throw PaulaException(e); }}
#else
#define CHECK(x,msg) { if (!(x)) { LOGERROR(msg); ASSERT(false, "CHECK failed"); }}
#endif