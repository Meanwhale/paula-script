#ifndef PAULA_RELEASE
#pragma once

#include"defs.h"

namespace paula
{
	class Args;

	namespace core
	{
		class Engine;

		void runErrorCheck(const Error* (* test)(), const Error* expectedError);
	
		const Error* testCallback (Args&args);

		void textTest();

		void doubleTest();
		void operatorTest();
		void variableTest();
		void functionTest();
		void loopTest();
		void ifTest();
		void parenthesisErrorTest();
		void callbackTest();
		void treeTest();
		void stackTest();
		void reservedNameTest();
        void semicolonTest();

		void runAll();
		//const Error* iteratorTest();
	}
}
#endif