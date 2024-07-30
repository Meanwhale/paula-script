#pragma once

#include <iostream>

#define LOG paula::log
#define ERR paula::err

#define HALT std::exit(0) 
#define STR(x) #x

#ifdef VERBOSE
#define VRB(x) x
#else
#define VRB(x)
#endif


// assert: internal error, check: user error


#if PAULA_RELEASE
#define ASSERT_MSG(x,msg) { if (!(x)) { trap(STR(x), __FILE__, __LINE__, msg); }}
#define ASSERT(x)         { if (!(x)) { trap(STR(x), __FILE__, __LINE__, ""); }}
#else
#define ASSERT_MSG(x,msg)
#define ASSERT(x)
#endif

#define IS_CHAR(c) (c>='a' && c<='z')

// error id is its definition line number
#define ERROR_TYPE(x) constexpr Error x (__LINE__,STR(x))

namespace paula
{
	typedef int32_t INT;
	typedef int64_t LONG;
	typedef double DOUBLE;
	typedef char CHAR;
	typedef uint8_t BYTE;

	class POut;
	class NullPrint;

#if PAULA_RELEASE
	extern const NullPrint log; // optimize debug print away
#else
	extern const POut& log;
#endif
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
	ERROR_TYPE (INDENTATION_ERROR);
	ERROR_TYPE (WRONG_NUMBER_OF_ARGUMENTS);
	ERROR_TYPE (CALLBACK_ERROR);
	ERROR_TYPE (TEXT_VARIABLE_OVERWRITE);

	// tree + interator

	ERROR_TYPE (TREE_IS_EMPTY);
	ERROR_TYPE (ITERATOR_HAS_NO_NEXT_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_CHILD_ELEMENT);
	ERROR_TYPE (ITERATOR_HAS_NO_PARENT_ELEMENT);
	ERROR_TYPE (ITERATOR_WRONG_DATA_TYPE);

	constexpr INT
		ARG_STACK_SIZE = 1024,
		VARS_SIZE = 1024,
		CONSTANTS_SIZE = 128,
		MAX_VAR_NAME_LENGTH = 128,
		MAX_VAR_NAME_DATA_LENGTH = 36,
		MAX_RETURN_VALUE_SIZE = 1024,
		MAX_TEXT_SIZE = 1024,

		LINE_UNDEFINED = 10001,
		LINE_ASSIGNMENT = 10002,
		LINE_CALL = 10003;
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