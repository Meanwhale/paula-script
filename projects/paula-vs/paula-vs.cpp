// paula-vs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "paula.h"
#include "tree.h"
#include "stream.h"
#include "test.h"

using namespace paula;

void treeTest();

int main()
{
	LOG("Hello World!\n");
	
	parenthesisTest();

	//treeTest();
	Paula p;
	//CharInputStream input("f(5,5)");
	//CharInputStream input("x:5\nx:f(5)\nfoo(12, (34, 56))");
	CharInputStream input("foo(12 34, (34, 56)");
	//CharInputStream input("x:5\nx:f(5)");
	p.run(input, true);
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
