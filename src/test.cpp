#include "defs.h"
#include "paula.h"
#include "test.h"

void paula::runErrorCheck(const Error* (*test)(), const Error* expectedError)
{
	auto error = test();
	
	if (error != NO_ERROR)
	{
		ERR
			.print("test: caught an exception: ")
			.print(error->name)
			.print(" (id=")
			.print(error->id)
			.print(")")
			.endl();
	}
	if (!Error::equal(expectedError, error))
	{
		ASSERT(false, "error don't match");
	}
}

void paula::parenthesisTest()
{
	runErrorCheck([]() {
		CharInputStream input("foo(12, (34, 56)");
		return Paula::one.run(input, false);
	}, &PARENTHESIS);
}
//
//const Error* paula::iteratorTest()
//{
//	// TODO
//}
