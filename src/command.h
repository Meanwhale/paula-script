#pragma once
#include "defs.h"
namespace paula
{
	class Paula;
	class Tree;

	class Command
	{
	public:
		Command(const char*);

		void execute(Paula&,Tree&);

		// TODO: callback
		// TODO: arg & return types

	private:

		INT name[12];
	};
}