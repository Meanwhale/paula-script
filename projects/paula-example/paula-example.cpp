// paula-example.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "paula.h"

using namespace paula;

const Error* doubler (Args&args)
{
	CHECK(args.count() == 1, WRONG_NUMBER_OF_ARGUMENTS);
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
	auto error = paula::addCallback("doubler", doubler);
    paula::runAndCatch("six: doubler(3)");
	INT value;
	if (paula::get("six").getInt(value))
	{
		std::cout<<"six = "<<value<<std::endl;
	}
	else
	{
		std::cout<<"something went wrong..."<<std::endl;
	}
}
