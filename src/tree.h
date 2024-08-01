#pragma once
#include "array.h"
namespace paula
{
	constexpr INT

		NODE_UNDEFINED			= 0xff000000,

		// tree types

		NODE_SUBTREE			= 0x01000000, // parser decides what kind or tree is it
		NODE_EXPR				= 0x02000000,
		NODE_STATEMENT			= 0x03000000,

		NODE_STACK				= 0x05000000,
		NODE_MAP				= 0x06000000, // like dictionary. list of KV nodes
		NODE_KV					= 0x07000000, // key-value pairs for map: text key that has a link to data

		// data types

		NODE_ANY_DATA			= 0x20000000,

		NODE_NAME				= 0x21000000, // variable, constant, or function name
		NODE_INTEGER			= 0x22000000, // 32-bit, e.g. 123
		NODE_DOUBLE				= 0x23000000, // 64-bit, e.g. 1.23
		NODE_TEXT				= 0x24000000, // string/const char*, e.g. "abc d"
		NODE_BOOL				= 0x25000000, // true = 1, false = 0

		NODE_VOID				= 0x2f000000,

		// controls tokens

		NODE_OPERATOR			= 0x31000000,

		//NODE_PARENTHESIS		= 0x00000000,
		//NODE_SQUARE_BRACKETS	= 0x00000000,
		//NODE_CURLY_BRACKETS	= 0x00000000,

		// masks
		TAG_MASK	= 0xff000000,
		SIZE_MASK	= 0x00ffffff;

	class TreeIterator;
	class Args;
	class Var;

	class Tree
	{
	public:
		Tree(INT size);

		void addInt(INT parentIndex, INT data);
		void addBool(INT parentIndex, bool data);
		void addOperatorNode(INT parentIndex, CHAR op);
		void addDouble(INT parentIndex, double value);
		void addText(INT parentIndex, const char* text);
		void addText(INT parentIndex, const unsigned char * bytes, INT first, INT last, INT nodeType);
		void addData(INT stackIndex, TreeIterator& src);
        void pushData(INT stackIndex, INT* src);
		INT addSubtree(INT parentIndex, INT type);
		bool hasCapacity(INT size);

		// stack functions

		void pushInt(INT stackIndex, INT value);
		void pushBool(INT stackIndex, bool value);
		void pushData(INT stackIndex, TreeIterator&src);
		INT stackTopIndex(INT stackIndex);
		INT popInt(INT stackIndex);
		void pop(INT stackIndex);
		bool stackEmpty(INT stackIndex);

		INT stackSize(INT stackIndex);

		// map functions

		bool getBool(bool& out, const char* varName);

        bool getDouble(double& out, const char* varName);

		bool getInt(int& out, const char* varName);

        bool getChars(char*& out, const char* varName);

		INT getIndexOfData(const char *, INT dataType);


		// accessors

        INT getType(INT index);
		INT getNodeSize(INT index);
		bool isSubtree(INT nodeIndex);
		bool isStack(INT nodeIndex);
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

		friend class TreeIterator;
		friend class Args;

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
		TreeIterator(Tree& _tree, INT _index);

		Var var() const;

		void printTree(bool compact);

		bool next(); // goes to next sibling. returns true if there's a sibling.

		void toChild();
		void toParent();

		bool hasNext();
		bool hasChild();
		bool hasParent();
		bool isType(INT nodeType);
		bool isNextType(INT nodeType);
		bool isTextType();
		INT type();
		INT size();

		void overwrite(TreeIterator& src);

		// data getters

		INT getDepth();


		//bool getBool();
		//INT getInt();
		//CHAR getOp();
		//const char * getText();

		bool matchTextData(INT*data);
		INT* getTextData();

		friend class Tree;
		friend class Args;
		friend class Paula;

	private:

		Tree& tree;
		INT index, depth;

		//TreeIterator() = delete;
		//TreeIterator& operator=(const TreeIterator&) = delete;
	};
}