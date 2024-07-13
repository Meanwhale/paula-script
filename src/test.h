#pragma once

#include"defs.h"

namespace paula
{
	void runErrorCheck(const Error* (* test)(), const Error* expectedError);

	void parenthesisTest();
	//const Error* iteratorTest();
}