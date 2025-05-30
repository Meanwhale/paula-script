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
		void logicalTest();
		void variableTest();
		void commandTest();
		void callbackTest();
		void parenthesisErrorTest();
		void callbackTest();
		void treeTest();
		void stackTest();
		void reservedNameTest();
        void argTest();
        void procTest();
		void fiboTest();
		void procedureBackTest();
		void scriptFunctionTest();
        void functionArgTest();
		void recursiveFunctionTest();
        void semicolonTest();

		void loopTest();
		void ifTest();

        void safeTest();

        void testAll();
		//const Error* iteratorTest();
	}
}
#endif

