#include "paula.h"
#include "engine.h"
#include "stream.h"

using namespace paula;

void paula::printVersion()
{
	pout.print(PAULA_VERSION);
}

const Error* paula::run(const char* code, const char** args, int numArgs)
{
	CharInput input(code);
	return core::Engine::one.run(input, args, numArgs);
}
const Error* paula::run(const char* code)
{
	CharInput input(code);
	return core::Engine::one.run(input);
}
const Error* paula::run(IInputStream&str)
{
	return core::Engine::one.run(str);
}

void paula::runSafe(const char*code)
{
	CharInput input(code);
	core::Engine::one.runSafe(input);
}

void paula::runSafe(IInputStream&str)
{
	core::Engine::one.runSafe(str);
}

Var paula::get(const char* varName)
{
	return core::Engine::one.vars.get(varName);
}

ERROR_STATUS paula::addCallback(const char* callbackName, const Error* (*_action)(Args&))
{
	return core::Engine::one.addCallback(callbackName, _action);
}