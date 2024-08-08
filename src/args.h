#pragma once
#include "defs.h"
#include "array.h"
#include "tree.h"

using namespace paula::core;

namespace paula
{
	namespace core
	{
		class Engine;
	}
	class Var
	{
		// handle to data for functions (via Args)
	public:
		INT type() const;
		bool match(INT tag) const;
		bool getInt(INT& out) const;
		bool getDouble(DOUBLE& out) const;
		bool getBool(bool& out) const;
		bool getOp(char& out) const;
		bool getChars(char*&out) const;

		bool isSubtree() const;

		friend class Args;
		friend class core::Tree;
		friend class core::TreeIterator;
	private:
		const INT* ptr; // pointer to a data tag in a tree. volatile, as data can change
		Var(const INT*_ptr);
		Var();
	};

	class Args
	{
	private:
		Array<INT> returnValue;

		Tree& tree;
		INT numArgs;
		core::TreeIterator it;

		static INT emptyData;

		friend class Var;
		friend class core::Engine;

	public:
		Args(Tree&_tree);
		INT count();
		void returnInt(INT);
		bool hasReturnValue();
		void returnBool(bool value);
		Var get(INT dataIndex);
		void reset(INT numArgs);

		friend class Engine;
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