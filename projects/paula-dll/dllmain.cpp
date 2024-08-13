#include "paula.h"

// Paula DLL

extern "C" {
	//__declspec(dllexport) int Add(int a, int b) {
	//	return a + b;
	//}
	__declspec(dllexport) void printVersion() {
		paula::printVersion();
	}
}