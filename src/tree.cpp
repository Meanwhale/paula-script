#include "tree.h"

namespace paula
{
// layout:
//		STRUCT NODE:	TAG (node type, size), PARENT, NEXT, CHILD
//		DATA NODE:		TAG (node type, size), PARENT, NEXT, DATA
// size = number of ints after the tag


paula::Tree::Tree(INT size) : data(size)
{
	clear();
}

INT paula::Tree::node(INT tag, INT size)
{
	return tag | size;
}

INT paula::Tree::insertNode(INT parentIndex, INT tag, INT size)
{
	ASSERT(isSubtree(parentIndex), "not a subtree");
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


void Tree::addText(INT parentIndex, Array<BYTE>& src, INT firstByte, INT lastByte)
{
	// Meanscriptin versio miss‰ eka int on merkkien lukum‰‰r‰
	//		data = new IntArray((length / 4) + 2);
	//		data[0] = length;
	//		MS.BytesToInts(src, start, data.Data(), 1, length);


	//		TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

	INT numBytes = lastByte - firstByte;
	INT textDataSize = (numBytes / 4) + 2;

	insertNode(parentIndex, NODE_TEXT, 3 + textDataSize);
	
	data[top++] = numBytes;

	bytesToInts(src.get(), firstByte, data, top, numBytes);

	top += textDataSize;
}

INT paula::Tree::addSubtree(INT parentIndex, INT type)
{
	ASSERT(isSubtreeTag(type), "");
	INT newSubtreeIndex = insertNode(parentIndex, type, 3);
	data[top++] = -1;
	return newSubtreeIndex;
}

INT Tree::get(INT index)
{
	return data[index];
}
INT Tree::getTag(INT index)
{
	return maskNodeTag(data[index]);
}

bool paula::Tree::isSubtree(INT nodeIndex)
{
	return isSubtreeTag(maskNodeTag(data[nodeIndex]));
}

bool paula::Tree::isSubtreeTag(INT tag)
{
	return (tag & 0xf0ffffff) == 0;
}

int paula::Tree::maskNodeTag(INT node)
{
	return node & TAG_MASK;
}

int paula::Tree::nodeSize(INT node)
{
	return node & SIZE_MASK;
}

void Tree::init(INT parentType)
{
	// add root node
	data[0] = node(parentType, 1);
	data[1] = -1; // no parent
	data[2] = -1; // no sibling
	data[3] = -1; // no child
	top = 4;
}

void Tree::clear()
{
	data[0] = 0;
	top = -1;
}

bool Tree::isClear()
{
	return top < 0;
}

void paula::Tree::printData()
{
	if (isClear())
	{
		LOGLINE("TREE is CLEAR");
		return;
	}
	for(int n=0; n<top; n++)
	{
		LOG(n);
		LOG(": ");
		LOG(data[n]);
		LOG("\n");
	}
}

void paula::Tree::print()
{
	if (isClear())
	{
		LOGLINE("TREE is CLEAR");
		return;
	}

	printNode(0, 0);
}
const char* Tree::treeTypeName(INT tag)
{
	switch(tag)
	{
	case 0x01000000: return "<subtree>";
	case 0x02000000: return "<expr>";
	case 0x03000000: return "<assignment>";
	case 0x04000000: return "<command>";
	}
	return "<! ! ! error ! ! !>";
}
void paula::Tree::printNode(INT index, INT depth)
{
	if (data[index+2] > 0) printNode(data[index+2], depth);
	for (int n=0; n<depth; n++) LOG("  ");

	// LOG("parent="<<(data[index+1])<<" next="<<(data[index+2])<<" data size="<<nodeSize(data[index]));
	LOG("NODE");

	if (maskNodeTag(data[index]) == NODE_TEXT)
	{
		LOG(" TEXT: ["<<(data[index+3])<<"] "); // print number of characters
		auto ptr = (data.get() + index + 4);
		auto chptr = (char *)ptr;
		LOG(chptr);
		LOGLINE("");
	}
	else
	{
		if (isSubtreeTag(maskNodeTag(data[index])))
		{
			LOG(treeTypeName(maskNodeTag(data[index])));
		}
		LOGLINE("");
		//LOGLINE(" data: "<<(data[index+3]));
	}

	if (isSubtree(index))
	{
		INT childIndex = data[index+3];
		if (childIndex > 0) printNode(childIndex, depth+1);
	}
}

}