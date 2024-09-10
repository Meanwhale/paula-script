#ifndef PAULA_RELEASE
#include "defs.h"
#include "paula.h"
#include "engine.h"
#include "test.h"
#include <iostream>
#include <cstring>

using namespace paula;
using namespace paula::core;

void core::runErrorCheck(const Error* (*test)(), const Error* expectedError)
{
	auto error = test();
	
	if (!Error::equal(expectedError, error))
	{
		ERR.print("catch: ").print(error).print(" expected: ").print(expectedError).endl();
		ASSERT_MSG(false, "error don't match");
	}
}

#define ERROR_TEST(code,error) runErrorCheck([]() { return paula::run(code); }, &error);

void core::doubleTest()
{
	// conversion test

	LONG tmp = doubleToLongFormat(123.456);
	DOUBLE d = longToDoubleFormat(tmp);
	ASSERT(123.456 == d);

	// script test

	auto err = paula::run("a:123.456 ");
	ASSERT(err == NO_ERROR);
	DOUBLE a;
	//ASSERT(Paula::one.vars.getDouble(a, "a"));
	ASSERT(paula::get("a").getDouble(a));
	ASSERT(a == 123.456);
}

#define TEST_INT(name,value) a = -123456; ASSERT(paula::get(name).getInt(a)); LOG.print("a:").print(a).endl(); ASSERT(a == value);
#define TEST_BOOL(name,value) b = false; ASSERT(paula::get(name).getBool(b)); LOG.print("b:").print(b).endl(); ASSERT(b == value);
#define TEST_TEXT(name,value) t = nullptr; ASSERT(paula::get(name).getChars(t)); LOG.print("t:").print(t).endl(); ASSERT(strcmp(t, value) == 0);

void core::operatorTest()
{
	auto err = paula::run("a:5+5\nb:(a*2)\nc:b/5\nd:c-1\nvale:a>1000\ntosi:a>0\nsama:1=1\neisama:4=5");
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

void core::variableTest()
{
	auto err = paula::run("a:5");
	ASSERT(err == NO_ERROR);
	INT a;
	TEST_INT("a", 5);
}
void core::functionTest()
{
	auto err = paula::run("b:true\ntmp:not(b)");
	ASSERT(err == NO_ERROR);
	bool b;
	TEST_BOOL("b", true);
	TEST_BOOL("tmp", false);
}

void core::loopTest()
{
	auto err = paula::run("b:true\nwhile(b)\n\tb:false");
	ASSERT(err == NO_ERROR);

	err = paula::run("a:1\nb:true\nwhile(b)\n\tb:false\n\twhile(a<5)\n\t\ta:a+1");
	ASSERT(err == NO_ERROR);
	INT a;
	TEST_INT("a", 5);
}
void core::ifTest()
{
	auto err = paula::run("a:1\nb:1\nwhile(a<5)\n\ta:a+1\n\tif(a>4)\n\t\tb:b+10\n\t\tif(a>4)\n\t\t\tb:b+10");
	ASSERT(err == NO_ERROR);
	INT a;
	TEST_INT("a", 5);
	TEST_INT("b", 21);
}

void core::parenthesisErrorTest()
{
	ERROR_TEST("foo (12, (34, 56)", PARENTHESIS);
}

const Error* core::testCallback (Args&args)
{
	LOG.println("-------- TEST ACTION --------");
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
	INT value = -1;
	if(args.get(0).getInt(value))
	{
		args.returnInt(2*value);
		return NO_ERROR;
	}
	return &TYPE_MISMATCH;
}

void core::textTest()
{
	auto error = paula::run("t:\"hello!\"");
	ASSERT(error == NO_ERROR);
	char * t;
	TEST_TEXT("t", "hello!");

	error = paula::run("t:\"\\x7E\"");
	ASSERT(error == NO_ERROR);
	TEST_TEXT("t", "~");

	error = paula::run("t:\"\\n\"");
	ASSERT(error == NO_ERROR);
	TEST_TEXT("t", "\n");

	ERROR_TEST("t:\"hello!\"\nt:\"a\"", TEXT_VARIABLE_OVERWRITE);
	ERROR_TEST("t:\"abc", QUOTE_ERROR);
	ERROR_TEST("t:\"\\!", QUOTE_ERROR);
}

void core::callbackTest()
{
	auto error = paula::addCallback("testCallback", testCallback);
	ASSERT(error == NO_ERROR);
	auto err = paula::run("a:3\na:testCallback(3)");
	ASSERT(error == NO_ERROR);
	INT a;
	TEST_INT("a", 6);
}

void core::treeTest()
{
	Tree tree(1024);
	tree.init(NODE_SUBTREE);
	tree.addInt(0, 100);
	tree.addInt(0, 200);
	tree.addInt(0, 300);
	INT parent = tree.addSubtree(0, NODE_SUBTREE);
	tree.addInt(parent, 400);
	tree.addInt(parent, 500);

	TreeIterator it(tree,parent);
	it.toChild();
	INT a;
	ASSERT(it.var().getInt(a));
	ASSERT(a == 400);
}
void core::stackTest()
{
	Stack stack(1024);
	stack.pushInt(123);
	stack.pushInt(456);
	stack.pop();
	stack.pushInt(789);
	stack.pop();
	stack.pop();
	ASSERT(stack.itemCount() == 0);
}
void core::reservedNameTest()
{
	ERROR_TEST("if:1", RESERVED_NAME);
	ERROR_TEST("true:1", RESERVED_NAME);

	auto error = paula::addCallback("while", testCallback);
	ASSERT(Error::equal(error, &RESERVED_NAME));

	error = paula::addCallback("true", testCallback);
	ASSERT(Error::equal(error, &RESERVED_NAME));
}
void core::semicolonTest()
{
	auto error = paula::run("i:5;i:i+1");
	ASSERT(error == NO_ERROR);
	INT a;
	TEST_INT("i", 6);

	ERROR_TEST("b:true;if(b)\n\tb:false", CONDITION_LINE_WITH_SEMICOLON);
	ERROR_TEST("b:true;while(b)\n\tb:false", CONDITION_LINE_WITH_SEMICOLON);
}
void core::testAll()
{
	stackTest();
	treeTest();

	variableTest();
	textTest();
	doubleTest();

	functionTest();
	parenthesisErrorTest();
	operatorTest();
	reservedNameTest();
	semicolonTest();
	callbackTest();

	loopTest();
	ifTest();

}
#endif