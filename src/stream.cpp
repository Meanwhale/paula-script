#include "stream.h"
#include "utils.h"
#include "tree.h"
#include "args.h"
#include <iostream>
#include <fstream>

using namespace paula;
using namespace paula::core;
using namespace std;

static void encodeInt(INT num, uint8_t bytes[4])
{
	uint32_t u = static_cast<uint32_t>(num);
	bytes[0] = static_cast<uint8_t>(u & 0xFF);
	bytes[1] = static_cast<uint8_t>((u >> 8) & 0xFF);
	bytes[2] = static_cast<uint8_t>((u >> 16) & 0xFF);
	bytes[3] = static_cast<uint8_t>((u >> 24) & 0xFF);
}

void StandardBinaryOutput::flush()
{
	std::cout.flush();
}
void StandardBinaryOutput::close()
{
}
bool StandardBinaryOutput::closed() const
{
	return false;
}

void paula::StandardBinaryOutput::write(INT num)
{
	uint8_t bytes[4];
	encodeInt(num, bytes);
	std::cout.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}


// print out

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
		char * s; if (x.getChars(s)) print(s);
	}
	else if (t == NODE_INTEGER)
	{
		INT i; if (x.getInt(i)) print(i);
	}
	else if (t == NODE_DOUBLE)
	{
		DOUBLE i; if (x.getDouble(i)) print(i);
	}
	else if (t == NODE_BOOL)
	{
		bool b; if (x.getBool(b)) print(b ? "true" : "false");
	}
	else if (t == NODE_RAW_TREE)
	{
		print("<RAW TREE>");
	}
	else if (t == NODE_LOGICAL)
	{
		INT op;
		if (x.getLogical(op))
		{
			switch(op)
			{
			case LOGICAL_AND: print("and"); break;
			case LOGICAL_OR:  print("or" ); break;
			case LOGICAL_XOR: print("xor"); break;
			default: print("???");
			}
		}
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
		print("<! ! ! TreeIterator::print: unknown node ! ! !>");
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
	uint32_t u = static_cast<uint32_t>(h);
	for (int i = 28; i >= 0; i -= 4)
	{
		int index = (u >> i) & 0x0F;
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

void STDOut::flush() { std::cout<<std::flush; }
void STDOut::close() { }
bool STDOut::closed() const { return false; }
const POut& STDOut::print(char x) const { std::cout<<x; return *this; }
const POut& STDOut::print(const char* x) const { std::cout<<x; return *this; }
const POut& STDOut::print(long x) const { std::cout<<x; return *this; }
const POut& STDOut::print(double x) const { std::cout<<x; return *this; }


// sdt::cerr

void STDErr::flush() { std::cerr<<std::flush; }
void STDErr::close() { }
bool STDErr::closed() const { return false; }
const POut& STDErr::print(char x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(const char* x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(long x) const { std::cerr<<x; return *this; }
const POut& STDErr::print(double x) const { std::cerr<<x; return *this; }

// null printer

void             NullPrint::flush() const { }
const NullPrint& NullPrint::print(char) const { return *this; }
const NullPrint& NullPrint::print(const char*) const { return *this; }
const NullPrint& NullPrint::print(int) const { return *this; }
const NullPrint& NullPrint::print(float) const { return *this; }
const NullPrint& NullPrint::print(bool) const { return *this; }
const NullPrint& NullPrint::print(double) const { return *this; }
const NullPrint& NullPrint::printHex(INT i) const { return *this; }
const NullPrint& NullPrint::printCharSymbol(CHAR c) const { return *this; }
const NullPrint& NullPrint::print(const Error* a) const { return *this; }
const NullPrint& NullPrint::println(const char*) const { return *this; }
const NullPrint& NullPrint::print(const TreeIterator& x) const { return *this; }
const NullPrint& NullPrint::print(const Var& x) const { return *this; }
const NullPrint& NullPrint::endl() const { return *this; }

// binary input

bool IInputStream::readInt(INT&out)
{
	BYTE b0, b1, b2, b3;
	if (!read(b0)) return false;
	if (!read(b1)) return false;
	if (!read(b2)) return false;
	if (!read(b3)) return false;

	out = static_cast<uint32_t>(b0)
		| (static_cast<uint32_t>(b1) << 8)
		| (static_cast<uint32_t>(b2) << 16)
		| (static_cast<uint32_t>(b3) << 24);

	return true;
}

// const char * input

CharInput::CharInput(const char * _str) :
	str(_str),
	i(0)
{
}

bool CharInput::read(BYTE&c)
{
	if (str[i] == '\0') return false;
	c = str[i++];
	return true;
}

void CharInput::close()
{
}
// standard input

bool StandardInput::read(BYTE&c)
{
	char temp;
	if (std::cin.get(temp)) { c = static_cast<unsigned char>(temp); return true; }
	return false;
}

void StandardInput::close()
{
}
// standard binary input

bool StandardBinaryInput::read(BYTE&c)
{ 
	char temp;
	if (std::cin.read(&temp, 1))
	{
		c = static_cast<unsigned char>(temp);
		return true;
	}
	return false;
}

void StandardBinaryInput::close()
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
FileInput::FileInput(const char*fn, bool binary)
{	
	std::ios::openmode mode = std::ios::in;
	if (binary) {
		mode |= std::ios::binary;
	}
	file.open(fn, mode);
}

FileInput::~FileInput()
{
	// close();
}

bool FileInput::read(BYTE&c)
{
	char temp;
	if (file.get(temp)) { c = static_cast<unsigned char>(temp); return true; }
	return false;
}

void FileInput::close()
{
	if (file.is_open()) file.close();
}
#endif


// file binary out

FileBinaryOutput::FileBinaryOutput(const char * fileName) : out(fileName, std::ios::binary)
{
}
void FileBinaryOutput::flush() 
{
	if (!closed()) out.flush();
}
void FileBinaryOutput::close() 
{ 
	if (!closed()) out.close();
}
bool FileBinaryOutput::closed() const
{
	return !out.is_open();
}
void FileBinaryOutput::write(INT num)
{
	uint8_t bytes[4];
	encodeInt(num, bytes);
	out.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
}


// binary out

void paula::BinaryOutputStream::writeArray(INT * data, INT size)
{
	for (INT i=0; i<size; i++) write(data[i]);

	//std::cout.write(reinterpret_cast<const char*>(data), size * sizeof(INT));
}


#ifndef PAULA_MINI
// testing I/O

paula::ArrayBinaryOutput::ArrayBinaryOutput() : i(0), buffer(10000)
{
}
void paula::ArrayBinaryOutput::flush()
{
}
void paula::ArrayBinaryOutput::close()
{
}
bool paula::ArrayBinaryOutput::closed() const
{
	return false;
}
void paula::ArrayBinaryOutput::write(INT number)
{
	ASSERT_MSG(i + 4 <= buffer.length(), "ArrayBinaryOutput overflow");
	uint8_t bytes[4];
	encodeInt(number, bytes);
	buffer[i++] = bytes[0];
	buffer[i++] = bytes[1];
	buffer[i++] = bytes[2];
	buffer[i++] = bytes[3];
}
INT paula::ArrayBinaryOutput::getByteSize() const
{
	return i;
}
paula::ArrayBinaryInput::ArrayBinaryInput(paula::core::Array<uint8_t>& _buffer, INT _size) : i(0), size(_size), buffer(_buffer)
{
}
bool paula::ArrayBinaryInput::read(BYTE&output)
{
	if (i >= size) return false;
	output = buffer[i++];
	return true;
}
bool paula::ArrayBinaryInput::readInt(INT&output)
{
	if (i + 4 > size) return false;
	output =
		 static_cast<uint32_t>(buffer[i])           |
		(static_cast<uint32_t>(buffer[i + 1]) <<  8) |
		(static_cast<uint32_t>(buffer[i + 2]) << 16) |
		(static_cast<uint32_t>(buffer[i + 3]) << 24);

	i += 4;
	return true;
}
void paula::ArrayBinaryInput::close()
{
}
#endif