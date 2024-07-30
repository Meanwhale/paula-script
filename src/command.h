#pragma once
#include "defs.h"
#include "tree.h"
namespace paula
{
	class Paula;
	class Tree;
	class TreeIterator;
	class Args;

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


	class Data
	{
	public:
		Data();

		bool match(INT tag);
		bool getInt(INT& out);

        bool getBool(bool& out);

		bool getChars(char*&out);

		friend class Args;
	private:
		const INT* ptr; // pointer to a data tag in a tree. volatile, as data can change
		Data(INT*_ptr);

	};

	class Args
	{
	private:
		Array<INT> returnValue;

		Tree& tree;
		INT numArgs;
		TreeIterator it;

		static INT emptyData;

		friend class Data;
	
	public:
		Args(Tree&_tree);
		INT argCount();
		void returnInt(INT);
		bool hasReturnValue();
		void returnBool(bool value);
		bool get(INT index, Data& out);
		void reset(INT numArgs);

		friend class Paula;
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