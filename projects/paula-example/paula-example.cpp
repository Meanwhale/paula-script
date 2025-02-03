// paula-example.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "paula.h"
#include "test.h"

using namespace paula;

const paula::Error* doubler (paula::Args&args)
{
	if (args.count() != 1) return &WRONG_NUMBER_OF_ARGUMENTS;
	INT value = -1;
	if(args.get(0).getInt(value))
	{
		args.returnInt(2*value);
		return NO_ERROR;
	}
	return &CALLBACK_ERROR;
}

const paula::Error* myCallback (paula::Args&args)
{
	for (int i=0; i<args.count(); i++)
	{
		char * value;
		if (args.get(i).getChars(value))
		{
			std::cout<<"arg "<<i<<": "<<value<<std::endl;
		}
	}
	return NO_ERROR;
}

int main()
{
	// register a callback for Paula engine
	//auto error = paula::addCallback("test", myCallback);
	// call it from a script
	//error = paula::run("test(\"foo\", \"bar\")");

	//auto error = paula::addCallback("doubler", doubler);
 //   paula::runSafe("six: doubler(3)");
	//INT value;
	//Var v = paula::get("six");
	//if (v.getInt(value))
	//{
	//	std::cout<<"six = "<<value<<std::endl;
	//}
	//else
	//{
	//	std::cout<<"something went wrong..."<<std::endl;
	//}

	//core::fiboTest();
	//core::testAll();
	core::scriptFunctionTest();
	//recursiveProcedureTest();
}
