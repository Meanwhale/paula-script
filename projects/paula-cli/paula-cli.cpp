#include <iostream>
#include "paula.h"

using namespace paula;

int main()
{
    std::cout << "PAULA CLI: TODO\n";

	CharInputStream input("t:\"hello!\"");
	auto error = Paula::one.run(input, false);
}
