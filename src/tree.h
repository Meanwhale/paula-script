#pragma once
#include "array.h"
#include "node_types.h"
namespace paula {
	class Var;
	class Args;
	namespace core
	{
		class TreeIterator;
	
		class Tree
		{
		public:
			Tree(INT size);
			Tree(INT * a, int s);
			void addInt(INT parentIndex, INT data);
			void addBool(INT parentIndex, bool data);
			void addOperatorNode(INT parentIndex, CHAR op);
			void addDouble(INT parentIndex, double value);
			void addRawTree(INT parentIndex, Tree& tree);
			void addText(INT parentIndex, const char* text);
			void addText(INT parentIndex, const unsigned char * bytes, INT first, INT last, INT nodeType);
			void addData(INT parentIndex, Var src);
			void addData(INT stackIndex, TreeIterator& src);
			INT addSubtree(INT parentIndex, INT type);
			bool hasCapacity(INT size);


			// map functions

			Var get(const char * varName);

			Var getAt(INT dataIndex);

			//bool getBool(bool& out, const char* varName);
	  //      bool getDouble(double& out, const char* varName);
			//bool getInt(int& out, const char* varName);
	  //      bool getChars(char*& out, const char* varName);

			INT getIndexOfData(const char * varName);


			// accessors

			INT getType(INT index);
			INT getNodeSize(INT index);
			bool isSubtree(INT nodeIndex);
			bool isSubtreeTag(INT tag);
			INT nodeSize(INT node);

			// utils

			void init(INT parentType);
			void clear();
			bool isClear();
			void printData();
			void print();
			void printSubtree(TreeIterator&);
			void printCompact(TreeIterator&);

			friend class Stack;
			friend class StackIterator;
			friend class TreeIterator;
			friend class Engine;
			friend class paula::Var;
			friend class paula::Args;

		private:
			Array<INT> data;
			INT top;
			INT node(INT tag, INT size);
			void insertToTree(INT parentIndex, INT tag, INT size);

			//void printNode(INT index, INT depth);

			Tree() = delete;
			Tree& operator=(const Tree&) = delete;
		};

		class TreeIterator
		{
		public:
			TreeIterator(const TreeIterator&);
			TreeIterator(Tree&);
			TreeIterator(Tree& _tree, INT _index);

			Var var() const;

			void printTree(bool compact);

			bool next(); // goes to next sibling. returns true if there's a sibling.

			void toChild();
			void toParent();
			void jumpTo(INT);

			bool hasNext();
			bool hasChild();
			bool hasParent();
			bool isType(INT nodeType);
			bool isNextType(INT nodeType);
			bool isTextType();
			INT type();
			INT size();

			void overwrite(Var src);

			// data getters

			INT getDepth();

			INT* getTextData();

			friend class Stack;
			friend class Tree;
			friend class Engine;
			friend class paula::Var;
			friend class paula::Args;

		private:

			Tree& tree;
			INT index, depth;

			//TreeIterator() = delete;
			//TreeIterator& operator=(const TreeIterator&) = delete;
		};
	}
}