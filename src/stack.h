#pragma once
#include "array.h"
namespace paula {
	class Var;
	class Args;
	namespace core
	{
		class Stack
		{
		public:
			Stack(INT size);

			void clear();
			void addNode(INT tag, INT size);
			void pushData(INT* src);
			void pushInt(INT value);
			void pushBool(bool value);
			void pushData(TreeIterator&src);
			INT popInt();
			INT previous(INT index);
			INT next(INT index);
			void pop();
			bool stackEmpty();
			INT itemCount();
			INT* topPtr();
			void print();

			friend class StackIterator;
		private:
			Array<INT> data;
			INT top, count;

			Stack() = delete;
			Stack& operator=(const Stack&) = delete;
		};

		class StackIterator
		{
		public:
			StackIterator(Stack&);
			Var var();
			bool next();
			INT type();

			friend class Stack;
			friend class Var;
		private:
			Stack& stack;
			INT* ptr;
		};
	}
}