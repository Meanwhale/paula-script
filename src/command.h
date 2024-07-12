#pragma once
#include "defs.h"
#include "array.h"
namespace paula
{
	class Paula;
	class Tree;
	class TreeIterator;

	class Command
	{
	public:
		Command(const char* _name, void (* _action)(Paula&,Tree&));

		void execute(Paula&,Tree&);

		void (*action) (Paula&,Tree&); // pointer to callback function

		INT name[12];

		// TODO: callback
		// TODO: arg & return types

	private:


		// hide
		Command() = delete;
		Command& operator=(const Command&) = delete;
	};
	/*
	class ArgDef
	{
	public:
		ArgDef(INT size);

		bool match(Tree&);
		bool match(TreeIterator&);

		Array<INT>types;

	private:

		ArgDef() = delete;
	};
	*/
}