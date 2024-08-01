#include "paula.h"
#include <string.h>

using namespace paula;
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
int run (const string & filename)
{
	pout.print("read file: ").print(filename.c_str()).endl();
	FileInput input(filename.c_str());
	auto error = Paula::one.run(input, false);
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
	else if (argc == 3)
	{
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
			return run(fn);
		}
	}
	error();
	return -1;
	//CharInputStream input("t:\"hello!\"");
	//auto error = Paula::one.run(input, false);
	//std::cin.get();  // Waits for Enter key
}
