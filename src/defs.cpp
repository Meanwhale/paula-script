#include "defs.h"
#include "array.h"

bool paula::Error::equal(const Error* a, const Error* b)
{
	if (a == nullptr) return b == nullptr;
	if (b == nullptr) return false;
	return a->id == b->id;
}

constexpr int MAX_KEYWORDS = 32;
paula::INT* paula::keywords[MAX_KEYWORDS];
int keywordCount = 0;

void paula::addKeyword(INT* w)
{
	ASSERT(keywordCount < MAX_KEYWORDS);
	keywords[keywordCount] = w;
	keywordCount++;
}
