#include "paula.h"
#include "engine.h"
#include "stream.h"

using namespace paula;

void paula::printVersion()
{
	pout.print(PAULA_VERSION);
}

#ifndef PAULA_MINI
const Error* paula::run(const char* code, const char** args, int numArgs)
{
	CharInput input(code);
	return core::Engine::one.runScript(input, args, numArgs);
}
const Error* paula::run(const char* code)
{
	CharInput input(code);
	return core::Engine::one.runScript(input);
}
const Error* paula::runScript(IInputStream&str)
{
	return core::Engine::one.runScript(str);
}
const Error* paula::runBytecode(IInputStream&str, const char **args, int numArgs)
{
	return core::Engine::one.runBytecode(str, args, numArgs);
}
const Error *  paula::compile(IInputStream&in, BinaryOutputStream&out)
{
	return core::Engine::one.compile(in, out);
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

const INT* paula::getPtr(const char* varName)
{
	auto var = get(varName);
	return var.getPtr();
}

ERROR_STATUS paula::addCallback(const char* callbackName, const Error* (*_action)(Args&))
{
	return core::Engine::one.addCallback(callbackName, _action);
}
#endif