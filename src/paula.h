#pragma once

#include "defs.h"
#include "args.h"

namespace paula
{
	class IInputStream;

	// user interface

	constexpr const char * PAULA_VERSION = "0.1";

	void printVersion();

	/**
	*  @brief Run a script and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error * run(const char*);

	/**
	*  @brief Run a script and return error, or NO_ERROR, if everything went well.
	*/
	[[nodiscard]] const Error * run(IInputStream&);

    const Error* newRun(IInputStream& str);
	
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
	*  @brief Add a callback to call from a Paula script.
	*/
	const Error * addCallback(const char* name, const Error* (*_action)(Args&));
}