#pragma once

#include"defs.h"

namespace paula
{
	void runErrorCheck(const Error* (* test)(), const Error* expectedError);

	void doubleTest();


	void variableTest();

	void functionTest();

	void loopTest();

	void parenthesisTest();
    void runAll();
	//const Error* iteratorTest();
}