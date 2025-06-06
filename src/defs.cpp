#include "defs.h"
#include "array.h"
#include "stream.h"

using namespace paula;

// outputs
STDOut stdOut = STDOut();
STDErr stdErr = STDErr();
STDOut stdPrint = STDOut();

#if PAULA_RELEASE
NullPrint paula::log = paula::NullPrint();
#else
POut& paula::log = stdOut;
#endif

#if PAULA_MINI
NullPrint paula::err = NullPrint();
#else
POut& paula::err = stdErr;
#endif

POut& paula::pout = stdOut;

bool paula::Error::equal(const Error* a, const Error* b)
{
	if (a == nullptr) return b == nullptr;
	if (b == nullptr) return false;
	return a->id == b->id;
}

