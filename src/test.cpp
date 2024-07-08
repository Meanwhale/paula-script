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
		LOGLINE("test: caught an exception: " << e.what()<<" (id="<<e.id<<")");
		if (e.id != expectedError)
		{
			ASSERT(false, "error don't match");
		}
	}
}

void paula::parenthesisTest()
{
	runErrorCheck([]() {
		Paula p;
		CharInputStream input("foo(12, (34, 56)");
		p.run(input, false);
	}, Error::PARENTHESIS);
}
