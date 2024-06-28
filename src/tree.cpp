#include "tree.h"

namespace paula
{
// layout:
//		STRUCT NODE:	TAG (node type, size), PARENT, NEXT, CHILD
//		DATA NODE:		TAG (node type, size), PARENT, NEXT, DATA
// size = number of ints after the tag


paula::Tree::Tree(INT size) : data(size)
{
	// add root node
	data[0] = node(NODE_STRUCT, 1);
	data[1] = -1; // no parent
	data[2] = -1; // no sibling
	data[3] = -1; // no child
	top = 4;
}

INT paula::Tree::node(INT tag, INT size)
{
	return tag | size;
}

INT paula::Tree::insertNode(INT parentIndex, INT tag, INT size)
{
	ASSERT(isStruct(parentIndex), "not a struct");
	INT sibling = data[parentIndex + 3]; // save previous first child
	data[parentIndex + 3] = top; // set the new node as new first child

	INT nodeIndex = top;

	data[top++] = node(tag, size); // size = 3 (parent, next, data)
	data[top++] = parentIndex;
	data[top++] = sibling; // -1 if no siblings yet

	return nodeIndex;
}

void paula::Tree::addInt(INT parentIndex, INT value)
{
	insertNode(parentIndex, NODE_INTEGER, 3);
	data[top++] = value;
}

INT paula::Tree::addStruct(INT parentIndex)
{
	INT newStructIndex = insertNode(parentIndex, NODE_STRUCT, 3);
	data[top++] = -1;
	return newStructIndex;
}

bool paula::Tree::isStruct(INT nodeIndex)
{
	return nodeTag(data[nodeIndex]) == NODE_STRUCT;
}

int paula::Tree::nodeTag(INT node)
{
	return node & TAG_MASK;
}

int paula::Tree::nodeSize(INT node)
{
	return node & SIZE_MASK;
}

void paula::Tree::print()
{
	for(int n=0; n<top; n++)
	{
		LOG(n);
		LOG(": ");
		LOG(data[n]);
		LOG("\n");
	}

	printNode(0, 0);
}
void paula::Tree::printNode(INT index, INT depth)
{
	if (data[index+2] > 0) printNode(data[index+2], depth);
	for (int n=0; n<depth; n++) LOG("  ");

	LOGLINE("parent="<<(data[index+1])<<" next="<<(data[index+2])<<" data size="<<nodeSize(data[index])<<" data: "<<(data[index+3]));

	if (isStruct(index))
	{
		INT childIndex = data[index+3];
		if (childIndex > 0) printNode(childIndex, depth+1);
	}
}

}