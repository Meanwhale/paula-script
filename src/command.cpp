#include"command.h"
#include"utils.h"
#include"array.h"
#include"tree.h"

paula::Command::Command(const char* str, void (* _action)(Paula&,Tree&)) :
	action(_action)
{
	INT length = (INT)strlen(str);
	Array<INT> nameData (name, 12);
	nameData[0] = length;
	bytesToInts((const unsigned char *)str, 0, nameData, 1, length);
}

void paula::Command::execute(Paula& paula, Tree& tree)
{
	LOGLINE("Command: EXECUTE!");
	action(paula,tree);
}

paula::ArgDef::ArgDef(INT size) : types(size)
{
}

bool paula::ArgDef::match(Tree& tree)
{
	TreeIterator it(tree);
	it.toChild();
	return match(it);
}

bool paula::ArgDef::match(TreeIterator& _it)
{
	TreeIterator it(_it); // copy
	INT i=0;
	// check that node types match argdef
	do
	{
		if (i >= types.length()) return false;
		if (!(types[i] == NODE_ANY_DATA) && !it.isType(types[i])) return false;
		i++;
	}
	while(it.next());

	return i == types.length();
}
