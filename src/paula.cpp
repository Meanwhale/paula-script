#include "paula.h"
#include "engine.h"
#include "stream.h"

using namespace paula;

void paula::printVersion()
{
	pout.print(PAULA_VERSION);
}

const Error* paula::run(const char* code)
{
	CharInput input(code);
	return core::Engine::one.run(input, false);
}
const Error* paula::run(IInputStream&str)
{
	return core::Engine::one.run(str, false);
}
const Error* paula::newRun(IInputStream&str)
{
	return core::Engine::one.run(str, false);
}

void paula::runSafe(const char*code)
{
	CharInput input(code);
	auto tmp = core::Engine::one.run(input, true);
}

void paula::runSafe(IInputStream&str)
{
	auto tmp = core::Engine::one.run(str, true);
}

Var paula::get(const char* varName)
{
	return core::Engine::one.vars.get(varName);
}

ERROR_STATUS paula::addCallback(const char* callbackName, const Error* (*_action)(Args&))
{
	return core::Engine::one.addCallback(callbackName, _action);
}