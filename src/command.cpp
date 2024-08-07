#include"command.h"
#include"utils.h"
#include"array.h"
#include"tree.h"
#include"stream.h"
#include"paula.h"
#include <cstring> 

using namespace paula;

core::Command::Command() :
	action(0)
{
}
core::Command::Command(const char* str, const Error * (* _action)(Paula&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(str, nameData);
	action = _action;
}
void core::Command::setup(Array<INT>& _nameData, const Error * (* _action)(Paula&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	nameData.copyFrom(_nameData);
	action = _action;
}
const Error * core::Command::execute(Paula& paula, Args& args)
{
	LOG.println("Command: EXECUTE!");
	return action(paula,args);
}

const Error* core::Callback::execute(Paula&, Args&args)
{
	LOG.println("Callback: EXECUTE!");
	return action(args);
}

void core::Callback::setup(Array<INT>& _nameData, const Error* (*_action)(Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	nameData.copyFrom(_nameData);
	action = _action;
}

core::Callback::Callback() :
	action(0)
{
}
