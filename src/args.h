#pragma once
#include "defs.h"
#include "array.h"
#include "tree.h"
#include "stack.h"

using namespace paula::core;

namespace paula
{
	namespace core
	{
		class Engine;
	}
	/**
	 * @brief Access Paula script variable data.
	 */
	class Var
	{
	public:
		/**
		 * @brief Get variable type, eg. NODE_INTEGER. See node_types.h.
		 * @return Node (variable) type.
		 */
		INT type() const;
		/**
		* @brief Get node size.
		* @return Node size.
		*/
		INT size() const;
		/**
		 * @brief Compare node (variable) type.
		 * @param tag eg. NODE_INTEGER. See node_types.h.
		 * @return True if types match.
		 */
		bool match(INT tag) const;
		/**
		 * @brief Write variable's integer (32 bits) value to reference, if variable is of correct type.
		 * @param out Write target. If type doesn't match, the value is not changed.
		 * @return True if type is correct and the value is written.
		 */
		bool getInt(INT& out) const;
		/**
		* @brief Write variable's double (64-bit floating point) value to reference, if variable is of correct type.
		* @param out Write target. If type doesn't match, the value is not changed.
		* @return True if type is correct and the value is written.
		*/
		bool getDouble(DOUBLE& out) const;
		/**
		* @brief Write variable's boolean value to reference, if variable is of correct type.
		* @param out Write target. If type doesn't match, the value is not changed.
		* @return True if type is correct and the value is written.
		*/
		bool getBool(bool& out) const;
		/**
		* @brief Write variable's operation character (eg. '+' or '<') to reference, if variable is of correct type.
		* @param out Write target. If type doesn't match, the value is not changed.
		* @return True if type is correct and the value is written.
		*/
		bool getOp(char& out) const;
		/**
		* @brief Write variable's text value (char*) to reference, if variable is of correct type.
		* @param out Write target. If type doesn't match, the value is not changed.
		* @return True if type is correct and the value is written.
		*/
		bool getChars(char*&out) const;
		/**
		 * @brief Check if a variable is a subtree.
		 * @return True if variable is a subtree.
		 */
		bool isSubtree() const;

		friend class Args;
		friend class core::Tree;
		friend class core::TreeIterator;
		friend class core::StackIterator;
		friend class core::Engine;
	private:
		const INT* ptr; // pointer to a data tag in a tree. volatile, as data can change
		Var(const INT*_ptr);
		Var();
	};
	/**
	 * @brief Access callback arguments (e.g. value of "a" in "f(a)"). Set return value of the callback function, called from a script.
	 */
	class Args
	{
	public:
		/**
		 * @brief Get number of arguments, eg. 3 for "f(a, b, c)"
		 * @return Argument count.
		 */
		INT count();
		/**
		 * @brief Set return value, returned by a callback.
		 * @param value Integer value.
		 */
		void returnInt(INT value);
		/**
		* @brief Set return value, returned by a callback.
		* @param value Boolean value.
		*/
		void returnBool(bool value);
		/**
		 * @brief Check if a return value is set.
		 * @return True if a return value is set.
		 */
		bool hasReturnValue();
		/**
		 * @brief Get argument value, eg. get(2) to value of c if the call is "f(a, b, c)".
		 * @param dataIndex Argument index, starting from 0. 0 <= dataIndex < count().
		 * @return Argument value, or void (NODE_VOID) value doesn't exist. See class Var.
		 */
		Var get(INT dataIndex);

		friend class Engine;

	private:
		Args(Stack&_tree);
		void reset(INT numArgs);

		Array<INT> returnValue;

		Stack& stack;
		INT numArgs;

		Args() = delete;
		static INT emptyData;

		friend class Var;
		friend class core::Engine;

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