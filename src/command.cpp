#include"command.h"
#include"utils.h"
#include"array.h"
#include"tree.h"
#include"stream.h"
#include"paula.h"

using namespace paula;

paula::Command::Command() :
	action(0)
{
}
paula::Command::Command(const char* str, const Error * (* _action)(Paula&,Args&))
{
	setup(str, _action);
}
void paula::Command::setup(const char* str, const Error * (* _action)(Paula&,Args&))
{
	action = _action;
	INT length = (INT)strlen(str);
	Array<INT> nameData (name, 12);
	nameData[0] = length;
	bytesToInts((const unsigned char *)str, 0, nameData, 1, length);
	addKeyword(name);
}
const paula::Error * paula::Command::execute(Paula& paula, Args& args)
{
	LOG.println("Command: EXECUTE!");
	return action(paula,args);
}