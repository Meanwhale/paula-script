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

void bytesToInts(Array<BYTE>& bytes, int bytesOffset, Array<INT>& ints, int intsOffset, int bytesLength)
{
	// order: 0x04030201

	// bytes:	b[3] b[2] b[1] b[0] b[7] b[6] b[5] b[4]...
	// ints:	_________i[0]______|_________i[1]______...

	int shift = 0;
	ints[intsOffset] = 0;
	for (int i = 0; i <= bytesLength;)
	{
		auto theByte = i == bytesLength ? '\0' : bytes[bytesOffset + i]; // add ending character to the end
		ints[(i / 4) + intsOffset] += (theByte & 0x000000FF) << shift;

		i++;
		if (i % 4 == 0)
		{
			shift = 0;
			if (i <= bytesLength)
			{
				ints[(i / 4) + intsOffset] = 0;
			}
		}
		else shift += 8;
	}
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

	bytesToInts(src, firstByte, data, top, numBytes);

	top += textDataSize;
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

void Tree::clear()
{
	// add root node
	data[0] = node(NODE_STRUCT, 1);
	data[1] = -1; // no parent
	data[2] = -1; // no sibling
	data[3] = -1; // no child
	top = 4;
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

	LOG("parent="<<(data[index+1])<<" next="<<(data[index+2])<<" data size="<<nodeSize(data[index]));

	if (nodeTag(data[index]) == NODE_TEXT)
	{
		LOG(" TEXT: ["<<(data[index+3])<<"] "); // print number of characters
		auto ptr = (data.get() + index + 4);
		auto chptr = (char *)ptr;
		LOG(chptr);
		LOGLINE("");
	}
	else
	{
		LOGLINE(" data: "<<(data[index+3]));
	}

	if (isStruct(index))
	{
		INT childIndex = data[index+3];
		if (childIndex > 0) printNode(childIndex, depth+1);
	}
}

}