#pragma once

#include "defs.h"
#include "args.h"
#include <functional>

namespace paula
{
	class IInputStream;
	class IOutputStream;

	// user interface

	constexpr const char * PAULA_VERSION = "0.2";

	void printVersion();
#ifndef PAULA_MINI

	/**
	*  @brief Run a script with arguments and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error* run(const char* code, const char** args, int numArgs);

	/**
	*  @brief Run a script and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error * run(const char*);

	/**
	*  @brief Run a script and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error * runScript(IInputStream&);
	/**
	*  @brief Run a script and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error * runBytecode(IInputStream&, const char **args, int numArgs);
	/**
	*  @brief Compile script and write bytecode.
	*/
	[[nodiscard]] const Error *  compile(IInputStream&, BinaryOutputStream&);
	/**
	*  @brief Run a script. If an error occurs, print the error.
	*/
	void runSafe(const char*);

	/**
	*  @brief Run a script. If an error occurs, print the error.
	*/
	void runSafe(IInputStream&);

	/**
	*  @brief Get a variable by name. Return 'empty' if the variable is not found.
	*/
	Var get(const char * varName);

	/**
	*  @brief Get a pointer to variable by name. Use for DLL.
	*/
	const INT* getPtr(const char * varName);

	/**
	*  @brief Add a callback to call from a Paula script.
	*/
	/**
	*  @brief Add a callback to call from a Paula script.
	*  Accepts a plain function pointer, a capturing lambda, or any std::function-compatible callable.
	*  From C, use paula_add_callback() in paula_c.h instead.
	*/
	const Error * addCallback(const char* name, std::function<const Error*(Args&)> action);
#endif
}