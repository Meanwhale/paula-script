#pragma once
#include "defs.h"
#include "args.h"
#include "tree.h"
namespace paula
{
	class Paula;

	class Command
	{
	public:
		Command(const char* _name, const Error * (* _action)(Paula&,Args&));

		void setup(const char* callbackName, const Error* (*_action)(Paula&, Args&));

		const Error *  execute(Paula&,Args&);

		const Error * (*action) (Paula&,Args&); // pointer to callback function

		INT name[12];

		// TODO: callback
		// TODO: arg & return types

		friend class Paula;

	private:

		Command();

		Command& operator=(const Command&) = delete;
	};
}