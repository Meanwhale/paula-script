#include "paula.h"
#include "engine.h"
#include "stream.h"
#include <string.h>

using namespace paula;
using namespace paula::core;
using namespace std;

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
#ifdef PAULA_MINI
	pout.print("Paula Script MINI CLI, v. ").print(PAULA_VERSION).endl();
#else
	pout.print("Paula Script command line interface, version ").print(PAULA_VERSION).endl();
#endif
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
int runCLIScript (IInputStream&input)
{
	auto error = Engine::one.runScript(input);
	if (error != NO_ERROR)
	{
#ifdef PAULA_MINI
		pout.print("ERROR #").print(error).endl(); // error print disabled for MINI
#else
		err.println("ERROR: ").print(error).endl();
#endif
		return -1;
	}
	pout.endl();
	return 0;
}
int runCLIBytecode (IInputStream&input)
{
	auto error = Engine::one.runBytecode(input, nullptr, 0); // TODO CLI args
	if (error != NO_ERROR)
	{
#ifdef PAULA_MINI
		pout.print("ERROR #").print(error).endl(); // error print disabled for MINI
#else
		err.println("ERROR: ").print(error).endl();
#endif
		return -1;
	}
	pout.endl();
	return 0;
}
int compileCLIScript (IInputStream&input, BinaryOutputStream&output)
{
	auto error = Engine::one.compile(input, output);
	if (error != NO_ERROR)
	{
#ifdef PAULA_MINI
		pout.print("ERROR #").print(error).endl(); // error print disabled for MINI
#else
		err.println("ERROR: ").print(error).endl();
#endif
		return -1;
	}
	pout.endl();
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
			return runCLIScript(input);
		}
	}
	else if (argc == 3)
	{
#ifndef PAULA_MINI
		if (strcmp(argv[1], "-f") == 0)
		{   
			// -f read and run script from a file

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
			if(strcmp(argv[1], "-c") != 0)
			{
				// don't mess binary output
				pout.print("read file: ").print(fn.c_str()).endl();
			}
			FileInput input(fn.c_str(), false);
			return runCLIScript(input);

			StandardBinaryOutput output;
			return compileCLIScript(input, output);
		}
		if (strcmp(argv[1], "-b") == 0)
		{
			// read bytecode from input file

			std::string inputFileName = argv[2];

			if (!FileInput::exists(inputFileName)) {
				const char* value = getenv(evDir);
				if (value) 
				{
					inputFileName = separator() + inputFileName;
					inputFileName = value + inputFileName;
					if (!FileInput::exists(inputFileName)) return fileNotFound();
				} else return fileNotFound();
			}
			FileInput input(inputFileName.c_str(), true);
			return runCLIBytecode(input);
		}
	}
	else if (argc == 4)
	{
		if (strcmp(argv[1], "-c") == 0)
		{
			// -c compile script to bytecode (stdout)

			std::string  inputFileName = argv[2];
			std::string outputFileName = argv[3];

			if (!FileInput::exists(inputFileName)) {
				const char* value = getenv(evDir);
				if (value) 
				{
					inputFileName = separator() + inputFileName;
					inputFileName = value + inputFileName;
					if (!FileInput::exists(inputFileName)) return fileNotFound();
				} else return fileNotFound();
			}
			FileInput input(inputFileName.c_str(), false);
			FileBinaryOutput output(outputFileName.c_str());
			return compileCLIScript(input, output);
		}
#endif
	}
	error();
	return -1;
	//CharInputStream input("t:\"hello!\"");
	//auto error = Paula::one.run(input, false);
	//std::cin.get();  // Waits for Enter key
}
