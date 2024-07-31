#include "args.h"

using namespace paula;

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

Var paula::Args::get(INT dataIndex)
{
	if (dataIndex < 0 || dataIndex >= numArgs)
	{
		ERR.print("index out of range: ").print(dataIndex).print("/").print(numArgs).endl();
		return Var(&NODE_VOID);
	}
	ASSERT(tree.stackTopIndex(0) > 0);

	it.index = tree.stackTopIndex(0); // iterator points to first element

	// go to data

	for(INT i=0; i< dataIndex; i++)
	{
		it.next();
	}
	LOG.print("get: ").print(it).endl();

	return Var(tree.data.ptr(it.index)); // set pointer to the data
}

void paula::Args::reset(INT _numArgs)
{
	// _initIndex points one item before first arg.
	numArgs = _numArgs;
	returnValue[0] = -1;
}

paula::Var::Var(const INT* _ptr) : ptr(_ptr)
{
}

paula::Var::Var() : ptr(&Args::emptyData)
{
}

INT paula::Var::type() const
{
	return *ptr & TAG_MASK;
}
bool paula::Var::match(INT tag) const
{
	// check data type match
	return (tag & TAG_MASK) == (*ptr & TAG_MASK);
}
bool paula::Var::getInt(INT& out) const
{
	return readInt(out, ptr);
}
bool paula::Var::getBool(bool& out) const
{
	return readBool(out, ptr);
}
bool paula::Var::getOp(char& out) const
{
	return readOp(out, ptr);
}
bool paula::Var::readChars(char*&out) const
{
	// out: reference to a pointer.
	// this function could return a pointer (null if not successful)
	// but it would be different from other getters.

	if (!(match(NODE_TEXT) || match(NODE_NAME))) return false;
	out = (char*) (ptr + 4);
	return true;
}
bool paula::Var::isSubtree() const
{
	return (type() & 0xf0ffffff) == 0;
}