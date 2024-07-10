#include "tree.h"

namespace paula
{
// layout:
//		SUBTREE NODE:	TAG (node type, size), PARENT, NEXT, FIRST CHILD, LAST CHILD
//		DATA NODE:		TAG (node type, size), PARENT, NEXT, DATA [...]
// size = number of ints after the tag


paula::Tree::Tree(INT size) : data(size)
{
	clear();
}

INT paula::Tree::node(INT tag, INT size)
{
	return tag | size;
}

void paula::Tree::insertTree(INT parentIndex, INT tag, INT size)
{
	ASSERT(isSubtree(parentIndex), "not a subtree");
	INT previousLast = data[parentIndex + 4]; // save previous last child
	data[parentIndex + 4] = top; // set the new node as new last child
	
	if (data[parentIndex + 3] < 0)
	{
		ASSERT(previousLast < 0, "");
		data[parentIndex + 3] = top; // first child
	}
	else
	{
		ASSERT(previousLast > 0, "");
		data[previousLast + 2] = top; // set new node as previous last's next sibling
	}

	data[top++] = node(tag, size); // size = 3 (parent, next, data)
	data[top++] = parentIndex;
	data[top++] = -1; // no siblings yet
}

void paula::Tree::addInt(INT parentIndex, INT value)
{
	insertTree(parentIndex, NODE_INTEGER, 3);
	data[top++] = value;
}


void Tree::addText(INT parentIndex, Array<BYTE>& src, INT firstByte, INT lastByte, INT nodeType)
{
	// TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

	INT numBytes = lastByte - firstByte;
	INT intsSize = textDataSize(numBytes);

	insertTree(parentIndex, nodeType, 3 + intsSize);
	
	data[top++] = numBytes;

	bytesToInts(src.get(), firstByte, data, top, numBytes);

	top += intsSize;
}

INT paula::Tree::addSubtree(INT parentIndex, INT type)
{
	ASSERT(isSubtreeTag(type), "");
	INT newSubtreeIndex = top;
	insertTree(parentIndex, type, 4);
	data[top++] = -1; // first child
	data[top++] = -1; // last child
	return newSubtreeIndex;
}

// ------------- STACK BEGIN

// STACK NODE: TAG (node type, size), PARENT, NEXT, FIRST CHILD, -1 [not used but there to be same size as subtree node]

void paula::Tree::pushStack(INT stackIndex, INT tag, INT size)
{
	ASSERT(isStack(stackIndex),"");
	INT previousFirst = data[stackIndex + 3]; // can be -1 if first
	data[stackIndex + 3] = top; // new first

	data[top++] = node(tag, size); // node data
	data[top++] = stackIndex;
	data[top++] = previousFirst; // set next stack item
}

void Tree::pushInt(INT stackIndex, INT value)
{
	ASSERT(isStack(stackIndex),"");
	pushStack(stackIndex, NODE_INTEGER, 3);
	data[top++] = value;
}

void Tree::pushData(INT stackIndex, TreeIterator& src)
{
	ASSERT(isStack(stackIndex),"");
	INT type = src.type();
	INT size = src.size();
	pushStack(stackIndex, type, size);
	// copy actual data
	for(INT i=3; i<=size; i++)
	{
		LOGLINE("copy value: "<<src.tree.data[src.index + i]);
		data[top++] = src.tree.data[src.index + i];
	}
}

INT Tree::stackTopIndex(INT stackIndex)
{
	ASSERT(isStack(stackIndex),"");
	return data[stackIndex + 3];
}

void Tree::pop(INT stackIndex)
{
	// TODO: check if this was the "top" item. if yes, descrease top to save space.

	ASSERT(isStack(stackIndex),"");
	INT itemIndex = data[stackIndex + 3]; // first child
	ASSERT(itemIndex > 0, ""); // something to pop
	data[stackIndex + 3] = data[itemIndex + 2]; // top node's next is now on the top
}

bool Tree::stackEmpty(INT stackIndex)
{
	ASSERT(isStack(stackIndex),"");
	return data[stackIndex + 3] < 0;
}
INT Tree::stackSize(INT stackIndex)
{
	ASSERT(isStack(stackIndex),"");
	INT n = 0;
	INT stackItemIndex = data[stackIndex + 3]; // first child
	while (stackItemIndex > 0)
	{
		n++;
		stackItemIndex = data[stackItemIndex + 2];
	}
	return n;
}

// ------------- STACK END

INT Tree::get(INT index)
{
	return data[index];
}
INT Tree::getType(INT index)
{
	return maskNodeTag(data[index]);
}
INT Tree::getNodeSize(INT index)
{
	return data[index] & 0x00ffffff;
}
bool paula::Tree::isSubtree(INT nodeIndex)
{
	return isSubtreeTag(maskNodeTag(data[nodeIndex]));
}
bool paula::Tree::isStack(INT nodeIndex)
{
	return maskNodeTag(data[nodeIndex]) == NODE_STACK;
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
	data[3] = -1; // no first child
	data[4] = -1; // no last child
	top = 5;
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
	TreeIterator it(*this);
	it.print(false);
	LOGLINE("");
	printSubtree(it);
	//printNode(0, 0);

	//printData();
}
void Tree::printSubtree(TreeIterator& it)
{
	if (!it.hasChild()) return;
	it.toChild();

	do
	{
		for (int n=0; n<it.getDepth(); n++) LOG("  ");
		it.print(false);
		LOGLINE("");

		if (it.hasChild())
		{
			printSubtree(it);
		}

	} while(it.next());

	it.toParent();
}
void Tree::printCompact(TreeIterator&it)
{
	if (!it.hasChild()) return;
	it.toChild();
	LOG("(");
	do
	{
		it.print(true);
		if (it.hasChild()) printCompact(it);
		if (it.hasNext()) LOG(" ");
	} while(it.next());
	LOG(")");
	it.toParent();
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
/*void paula::Tree::printNode(INT index, INT depth)
{
	// tarviiko en��?

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
}*/


//--------------------------------------------------------------
// TreeIterator
//--------------------------------------------------------------

TreeIterator::TreeIterator(const TreeIterator& it) :
	tree(it.tree),
	index(it.index),
	depth(it.depth)
{
	// copy constructor
}

TreeIterator::TreeIterator(Tree& _tree) :
	tree(_tree),
	index(0),
	depth(0)
{
	CHECK(!tree.isClear(), TREE_IS_EMPTY);
}

void TreeIterator::print(bool compact)
{
	if (isType(NODE_TEXT) || isType(NODE_NAME))
	{
		LOG(getText());
	}
	else if (isType(NODE_INTEGER))
	{
		LOG(getInt());
	}
	else if (tree.isSubtree(index))
	{
		if (!compact) LOG(tree.treeTypeName(tree.getType(index)));
	}
	else if (tree.isStack(index))
	{
		if (!compact) LOG("<stack>");
	}
	else
	{
		LOG("<! ! ! TreeIterator::print: unknown node ! ! !>");
	}
}

bool TreeIterator::next()
{
	if (!hasNext()) return false;
	index = tree.data[index+2];
	return true;
}

void TreeIterator::toChild()
{
	CHECK(hasChild(), ITERATOR_HAS_NO_CHILD_ELEMENT);
	index = tree.data[index+3];
	depth++;
}

void TreeIterator::toParent()
{
	CHECK(hasParent(), ITERATOR_HAS_NO_PARENT_ELEMENT);
	index = tree.data[index+1];
	depth--;
}

bool TreeIterator::hasNext()
{
	return tree.data[index+2] > 0;
}

bool TreeIterator::hasChild()
{
	if (tree.isSubtree(index) || tree.isStack(index))
	{
		INT childIndex = tree.data[index+3];
		return childIndex > 0;
	}
	return false;
}

bool TreeIterator::hasParent()
{
	return tree.data[index+1] >= 0; // TODO: or index != 0 as only root has no parent...?
}

bool TreeIterator::isType(INT nodeType)
{
	return tree.getType(index) == nodeType;
}

bool TreeIterator::isTextType()
{
	return isType(NODE_TEXT) || isType(NODE_NAME);
}

INT TreeIterator::type()
{
	return tree.getType(index);
}

INT TreeIterator::size()
{
	return tree.getNodeSize(index);;
}

INT TreeIterator::getDepth()
{
	return depth;
}

INT TreeIterator::getInt()
{
	CHECK(isType(NODE_INTEGER), ITERATOR_WRONG_DATA_TYPE);
	return tree.data[index + 3];
}

const char* TreeIterator::getText()
{
	CHECK(isTextType(), ITERATOR_WRONG_DATA_TYPE);
	LOG("TEXT ["<<(tree.data[index+3])<<"] ");
	auto ptr = (tree.data.get() + index + 4);
	return (char *)ptr;
}

bool TreeIterator::matchTextData(INT* data)
{
	// TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

	CHECK(isTextType(), ITERATOR_WRONG_DATA_TYPE);

	//LOGLINE("match: "<<getText());

	int size = tree.getNodeSize(index);
	
	// compare, starting from char count

	for (INT i=3; i<size; i++)
	{
		//LOGLINE("matchTextData: compare "<<data[i-3]<<" vs "<<tree.data[index+i]);
		if (data[i-3] != tree.data[index+i]) return false;
	}
	return true;
}

}