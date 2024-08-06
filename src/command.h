#pragma once
#include "defs.h"
#include "args.h"
#include "tree.h"
namespace paula { namespace core
{
	class Paula;

	class Command
	{
	public:
		Command(const char* _name, const Error * (* _action)(Paula&,Args&));

		void setup(Array<INT>& _nameData, const Error* (*_action)(Paula&, Args&));

		const Error *  execute(Paula&,Args&);

		const Error * (*action) (Paula&,Args&); // pointer to callback function

		INT name[MAX_VAR_NAME_DATA_LENGTH];

		// TODO: callback
		// TODO: arg & return types

		friend class Paula;

	private:

		Command();

		Command& operator=(const Command&) = delete;
	};
}}