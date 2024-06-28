#include "paula.h"
namespace paula
{
void assert(bool x, const char * msg)
{
	if (!x)
	{
		std::cout<<msg<<std::endl;
		HALT;
	}
}

void Paula::run(const char* script)
{
	LOG("RUN: ");
	LOGLINE(script);


}
}