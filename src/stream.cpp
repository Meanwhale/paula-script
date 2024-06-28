#include "stream.h"

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
