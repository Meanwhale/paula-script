#include "defs.h"
#include "stream.h"

bool paula::Error::equal(const Error* a, const Error* b)
{
	if (a == nullptr) return b == nullptr;
	if (b == nullptr) return false;
	return a->id == b->id;
}
