#pragma once
#include "defs.h"
#include "args.h"
#include "tree.h"
namespace paula
{
	class ICallback
	{
	public:
		virtual const Error * execute(Paula& paula, Args& args) = 0;
		INT name[MAX_VAR_NAME_DATA_LENGTH];
	};

	class Callback : public ICallback
	{
		// user-defined callback

		const Error * execute(Paula&,Args&) override;

		friend class Paula;

	private:
		void setup(Array<INT>& _nameData, const Error* (*_action)(Args&));
		Callback();

		const Error * (*action) (Args&); // pointer to callback function

	};

	namespace core
	{
		class Paula;

		class Command : public ICallback
		{
			// system callback
		public:
			Command(const char* _name, const Error * (* _action)(Paula&,Args&));

			void setup(Array<INT>& _nameData, const Error* (*_action)(Paula&, Args&));

			const Error * execute(Paula&,Args&) override;


			friend class Paula;

		private:

			const Error * (*action) (Paula&,Args&); // pointer to callback function

			Command();

			Command& operator=(const Command&) = delete;
		};
	}	
}