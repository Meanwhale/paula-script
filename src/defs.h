#pragma once

#include <iostream>


#define LOG(x) std::cout<<x;
#define LOGCHAR(x) logChar(x, std::cout);
#define LOGLINE(x) std::cout<<x<<std::endl;

#define LOGERRORCHAR(x) logChar(x, std::cerr);
#define LOGERROR(x) std::cerr<<"\nERROR: "<<x<<std::endl;

#define HALT std::exit(0) 
#define STR(x) #x

// assert: internal error, check: user error

//#define ASSERT(x,msg) { if (!(x)) { printf("FAIL: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); assert(false, msg); }}
#define ASSERT(x,msg) { if (!(x)) { std::cout<<"FAIL: ("<<STR(x)<<"), file "<<__FILE__<<", line "<<__LINE__<<std::endl; assert(false, msg, INTERNAL); }}
#define EXIT(msg) { LOGERROR(msg); }

#define IS_CHAR(c) (c>='a' && c<='z')

// error id is its definition line number
#define ERROR_TYPE(x) constexpr Error x (__LINE__,STR(x))

namespace paula
{
	typedef int32_t INT;
	typedef char CHAR;
	typedef uint8_t BYTE;

	class Error
	{
	public:
		const INT id; // unique id only for internal use.
		const char * name;
		constexpr explicit Error(int _id, const char * _name) : id(_id), name(_name) { }
	};

	ERROR_TYPE (UNDEFINED);
	ERROR_TYPE (INTERNAL);
	ERROR_TYPE (ARRAY_OUT_OF_RANGE);
	ERROR_TYPE (PARENTHESIS);
	ERROR_TYPE (UNEXPECTED_CHARACTER);
	ERROR_TYPE (PARSE_ERROR);
	ERROR_TYPE (SYNTAX_ERROR);
	ERROR_TYPE (UNKNOWN_COMMAND);
	ERROR_TYPE (UNKNOWN_EXPRESSION);
	ERROR_TYPE (FUNCTION_DID_NOT_RETURN_A_VALUE);

	// tree + interator

	ERROR_TYPE (TREE_IS_EMPTY);
	ERROR_TYPE (ITERATOR_HAS_NO_NEXT_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_CHILD_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_PARENT_ELEMENT);
	ERROR_TYPE (ITERATOR_WRONG_DATA_TYPE);
}

#ifdef PAULA_EXCEPTIONS
#include <exception>
namespace paula
{
	class PaulaException : public std::exception {
	public:
		const Error error;
		PaulaException(Error _error) : std::exception(), error(_error) {}
		PaulaException(const char* msg, Error _id) : std::exception(msg), error(_id) {}
	};
}
#define CHECK(x,e) { if (!(x)) { throw PaulaException(e); }}
#else
#define CHECK(x,msg) { if (!(x)) { LOGERROR(msg); ASSERT(false, "CHECK failed"); }}
#endif