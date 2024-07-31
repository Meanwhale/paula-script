#include "stream.h"
#include "utils.h"
#include "tree.h"
#include "args.h"
#include <iostream>

using namespace paula;

paula::CharInputStream::CharInputStream(const char * _str) :
	str(_str),
	i(0)
{
}

CHAR paula::CharInputStream::read()
{
    return str[i++];
}

bool paula::CharInputStream::end()
{
    return str[i] == '\0';
}

void paula::CharInputStream::close()
{
}
/*
paula::BufferInputStream::BufferInputStream(Array<CHAR>& _str, INT _start, INT _last) :
	str(_str.get(), _str.length()),
	i(_start),
	last(_last)
{
}

CHAR paula::BufferInputStream::read()
{
	return str[i++];
}

bool paula::BufferInputStream::end()
{
	return i > last;
}

void paula::BufferInputStream::close()
{
}
*/
const POut& paula::POut::print(int x) const
{
	return print((long)x);
}

const POut& paula::POut::print(float x) const
{
	return print((double)x);
}

const POut& paula::POut::print(bool x) const
{
	return print(x ? "true" : "false");
}
const POut& paula::POut::println(const char* x) const
{
	if (x) return print(x).endl();
	return print("null").endl();
}

const POut& paula::POut::endl() const
{
	return print('\n');
}

// special prints

const POut& paula::POut::print(const TreeIterator& it) const
{
	print(it.var());
	return *this;
}
const POut& paula::POut::print(const Var& x) const
{
	INT t = x.type();

	if (t == NODE_TEXT || t == NODE_NAME)
	{
		char * s; if (x.readChars(s)) print(s);
	}
	else if (t == NODE_INTEGER)
	{
		INT i; if (x.getInt(i)) print(i);
	}
	else if (t == NODE_BOOL)
	{
		bool b; if (x.getBool(b)) print(b ? "true" : "false");
	}
	else if (t == NODE_OPERATOR)
	{
		char op; if (x.getOp(op)) print(op);
	}
	else if (x.isSubtree())
	{
		print(treeTypeName(t));
	}
	else
	{
		LOG.print("<! ! ! TreeIterator::print: unknown node ! ! !>");
	}
	return *this;
}

char hexs[] = 
{
		'0','1','2','3',
		'4','5','6','7',
		'8','9','a','b',
		'c','d','e','f'
};
const POut& paula::POut::printHex(INT h) const
{
	print("0x");
	for (INT i = 28; i >= 0; i -= 4)
	{
		int index = (h >> i);
		index &= 0x0000000f;
		print(hexs[index]);
	}
	return *this;
}

const POut& paula::POut::printCharSymbol(CHAR c) const
{
	if (c>=32 && c<127) print(c); // printable
	else print('#').print(charToInt(c)); // control char: print number
	return *this;
}
const POut& paula::POut::print(const Error* a) const
{
	if (a == nullptr) print("<no error>");
	else print(a->name);
	return *this;
}

// sdt::cout

void paula::STDOut::close() { }
bool paula::STDOut::closed() { return false; }
const POut& paula::STDOut::print(char x) const { std::cout<<x; return *this; }
const POut& paula::STDOut::print(const char* x) const { std::cout<<x; return *this; }
const POut& paula::STDOut::print(long x) const { std::cout<<x; return *this; }
const POut& paula::STDOut::print(double x) const { std::cout<<x; return *this; }


// sdt::cerr

void paula::STDErr::close() { }
bool paula::STDErr::closed() { return false; }
const POut& paula::STDErr::print(char x) const { std::cerr<<x; return *this; }
const POut& paula::STDErr::print(const char* x) const { std::cerr<<x; return *this; }
const POut& paula::STDErr::print(long x) const { std::cerr<<x; return *this; }
const POut& paula::STDErr::print(double x) const { std::cerr<<x; return *this; }

// null printer

const NullPrint& paula::NullPrint::print(const char*) const { return *this; }
const NullPrint& paula::NullPrint::print(double) const { return *this; }
const NullPrint& paula::NullPrint::printHex(INT i) const { return *this; }
const NullPrint& paula::NullPrint::printCharSymbol(CHAR c) const { return *this; }
const NullPrint& paula::NullPrint::print(const Error* a) const { return *this; }
const NullPrint& paula::NullPrint::println(const char*) const { return *this; }
const NullPrint& paula::NullPrint::endl() const { return *this; }
