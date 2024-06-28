#pragma once

#include <iostream>

namespace paula
{
#define LOG(x) std::cout<<x;
#define LOGLINE(x) std::cout<<x<<std::endl;

#define HALT std::exit(0) 
#define STR(x) #x
#define ASSERT(x,msg) { if (!(x)) { printf("FAIL: (%s), file %s, line %d.\n", STR(x), __FILE__, __LINE__); assert(false, msg); }}

typedef int32_t INT;

void assert(bool, const char *);

class Paula
{
public:

	void run(const char *);
};

}
#include "tree.h"