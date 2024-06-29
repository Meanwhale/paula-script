#pragma once
#include "array.h"
namespace paula
{
	// tags

	constexpr INT
		NODE_STRUCT				= 0x01000000,
		NODE_INTEGER			= 0x02000000,

		NODE_EXPR				= 0x03000000, // for parser
		NODE_PARENTHESIS		= 0x04000000,
		NODE_SQUARE_BRACKETS	= 0x05000000,
		NODE_CURLY_BRACKETS		= 0x06000000,
		NODE_TEXT				= 0x07000000,

		// masks
		TAG_MASK	= 0xff000000,
		SIZE_MASK	= 0x00ffffff;

	class Tree
	{
	public:
		Tree(INT size);

		void addInt(INT parentIndex, INT data);
		void addText(INT parentIndex, Array<BYTE>& src, INT first, INT last);
		INT addStruct(INT parentIndex);

		bool isStruct(INT nodeIndex);
		int nodeTag(INT node);
		int nodeSize(INT node);
		void clear();
		void print();
	private:
		Array<INT> data;
		INT top;
		INT node(INT tag, INT size);
		INT insertNode(INT parentIndex, INT tag, INT size);

		void printNode(INT index, INT depth);
	};
}