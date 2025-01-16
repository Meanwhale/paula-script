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
		void parenthesisErrorTest();
		void callbackTest();
		void treeTest();
		void stackTest();
		void reservedNameTest();
        void argTest();
        void procTest();
		void fiboTest();
        void semicolonTest();

		void loopTest();
		void ifTest();

        void safeTest();

        void testAll();
		//const Error* iteratorTest();
	}
}
#endif