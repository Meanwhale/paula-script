#pragma once
#include "defs.h"
#include "args.h"
#include "tree.h"
namespace paula
{
	namespace core
	{
		class Engine;

		class ICallback
		{
		public:
			virtual const Error * execute(Engine& paula, Args& args) = 0;
			INT name[MAX_VAR_NAME_DATA_LENGTH];
		};

		class Callback : public ICallback
		{
			// user-defined callback

			const Error * execute(Engine&,Args&) override;

			friend class Engine;

		private:
			void setup(Array<INT>& _nameData, const Error* (*_action)(Args&));
			Callback();

			const Error * (*action) (Args&); // pointer to callback function

		};


		class Command : public ICallback
		{
			// system callback
		public:
			Command(const char* _name, const Error * (* _action)(Engine&,Args&));

			void setup(Array<INT>& _nameData, const Error* (*_action)(Engine&, Args&));

			const Error * execute(Engine&,Args&) override;

			friend class Engine;

		private:

			const Error * (*action) (Engine&,Args&); // pointer to callback function

			Command();

			Command& operator=(const Command&) = delete;
		};
	}	
}