#pragma once

#include <iostream>

#define LOG paula::log
#define ERR paula::err

#define HALT std::exit(0) 
#define STR(x) #x

// assert: internal error, check: user error

#define ASSERT(x,msg) { if (!(x)) { paula::err.print("FAIL: (").print(STR(x)).print("), file ").print(__FILE__).print(", line ").print(__LINE__).endl(); assert(false, msg); }}
//#define EXIT(msg) { ERR.println(msg); }

#define IS_CHAR(c) (c>='a' && c<='z')

// error id is its definition line number
#define ERROR_TYPE(x) constexpr Error x (__LINE__,STR(x))

namespace paula
{
	typedef int32_t INT;
	typedef char CHAR;
	typedef uint8_t BYTE;

	class POut;

	extern const POut& log; // log output
	extern const POut& err; // error output
	extern const POut& user; // print output
	
	// language keywords

	extern INT* keywords[];
	void addKeyword(INT*);

	class Error
	{
	private:
		Error() = delete;
		Error& operator=(const Error&) = delete;
	public:
		const INT id; // unique id only for internal use.
		const char * name;
		constexpr explicit Error(int _id, const char * _name) : id(_id), name(_name) { }
		bool operator==(const Error& other) const { return this->id == other.id; }
		bool operator!=(const Error& other) const { return this->id != other.id; }
		static bool equal(const Error* a, const Error* b);
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
	ERROR_TYPE (EMPTY_ARGUMENT_VALUE);
	ERROR_TYPE (TYPE_MISMATCH);
	ERROR_TYPE (INVALID_OPERATOR);
	ERROR_TYPE (DIV_ZERO);
	ERROR_TYPE (VARIABLE_NOT_FOUND);

	// tree + interator

	ERROR_TYPE (TREE_IS_EMPTY);
	ERROR_TYPE (ITERATOR_HAS_NO_NEXT_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_CHILD_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_PARENT_ELEMENT);
	ERROR_TYPE (ITERATOR_WRONG_DATA_TYPE);
}

// return value for error checked functions

#define ERROR_STATUS [[nodiscard]] const Error*

// return error if needed

#define CHECK(x,e) { if (!(x)) { return &e; } }

#define NO_ERROR nullptr

// call ERROR_STATUS function and interrupt if there was an error

#define CHECK_CALL(f) { auto r = f; if (r != NO_ERROR) return r; }


/*
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
#define CHECK(x,msg) { if (!(x)) { ERR.println(msg); ASSERT(false, "CHECK failed"); }}
#endif
*/