#include"command.h"
#include"utils.h"
#include"array.h"
#include"tree.h"
#include"stream.h"
#include"paula.h"
#include <cstring> 

using namespace paula;

paula::Command::Command() :
	action(0)
{
}
paula::Command::Command(const char* str, const Error * (* _action)(Paula&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	charsToNameData(str, nameData);
	action = _action;
}
void paula::Command::setup(Array<INT>& _nameData, const Error * (* _action)(Paula&,Args&))
{
	Array<INT> nameData (name, MAX_VAR_NAME_DATA_LENGTH);
	nameData.copyFrom(_nameData);
	action = _action;
}
const paula::Error * paula::Command::execute(Paula& paula, Args& args)
{
	LOG.println("Command: EXECUTE!");
	return action(paula,args);
}