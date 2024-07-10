// paula-vs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "paula.h"
#include "tree.h"
#include "stream.h"
#include "test.h"

using namespace paula;

void treeTest();
void stackTest();

int main()
{
	LOG("Hello World!\n");
	
	//iteratorTest();
	//parenthesisTest();
	
	//stackTest();

	//treeTest();
	//CharInputStream input("f(5,5)");
	//CharInputStream input("x:5\nx:f(5)\nfoo(12, (34, 56))");
	//CharInputStream input("x:5\nx:f(5)");
	
	CharInputStream input("print(123456, foo(3, 4))");
	Paula::one.run(input, false);
}

void stackTest()
{
	Tree stack(1024);
	stack.init(NODE_STACK);
	stack.pushInt(0, 123);
	LOGLINE("stack size: "<<stack.stackSize(0));
	stack.pushInt(0, 456);
	LOGLINE("stack size: "<<stack.stackSize(0));
	stack.print();
	stack.pop(0);
	stack.print();
	stack.pushInt(0, 789);
	stack.print();
	stack.pop(0);
	stack.print();
	stack.pop(0);
	LOGLINE("stack size: "<<stack.stackSize(0));
	LOGLINE("stack empty? "<<stack.stackEmpty(0));
}

void treeTest()
{
	Tree tree(1024);
	tree.init(NODE_SUBTREE);
	tree.addInt(0, 100);
	tree.addInt(0, 200);
	tree.addInt(0, 300);
	INT parent = tree.addSubtree(0, NODE_SUBTREE);
	tree.addInt(parent, 400);
	tree.addInt(parent, 500);
	tree.print();
}
