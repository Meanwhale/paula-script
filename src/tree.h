#pragma once
#include "paula.h"
#include "array.h"
namespace paula
{
	// tags

	const int
		NODE_STRUCT  = 0x01000000,
		NODE_INTEGER = 0x02000000,

		// masks
		TAG_MASK	= 0xff000000,
		SIZE_MASK	= 0x00ffffff;

	class Tree
	{
	public:
		Tree(INT size);
		
		void addInt(INT parentIndex, INT data);
		INT addStruct(INT parentIndex);

		bool isStruct(INT nodeIndex);
		int nodeTag(INT node);
		int nodeSize(INT node);
		void print();
	private:
		Array<INT> data;
		INT top;
		INT node(INT tag, INT size);
		INT insertNode(INT parentIndex, INT tag, INT size);

		void printNode(INT index, INT depth);
	};
}