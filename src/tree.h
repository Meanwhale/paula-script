#pragma once
#include "array.h"
namespace paula
{
	// tags

	constexpr INT
		
		// tree types

		NODE_SUBTREE			= 0x01000000, // parser decides what kind or tree is it
		NODE_EXPR				= 0x02000000,
		NODE_ASSIGNMENT			= 0x03000000,
		NODE_COMMAND			= 0x04000000,

		NODE_STACK				= 0x11000000,

		// data types

		NODE_NAME				= 0x21000000, // variable, constant, or function name
		NODE_INTEGER			= 0x22000000, // e.g. 123
		NODE_FLOAT				= 0x23000000, // e.g. 1.23
		NODE_TEXT				= 0x24000000, // e.g. "abc d"

		// operators

		NODE_PLUS				= 0x31000000,
		NODE_MINUS				= 0x32000000,
		NODE_MUL				= 0x33000000,
		NODE_DIV				= 0x34000000,

		//NODE_PARENTHESIS		= 0x00000000,
		//NODE_SQUARE_BRACKETS	= 0x00000000,
		//NODE_CURLY_BRACKETS	= 0x00000000,

		// masks
		TAG_MASK	= 0xff000000,
		SIZE_MASK	= 0x00ffffff;

	class TreeIterator;

	class Tree
	{
	public:
		Tree(INT size);

		void addInt(INT parentIndex, INT data);
		void addText(INT parentIndex, Array<BYTE>& src, INT first, INT last, INT nodeType);
		INT addSubtree(INT parentIndex, INT type);

		// stack functions

		void pushInt(INT stackIndex, INT value);
		void pushData(INT stackIndex, TreeIterator&src);
		INT stackTopIndex(INT stackIndex);
		void pop(INT stackIndex);
		bool stackEmpty(INT stackIndex);

		INT stackSize(INT stackIndex);


		INT get(INT index);
        INT getType(INT index);
		INT getNodeSize(INT index);
		bool isSubtree(INT nodeIndex);
		bool isStack(INT nodeIndex);
		bool isSubtreeTag(INT tag);
		INT maskNodeTag(INT node);
		INT nodeSize(INT node);
		void init(INT parentType);
		void clear();
		bool isClear();
		void printData();
		void print();
		void printSubtree(TreeIterator&);
		void printCompact(TreeIterator&);
		const char * treeTypeName(INT tag); 

		friend class TreeIterator;

	private:
		Array<INT> data;
		INT top;
		INT node(INT tag, INT size);
		void pushStack(INT parentIndex, INT tag, INT size);
		void insertTree(INT parentIndex, INT tag, INT size);

		//void printNode(INT index, INT depth);

		Tree() = delete;
		Tree& operator=(const Tree&) = delete;
	};
	class TreeIterator
	{
	public:
		TreeIterator(const TreeIterator&);
		TreeIterator(Tree&);

		void print(bool compact);

		bool next(); // goes to next sibling. returns true if there's a sibling.

		void toChild();
		void toParent();

		bool hasNext();
		bool hasChild();
		bool hasParent();
		bool isType(INT nodeType);
		bool isTextType();
		INT type();
		INT size();

		// data getters

		INT getDepth();

		INT getInt();
		const char * getText();

		bool matchTextData(INT*data);

		friend class Tree;

	private:

		Tree& tree;
		INT index, depth;

		TreeIterator() = delete;
		TreeIterator& operator=(const TreeIterator&) = delete;
	};
}