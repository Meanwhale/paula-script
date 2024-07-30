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
/*
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
*/

paula::INT paula::Args::emptyData = paula::NODE_VOID | 0; // size=0

paula::Args::Args(Tree& _tree) :
	returnValue(MAX_RETURN_VALUE_SIZE),
	tree(_tree),
	numArgs(0),
	it(_tree)
{
}
INT paula::Args::argCount()
{
	return numArgs;
}
void paula::Args::returnInt(INT value)
{
	returnValue[0] = NODE_INTEGER | 3;
	returnValue[1] = -1; // no parent
	returnValue[2] = -1; // no next
	returnValue[3] = value;
}

bool paula::Args::hasReturnValue()
{
	return returnValue[0] != -1;
}
void paula::Args::returnBool(bool value)
{
	returnValue[0] = NODE_BOOL | 3;
	returnValue[1] = -1; // no parent
	returnValue[2] = -1; // no next
	returnValue[3] = value ? 1 : 0;
}

bool paula::Args::get(INT dataIndex, Data& out)
{
	if (dataIndex < 0 || dataIndex >= numArgs)
	{
		ERR.print("index out of range: ").print(dataIndex).print("/").print(numArgs).endl();
		out.ptr = &NODE_VOID;
		return false;
	}
	ASSERT(tree.stackTopIndex(0) > 0);

	it.index = tree.stackTopIndex(0); // iterator points to first element

	// go to data

	for(INT i=0; i< dataIndex; i++)
	{
		it.next();
	}
	LOG.print("get: ");
	it.print(true);
	LOG.endl();
	
	out.ptr = tree.data.ptr(it.index); // set pointer to the data
	return true;
}

void paula::Args::reset(INT _numArgs)
{
	// _initIndex points one item before first arg.
	numArgs = _numArgs;
	returnValue[0] = -1;
}


paula::Data::Data() : ptr(&Args::emptyData)
{
}

bool paula::Data::match(INT tag)
{
	// check data type match
	return (tag & TAG_MASK) == (*ptr & TAG_MASK);
}

bool paula::Data::getInt(INT& out)
{
	if (!match(NODE_INTEGER)) return false;
	out = *(ptr + 3);
	return true;
}
bool paula::Data::getBool(bool& out)
{
	if (!match(NODE_BOOL)) return false;
	out = (*(ptr + 3)) != 0;
	return true;
}
bool paula::Data::getChars(char*&out)
{
	// out: reference to a pointer.
	// this function could return a pointer (null if not successful)
	// but it would be different from other getters.

	if (!match(NODE_TEXT)) return false;
	out = (char*) (ptr + 4);
	return true;
}

paula::Data::Data(INT* _ptr) : ptr(_ptr)
{
}
