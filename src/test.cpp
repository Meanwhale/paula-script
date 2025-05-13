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
	else
	{
		LOG.println("Succesful error test!");
	}
}

#define ERROR_TEST(code,error) runErrorCheck([]() { return paula::run(code); }, &error);
#define ASSERT_NO_ERROR(error) {if (error != NO_ERROR) {ERR.print("ERROR: ").print(error).endl(); ASSERT(false);}}

void core::doubleTest()
{
	// conversion test

	LONG tmp = doubleToLongFormat(123.456);
	DOUBLE d = longToDoubleFormat(tmp);
	ASSERT(123.456 == d);

	// script test

	auto err = paula::run("a:123.456 ");
	ASSERT_NO_ERROR(err);
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
	auto err = paula::run("a:5+5\nb:(a*2)\nc:b/5\nd:c-1\ne:1+(2-3)\nvale:a>1000\ntosi:a>0\nsama:1=1\neisama:4=5");
	ASSERT_NO_ERROR(err);
	INT a;
	bool b;
	TEST_INT("a", 10);
	TEST_INT("b", 20);
	TEST_INT("c", 4);
	TEST_INT("d", 3);
	TEST_INT("e", 0);
	TEST_BOOL("vale", false);
	TEST_BOOL("tosi", true);
	TEST_BOOL("sama", true);
	TEST_BOOL("eisama", false);
}

void core::variableTest()
{
	auto err = paula::run("a:5");
	ASSERT_NO_ERROR(err);
	INT a;
	TEST_INT("a", 5);
}
void core::commandTest()
{
	auto err = paula::run("b:true\ntmp:not(b)");
	ASSERT_NO_ERROR(err);
	bool b;
	TEST_BOOL("b", true);
	TEST_BOOL("tmp", false);
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
	ASSERT_NO_ERROR(error);
	char * t;
	TEST_TEXT("t", "hello!");

	error = paula::run("t:\"\\x7E\"");
	ASSERT_NO_ERROR(error);
	TEST_TEXT("t", "~");

	error = paula::run("t:\"\\n\"");
	ASSERT_NO_ERROR(error);
	TEST_TEXT("t", "\n");

	ERROR_TEST("t:\"hello!\"\nt:\"a\"", TEXT_VARIABLE_OVERWRITE);
	ERROR_TEST("t:\"abc", QUOTE_ERROR);
	ERROR_TEST("t:\"\\!", QUOTE_ERROR);
}

void core::callbackTest()
{
	auto error = paula::addCallback("testCallback", testCallback);
	ASSERT_NO_ERROR(error);
	auto err = paula::run("a:3\na:testCallback(3)");
	ASSERT_NO_ERROR(error);
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
	stack.pushText("hei");
	stack.pushText("moi");
	stack.pushInt(789);
	stack.pop();

	StackIterator src(stack);
	LOG.print("stackTest: ").print(src.var()).endl();

	stack.pop();
	StackIterator src2(stack);
	LOG.print("stackTest: ").print(src2.var()).endl();

	char *t1, *t2;
	if (src.var().getChars(t1) && src2.var().getChars(t2))
	{
		ASSERT(strcmp(t1, "moi") == 0);
		ASSERT(strcmp(t2, "hei") == 0);
	}
	else ASSERT(false);

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
void core::argTest()
{
	const char* args[] = { "Hello", "World" };
	auto error = paula::run("hello:arg(0)\nworld:arg(1)", args, 2);
	ASSERT_NO_ERROR(error);
}
void core::semicolonTest()
{
	auto error = paula::run("i:5;i:i+1");
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("i", 6);

	ERROR_TEST("b:true;if(b)\n\tb:false", CONDITION_LINE_WITH_SEMICOLON);
	ERROR_TEST("b:true;while(b)\n\tb:false", CONDITION_LINE_WITH_SEMICOLON);
}
void core::loopTest()
{
	auto err = paula::run("b:true\nwhile(b)\n\tb:false");
	ASSERT_NO_ERROR(err);

	err = paula::run("a:1\nb:true\nwhile(b)\n\tb:false\n\twhile(a<5)\n\t\ta:a+1");
	ASSERT_NO_ERROR(err);
	INT a;
	TEST_INT("a", 5);
}
void core::ifTest()
{
	auto err = paula::run("a:1\nb:1\nwhile(a<5)\n\ta:a+1\n\tif(a>4)\n\t\tb:b+10\n\t\tif(a>4)\n\t\t\tb:b+10");
	ASSERT_NO_ERROR(err);
	INT a;
	TEST_INT("a", 5);
	TEST_INT("b", 21);
}
void core::procTest()
{
	auto error = paula::run("proc (\"adder\")\n\ta:arg(0)\n\ta:a+2\na:1\nadder(3)");
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("a", 5);
}
void core::fiboTest()
{
	const char * code = 

		"proc (\"fibo\")\n"
		"\ta: arg(0)\n"
		"\tb: (arg(0)) + (arg(1))\n"
		"N: 10\n"
		"a: 0\n"
		"b: 1\n"
		"while(N > 0)\n"
		"\tfibo(a, b)\n"
		"\tN: N-1";

	//const char * code = "proc (\"fibo\")\n\tf: (arg(0)) + (arg(1))\nN: 5\nf: 0\nwhile(N > 0)\n\tfibo(f, f+1)\n\tN: N-1";
	LOG.println(code);
	auto error = paula::run(code);
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("b", 89); // N=10, number 89 is 12th, first two are initialized
}
void core::procedureBackTest()
{
	auto error = paula::run("proc (\"five\")\n\ta:a+4\n\tback()\na:1\nfive()");
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("a", 5);
}
void core::scriptFunctionTest()
{
	auto error = paula::run("func (\"five\")\n\treturn(5)\na:five()");
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("a", 5);
}
void core::functionArgTest()
{
	const char * code =
		"func (\"dbl\")\n"
		"\tx: arg(0)\n"
		"\tx: x * 2\n"
		"\treturn(x)\n"
		"a:dbl(7)";

	auto error = paula::run(code);
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("a", 14);
}
void core::recursiveFunctionTest()
{
	const char * code =
		"func (\"rec\")\n"
		"\tx: arg(0)\n"
		"\tif (x > 4)\n"
		"\t\treturn(x)\n"
		"\treturn(x + 1)\n"
		"a:rec(1)";

	auto error = paula::run(code);
	ASSERT_NO_ERROR(error);
	INT a;
	TEST_INT("a", 5);
}

void core::safeTest()
{
	paula::runSafe("a:!"); // engine handles error
	LOG.println("Succesfully catched error in 'safeTest'!");
}
void core::testAll()
{
	stackTest();
	treeTest();

	variableTest();
	commandTest();
	textTest();
	doubleTest();

	callbackTest();
	parenthesisErrorTest();
	operatorTest();
	reservedNameTest();
	argTest();
	semicolonTest();

	loopTest();
	ifTest();
	procTest();
	fiboTest();
	procedureBackTest();
	functionArgTest();

	safeTest();

	LOG.println("\n\n---------------- ALL TESTS DONE ----------------\n");
}
#endif