#include "paula.h"
#include "engine.h"
#include <string.h>

using namespace paula;
using namespace paula::core;
using namespace std;

constexpr const char * PAULA_VERSION = "0.1";

const char* evDir = "PAULA_DIR";

inline char separator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}
void info()
{
	pout.print("Paula Script command line interface, version ").print(PAULA_VERSION).endl();
}
void error()
{
	err.println("Paula Script: unrecognized command line arguments...");
}
int fileNotFound()
{
	err.println("file not found.");
	return -1;
}
int runScript (IInputStream&input)
{
	auto error = Engine::one.run(input, false);
	if (error != NO_ERROR)
	{
		err.println("ERROR: ").print(error).endl();
		return -1;
	}
	return 0;
}
int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		info(); return 0;
	}
	else if (argc == 2)
	{
		if (strcmp(argv[1], "-i") == 0)
		{
			// read from standard input

			StandardInput input;
			return runScript(input);
		}
	}
	else if (argc == 3)
	{
#ifndef PAULA_MINI
		if (strcmp(argv[1], "-f") == 0)
		{   
			// read and run script from a file

			std::string fn = argv[2];

			if (!FileInput::exists(fn)) {
				const char* value = getenv(evDir);
				if (value) 
				{
					fn = separator() + fn;
					fn = value + fn;
					if (!FileInput::exists(fn)) return fileNotFound();
				} else return fileNotFound();
			}
			pout.print("read file: ").print(fn.c_str()).endl();
			FileInput input(fn.c_str());
			return runScript(input);
		}
#endif
	}
	error();
	return -1;
	//CharInputStream input("t:\"hello!\"");
	//auto error = Paula::one.run(input, false);
	//std::cin.get();  // Waits for Enter key
}
