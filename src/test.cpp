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

#define TEST_INT(name,value) a = -123456; ASSERT(Paula::one.vars.getInt(a, name)); LOG.print("a:").print(a).endl(); ASSERT(a == value);
#define TEST_BOOL(name,value) b = false; ASSERT(Paula::one.vars.getBool(b, name)); LOG.print("b:").print(b).endl(); ASSERT(b == value);
#define TEST_TEXT(name,value) t = nullptr; ASSERT(Paula::one.vars.getChars(t, name)); LOG.print("t:").print(t).endl(); ASSERT(strcmp(t, value) == 0);

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
	CharInputStream input("a:1\nb:true\nwhile(b)\n\tb:false\n\twhile(a<5)\n\t\ta:a+1");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	INT a;
	TEST_INT("a", 5);
}
void paula::ifTest()
{
	CharInputStream input("a:1\nb:1\nwhile(a<5)\n\ta:a+1\n\tif(a>4)\n\t\tb:b+10\n\t\tif(a>4)\n\t\t\tb:b+10");
	auto err = Paula::one.run(input, false);
	ASSERT(err == NO_ERROR);
	INT a;
	TEST_INT("a", 5);
	TEST_INT("b", 21);
}

void paula::parenthesisErrorTest()
{
	runErrorCheck([]() {
		CharInputStream input("foo (12, (34, 56)");
		return Paula::one.run(input, false);
	}, &PARENTHESIS);
}

const paula::Error* paula::testCallback (Paula&p,Args&args)
{
	LOG.println("-------- TEST ACTION --------");
	CHECK(args.argCount() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	Data data;
	args.get(0, data);
	INT value = false;
	if(data.getInt(value))
	{
		args.returnInt(2*value);
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}

void paula::textTest()
{
	CharInputStream input("t:\"hello!\"");
	auto error = Paula::one.run(input, false);
	ASSERT(error == NO_ERROR);
	char * t;
	TEST_TEXT("t", "hello!");

	runErrorCheck([]() {
		CharInputStream input2("t:\"hello!\"\nt:\"a\"");
		return Paula::one.run(input2, false);
	}, &TEXT_VARIABLE_OVERWRITE);
}

void paula::callbackTest()
{
	auto error = Paula::one.addCallback("testCallback", testCallback);
	ASSERT(error == NO_ERROR);
	CharInputStream input("a:3\na:testCallback(3)");
	auto err = Paula::one.run(input, false);
	ASSERT(error == NO_ERROR);
	INT a;
	TEST_INT("a", 6);
}

void paula::runAll()
{
	variableTest();
	textTest();
	doubleTest();

	functionTest();
	parenthesisErrorTest();
	loopTest();
	ifTest();
	operatorTest();

	callbackTest();
}

//
//const Error* paula::iteratorTest()
//{
//	// TODO
//}
