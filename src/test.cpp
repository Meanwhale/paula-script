#include "defs.h"
#include "paula.h"
#include "test.h"
#include <iostream>

void paula::runErrorCheck(const Error* (*test)(), const Error* expectedError)
{
	auto error = test();
	
	if (error != NO_ERROR)
	{
		ERR
			.print("test: caught an exception: ")
			.print(error->name)
			.print(" (id=")
			.print(error->id)
			.print(")")
			.endl();
	}
	if (!Error::equal(expectedError, error))
	{
		ASSERT_MSG(false, "error don't match");
	}
}

void paula::doubleTest()
{
	// conversion test

	LONG tmp = doubleToLongFormat(123.456);
	DOUBLE d = longToDoubleFormat(tmp);
	ASSERT(123.456 == d);

	// script test

	CharInputStream input("a:123.456");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	DOUBLE a;
	ASSERT(Paula::one.vars.getDouble(a, "a"));
	ASSERT(a == 123.456);
}

#define TEST_INT(name,value) a = -123456; ASSERT(Paula::one.vars.getInt(a, name)); ASSERT(a == value);
#define TEST_BOOL(name,value) b = false; ASSERT(Paula::one.vars.getBool(b, name)); ASSERT(b == value);

void paula::operatorTest()
{
	CharInputStream input("a:5+5\nb:(a*2)\nc:b/5\nd:c-1\nvale:a>1000\ntosi:a>0\nsama:1=1\neisama:4=5");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	INT a;
	bool b;
	TEST_INT("a", 10);
	TEST_INT("b", 20);
	TEST_INT("c", 4);
	TEST_INT("d", 3);
	TEST_BOOL("vale", false);
	TEST_BOOL("tosi", true);
	TEST_BOOL("sama", true);
	TEST_BOOL("eisama", false);
}

void paula::variableTest()
{
	CharInputStream input("a:5");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	INT a;
	ASSERT(Paula::one.vars.getInt(a, "a"));
	ASSERT(a == 5);
}
void paula::functionTest()
{
	CharInputStream input("b:true\ntmp:not(b)");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	bool value;
	ASSERT(Paula::one.vars.getBool(value, "b"))
	ASSERT(value);
	ASSERT(Paula::one.vars.getBool(value, "tmp"))
	ASSERT(!value);
}

void paula::loopTest()
{
	CharInputStream input("b:true\nwhile (b)\n\tb:not(b)\nprint (b)\nprint (b)\nprint (b)");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
}

void paula::parenthesisTest()
{
	runErrorCheck([]() {
		CharInputStream input("foo (12, (34, 56)");
		return Paula::one.run(input, false);
	}, &PARENTHESIS);
}

void paula::runAll()
{
	parenthesisTest();
	variableTest();
	functionTest();
	loopTest();
}

//
//const Error* paula::iteratorTest()
//{
//	// TODO
//}
