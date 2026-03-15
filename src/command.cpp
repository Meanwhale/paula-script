#include"command.h"
#include"utils.h"
#include"array.h"
#include"tree.h"
#include"stream.h"
#include"engine.h"
#include <cstring> 

using namespace paula;

core::Command::Command() :
	action(0)
{
}
core::Command::Command(const char* str, const Error * (* _action)(Engine&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(str, nameData);
	action = _action;
}
void core::Command::setup(Array<INT>& _nameData, const Error * (* _action)(Engine&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	nameData.copyFrom(_nameData);
	action = _action;
}
const Error * core::Command::execute(Engine& paula, Args& args)
{
	LOG.println("Command: EXECUTE!");
	return action(paula,args);
}

const bool paula::core::Command::retunsValue()
{
	return true;
}

const Error* core::Callback::execute(Engine&, Args&args)
{
	LOG.println("Callback: EXECUTE!");
	return action(args);
}

const bool paula::core::Callback::retunsValue()
{
	return true;
}

void core::Callback::setup(Array<INT>& _nameData, std::function<const Error*(Args&)> _action)
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	nameData.copyFrom(_nameData);
	action = _action;
}

core::Callback::Callback()
{
}

const Error* paula::core::ProcedureCallback::execute(Engine&engine, Args&args)
{
	return engine.callProcedure(scriptAddress, args, function);
}

const bool paula::core::ProcedureCallback::retunsValue()
{
	return function;
}

paula::core::ProcedureCallback::ProcedureCallback(): scriptAddress(-1), function(false)
{
}

paula::core::ProcedureCallback::ProcedureCallback(char* str, INT _scriptAddress, bool _function): scriptAddress(_scriptAddress)
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(str, nameData);
	function = _function;
}
