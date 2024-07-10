#pragma once
namespace paula
{
	void runErrorCheck(void (* test)(), Error expectedError);

	void parenthesisTest();
	void iteratorTest();
}