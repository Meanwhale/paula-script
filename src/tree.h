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

		// data types

		NODE_NAME				= 0x11000000, // variable, constant, or function name
		NODE_INTEGER			= 0x12000000, // e.g. 123
		NODE_FLOAT				= 0x13000000, // e.g. 1.23
		NODE_TEXT				= 0x14000000, // e.g. "abc d"

		// operators

		NODE_PLUS				= 0x21000000,
		NODE_MINUS				= 0x22000000,
		NODE_MUL				= 0x23000000,
		NODE_DIV				= 0x24000000,

		//NODE_PARENTHESIS		= 0x00000000,
		//NODE_SQUARE_BRACKETS	= 0x00000000,
		//NODE_CURLY_BRACKETS	= 0x00000000,

		// masks
		TAG_MASK	= 0xff000000,
		SIZE_MASK	= 0x00ffffff;

	class Tree
	{
	public:
		Tree(INT size);

		void addInt(INT parentIndex, INT data);
		void addText(INT parentIndex, Array<BYTE>& src, INT first, INT last);
		INT addSubtree(INT parentIndex, INT type);

		INT get(INT index);
        INT getTag(INT index);
		bool isSubtree(INT nodeIndex);
		bool isSubtreeTag(INT tag);
		INT maskNodeTag(INT node);
		INT nodeSize(INT node);
		void init(INT parentType);
		void clear();
		bool isClear();
		void printData();
		void print();
		const char * treeTypeName(INT tag);
	private:
		Array<INT> data;
		INT top;
		INT node(INT tag, INT size);
		INT insertNode(INT parentIndex, INT tag, INT size);

		void printNode(INT index, INT depth);
	};
}