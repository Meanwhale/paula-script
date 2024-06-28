// paula-vs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "paula.h"
#include "tree.h"
#include "stream.h"

using namespace paula;

void treeTest();

int main()
{
	LOG("Hello World!\n");
	treeTest();
	Paula p;
	CharInputStream input("test script\nsecond line");
	p.run(input);
}

void treeTest()
{
	Tree tree(1024);
	tree.addInt(0, 100);
	tree.addInt(0, 200);
	tree.addInt(0, 300);
	INT parent = tree.addStruct(0);
	tree.addInt(parent, 400);
	tree.addInt(parent, 500);
	tree.print();
}
