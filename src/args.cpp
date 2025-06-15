#include "args.h"
#include "engine.h"
#include "stream.h"

using namespace paula;
using namespace paula::core;

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

INT Args::emptyData = paula::NODE_VOID | 0; // size=0


paula::Args::Args() :
	engine(nullptr),
	stackBase(nullptr)
{
}

paula::Args::Args(Engine*_engine, INT* _stackBase) :
	engine(_engine),
	stackBase(_stackBase)
{
	engine->returnValue.clear();
}
INT Args::count()
{
	// read number of args on top of the args stack
	INT numArgs;
	if (readInt(numArgs, stackBase)) return numArgs;
	ASSERT(false);
	return 0;	
}
void Args::returnInt(INT value)
{
	engine->returnValue.pushInt(value);
	//engine->returnValue[0] = NODE_INTEGER | 3;
	//engine->returnValue[1] = -1; // no parent
	//engine->returnValue[2] = -1; // no next
	//engine->returnValue[3] = value;
}

bool Args::hasReturnValue()
{
	return engine->returnValue.itemCount() > 0;
}
void Args::returnData(Var x)
{
	engine->returnValue.pushData(x.ptr);
}
void Args::returnBool(bool value)
{
	engine->returnValue.pushBool(value);
	//engine->returnValue[0] = NODE_BOOL | 3;
	//engine->returnValue[1] = -1; // no parent
	//engine->returnValue[2] = -1; // no next
	//engine->returnValue[3] = value ? 1 : 0;
}

Var Args::get(INT dataIndex)
{
	INT numArgs = count();
	if (dataIndex < 0 || dataIndex >= numArgs)
	{
		ERR.print("index out of range: ").print(dataIndex).print("/").print(numArgs).endl();
		return Var(&NODE_VOID);
	}
	dataIndex = numArgs - dataIndex - 1; // reverse order 

	StackIterator it(engine->stack, stackBase); // iterator points to top = arg. count

	// go to data. it points to data before first argument

	for(INT i=0; i<=dataIndex; i++)
	{
		bool hasNext = it.next();
		ASSERT(hasNext);
	}
	//LOG.print("get: ").print(it.var()).endl();

	return it.var(); // set pointer to the data
}

void Args::print()
{
	LOG.print("ARGS: count ").print(count()).endl();
	for(INT i=0; i<count(); i++)
	{
		LOG.print("  ").print(i).print(": ").print(get(i)).endl();
	}
}

//////////////// VAR

Var::Var(const INT* _ptr) : ptr(_ptr)
{
}

Var::Var() : ptr(&Args::emptyData)
{
}

INT Var::type() const
{
	return *ptr & NODE_TYPE_MASK;
}
INT Var::size() const
{
	return *ptr & SIZE_MASK;
}
bool Var::match(INT tag) const
{
	// check data type match
	return (tag & NODE_TYPE_MASK) == (*ptr & NODE_TYPE_MASK);
}
bool Var::getInt(INT& out) const
{
	return readInt(out, ptr);
}
bool paula::Var::getDouble(DOUBLE& out) const
{
	return readDouble(out, ptr);
}
bool Var::getBool(bool& out) const
{
	return readBool(out, ptr);
}
bool Var::getOp(char& out) const
{
	return readOp(out, ptr);
}
bool Var::getLogical(INT& out) const
{
	return readLogical(out, ptr);
}
bool Var::getChars(char*&out) const
{
	// out: reference to a pointer.
	// this function could return a pointer (null if not successful)
	// but it would be different from other getters.

	if (!(match(NODE_TEXT) || match(NODE_NAME))) return false;
	out = (char*) (ptr + 4);
	return true;
}
bool Var::isSubtree() const
{
	return (type() & 0xf0ffffff) == 0;
}

const INT* paula::Var::getPtr()
{
	return ptr;
}
