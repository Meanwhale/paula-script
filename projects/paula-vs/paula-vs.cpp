// paula-vs.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "engine.h"
#include "tree.h"
#include "stream.h"
#include "test.h"

using namespace paula;
using namespace paula::core;

int main()
{
	LOG.println("Paula unit tests\n");
	
	runAll();
}

