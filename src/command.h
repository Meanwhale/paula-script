#pragma once
#include "defs.h"
#include "args.h"
#include "tree.h"
#include <functional>
namespace paula
{
	namespace core
	{
		class Engine;

		class ICallback
		{
		public:
			virtual const Error * execute(Engine& paula, Args& args) = 0;
			virtual const bool retunsValue() = 0;
			INT name[MAX_VAR_NAME_DATA_LENGTH]; // NOTE: unnecessary warning about 'name' initialization
		};

		class ProcedureCallback : public ICallback
		{
		public:
			ProcedureCallback();
			ProcedureCallback(char* name, INT _scriptAddress, bool _function);
			const Error * execute(Engine&,Args&) override;
			const bool retunsValue() override;
			INT scriptAddress;
			bool function; // function can return a value and blocks execution
		};
		class Callback : public ICallback
		{
		private:
			// user-defined callback

			const Error * execute(Engine&,Args&) override;
			const bool retunsValue() override;

			friend class Engine;

			void setup(Array<INT>& _nameData, std::function<const Error*(Args&)> _action);
			Callback();

			std::function<const Error*(Args&)> action;
		};


		class Command : public ICallback
		{
			// system callback
		public:
			Command(const char* _name, const Error * (* _action)(Engine&,Args&));

			void setup(Array<INT>& _nameData, const Error* (*_action)(Engine&, Args&));

			const Error * execute(Engine&,Args&) override;
			const bool retunsValue() override;

			friend class Engine;

		private:

			const Error * (*action) (Engine&,Args&); // pointer to callback function

			Command();

			Command& operator=(const Command&) = delete;
		};
	}	
}