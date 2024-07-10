#include "defs.h"
#include "paula.h"
#include "test.h"

void paula::runErrorCheck(void(*test)(), Error expectedError)
{
	try
	{
		test();
	}
	catch (const PaulaException& e)
	{
		LOGLINE("test: caught an exception: " << e.error.name<<" (id="<<e.error.id<<")");
		if (e.error.id != expectedError.id)
		{
			ASSERT(false, "error don't match");
		}
	}
}

void paula::parenthesisTest()
{
	runErrorCheck([]() {
		CharInputStream input("foo(12, (34, 56)");
		Paula::one.run(input, false);
	}, PARENTHESIS);
}

void paula::iteratorTest()
{
	// TODO
}
