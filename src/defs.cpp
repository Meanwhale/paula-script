#include "defs.h"
#include "array.h"
#include "stream.h"

using namespace paula;

// outputs
STDOut stdOut = STDOut();
STDErr stdErr = STDErr();
STDOut stdPrint = STDOut();
#if PAULA_RELEASE
const NullPrint paula::log = NullPrint();
#else
const POut& paula::log = stdOut;
#endif
const POut& paula::err = stdErr;
const POut& paula::pout = stdOut;

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
