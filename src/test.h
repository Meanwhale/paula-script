#pragma once

#include"defs.h"

namespace paula
{
	void runErrorCheck(const Error* (* test)(), const Error* expectedError);

	void doubleTest();
    void operatorTest();
	void variableTest();
	void functionTest();
	void loopTest();
	void ifTest();
	void parenthesisErrorTest();
	void callbackTest();

    void runAll();
	//const Error* iteratorTest();
}