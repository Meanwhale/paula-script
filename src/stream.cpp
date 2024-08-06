#include "stream.h"
#include "utils.h"
#include "tree.h"
#include "args.h"
#include <iostream>
#include <fstream>

using namespace paula;
using namespace paula::core;
using namespace std;


const POut& POut::print(int x) const
{
	return print((long)x);
}

const POut& POut::print(float x) const
{
	return print((double)x);
}

const POut& POut::print(bool x) const
{
	return print(x ? "true" : "false");
}
const POut& POut::println(const char* x) const
{
	if (x) return print(x).endl();
	return print("null").endl();
}

const POut& POut::endl() const
{
	return print('\n');
}

// special prints

const POut& POut::print(const TreeIterator& it) const
{
	print(it.var());
	return *this;
}
const POut& POut::print(const Var& x) const
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
const POut& POut::printHex(INT h) const
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

const POut& POut::printCharSymbol(CHAR c) const
{
	if (c>=32 && c<127) print(c); // printable
	else print('#').print(charToInt(c)); // control char: print number
	return *this;
}
const POut& POut::print(const Error* a) const
{
	if (a == nullptr) print("<no error>");
#ifdef PAULA_MINI
	else print(a->id);
#else
	else print(a->name);
#endif
	return *this;
}

// sdt::cout

void STDOut::close() { }
bool STDOut::closed() { return false; }
const POut& STDOut::print(char x) const { std::cout<<x; return *this; }
const POut& STDOut::print(const char* x) const { std::cout<<x; return *this; }
const POut& STDOut::print(long x) const { std::cout<<x; return *this; }
const POut& STDOut::print(double x) const { std::cout<<x; return *this; }


// sdt::cerr

void STDErr::close() { }
bool STDErr::closed() { return false; }
const POut& STDErr::print(char x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(const char* x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(long x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(double x) const { std::cerr<<x; return *this; }

// null printer

const NullPrint& NullPrint::print(const char*) const { return *this; }
const NullPrint& NullPrint::print(double) const { return *this; }
const NullPrint& NullPrint::printHex(INT i) const { return *this; }
const NullPrint& NullPrint::printCharSymbol(CHAR c) const { return *this; }
const NullPrint& NullPrint::print(const Error* a) const { return *this; }
const NullPrint& NullPrint::println(const char*) const { return *this; }
const NullPrint& NullPrint::print(const TreeIterator& x) const{ return *this; }
const NullPrint& NullPrint::endl() const { return *this; }


// const char * input

CharInput::CharInput(const char * _str) :
	str(_str),
	i(0)
{
}

bool CharInput::read(BYTE&c)
{
	c = str[i++];
	return c != '\0';
}

void CharInput::close()
{
}
// standard input

bool StandardInput::read(BYTE&c)
{ 
	if (std::cin.get((char&)c)) return true;
	return false;
}

void StandardInput::close()
{
}

#ifndef PAULA_MINI
// file input

bool FileInput::exists(const std::string& name)
{
	ifstream f(name.c_str());
	bool x = f.good();
	f.close();
	return x;
}
FileInput::FileInput(const char*fn) :
	file(fn),
	found(file.is_open())
{
	if (!found) err.print("FileInput: can't open file: ").print(fn).endl();
}

FileInput::~FileInput()
{
	close();
}

bool FileInput::read(BYTE&c)
{
	if (file.get((char&)c)) return true;
	return false; // Return null character on error
}

void FileInput::close()
{
	if (file.is_open()) file.close();
}
#endif
