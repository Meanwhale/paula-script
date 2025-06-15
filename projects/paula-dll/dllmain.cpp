#include "paula.h"

// Paula DLL

extern "C" {
	__declspec(dllexport) void printVersion() {
		paula::printVersion();
	}
	__declspec(dllexport) void runSafe(const char* code) {
		paula::runSafe(code);
	}
	__declspec(dllexport) const int* getPtr(const char* varName) {
		return paula::getPtr(varName);
	}
}