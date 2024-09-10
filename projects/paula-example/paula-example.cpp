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

int main()
{
	/*auto error = paula::addCallback("doubler", doubler);
    paula::runSafe("six: doubler(3)");
	INT value;
	Var v = paula::get("six");
	if (v.getInt(value))
	{
		std::cout<<"six = "<<value<<std::endl;
	}
	else
	{
		std::cout<<"something went wrong..."<<std::endl;
	}*/

	core::testAll();
}
