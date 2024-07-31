#include "tree.h"
#include "stream.h"
#include "args.h"

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
	ASSERT(isSubtree(parentIndex));
	INT previousLast = data[parentIndex + 4]; // save previous last child
	data[parentIndex + 4] = top; // set the new node as new last child
	
	if (data[parentIndex + 3] < 0)
	{
		ASSERT(previousLast < 0);
		data[parentIndex + 3] = top; // first child
	}
	else
	{
		ASSERT(previousLast > 0);
		data[previousLast + 2] = top; // set new node as previous last's next sibling
	}

	data[top++] = node(tag, size); // size = 3 (parent, next, data)
	data[top++] = parentIndex;
	data[top++] = -1; // no siblings yet
}

void paula::Tree::addOperatorNode(INT parentIndex, CHAR op)
{
	insertTree(parentIndex, NODE_OPERATOR, 3);
	data[top++] = charToInt(op);
}

void paula::Tree::addDouble(INT parentIndex, double value)
{
	insertTree(parentIndex, NODE_DOUBLE, 4);

	LONG number = doubleToLongFormat(value);
	INT hi =  longHighBits(number);
	INT low = longLowBits(number);
	data[top++] = hi;
	data[top++] = low;
}

void paula::Tree::addInt(INT parentIndex, INT value)
{
	insertTree(parentIndex, NODE_INTEGER, 3);
	data[top++] = value;
}

void paula::Tree::addBool(INT parentIndex, bool value)
{
	insertTree(parentIndex, NODE_BOOL, 3);
	data[top++] = value ? 1 : 0;
}

void Tree::addText(INT parentIndex, const char * text)
{
	INT numBytes = (INT)strlen(text);
	addText(parentIndex, (const unsigned char *) text, 0, numBytes, NODE_NAME);
}


void Tree::addText(INT parentIndex, const unsigned char * bytes, INT firstByte, INT lastByte, INT nodeType)
{
	// TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

	INT numBytes = lastByte - firstByte;
	INT intsSize = textDataSize(numBytes);

	insertTree(parentIndex, nodeType, 3 + intsSize);
	
	data[top++] = numBytes;

	bytesToInts(bytes, firstByte, data, top, numBytes);

	top += intsSize;
}

INT paula::Tree::addSubtree(INT parentIndex, INT type)
{
	ASSERT(isSubtreeTag(type));
	INT newSubtreeIndex = top;
	insertTree(parentIndex, type, 4);
	data[top++] = -1; // first child
	data[top++] = -1; // last child
	return newSubtreeIndex;
}

bool Tree::hasCapacity(INT size)
{
	return top + size < data.length();
}

// ------------- STACK BEGIN

// STACK NODE: TAG (node type, size), PARENT, NEXT, FIRST CHILD, -1 [not used but there to be same size as subtree node]

void paula::Tree::pushStack(INT stackIndex, INT tag, INT size)
{
	ASSERT(isStack(stackIndex));
	INT previousFirst = data[stackIndex + 3]; // can be -1 if first
	data[stackIndex + 3] = top; // new first

	data[top++] = node(tag, size); // node data
	data[top++] = stackIndex;
	data[top++] = previousFirst; // set next stack item
}

void Tree::pushInt(INT stackIndex, INT value)
{
	ASSERT(isStack(stackIndex));
	pushStack(stackIndex, NODE_INTEGER, 3);
	data[top++] = value;
}

void Tree::pushBool(INT stackIndex, bool value)
{
	ASSERT(isStack(stackIndex));
	pushStack(stackIndex, NODE_BOOL, 3);
	data[top++] = value ? 1 : 0;
}

void Tree::addData(INT parentIndex, TreeIterator& src)
{
	ASSERT(isSubtree(parentIndex));
	INT type = src.type();
	INT size = src.size();
	insertTree(parentIndex, type, size);
	// copy actual data
	for(INT i=3; i<=size; i++)
	{
		VRB(LOG.print("copy value: ").print(src.tree.data[src.index + i]).endl();)
		data[top++] = src.tree.data[src.index + i];
	}
}
void Tree::pushData(INT stackIndex, INT* src)
{
	// NOTE: almost same as below

	ASSERT(isStack(stackIndex));
	INT type = (*src) & TAG_MASK;
	INT size = (*src) & SIZE_MASK;
	pushStack(stackIndex, type, size);
	// copy actual data
	for(INT i=3; i<=size; i++)
	{
		VRB(LOG.print("copy RAW value: ").print(*(src + i)).endl();)
		data[top++] = *(src + i);
	}
}
void Tree::pushData(INT stackIndex, TreeIterator& src)
{
	ASSERT(isStack(stackIndex));
	INT type = src.type();
	INT size = src.size();
	pushStack(stackIndex, type, size);
	// copy actual data
	for(INT i=3; i<=size; i++)
	{
		VRB(LOG.print("copy value: ").print(src.tree.data[src.index + i]).endl();)
		data[top++] = src.tree.data[src.index + i];
	}
}

INT Tree::stackTopIndex(INT stackIndex)
{
	ASSERT(isStack(stackIndex));
	return data[stackIndex + 3];
}

INT Tree::popInt(INT stackIndex)
{
	INT stackTop = stackTopIndex(stackIndex);
	ASSERT(getType(stackTop) == NODE_INTEGER);
	INT value = data[stackTop + 3];
	pop(stackIndex);
	return value;
}

void Tree::pop(INT stackIndex)
{
	// TODO: check if this was the "top" item. if yes, descrease top to save space.

	ASSERT(isStack(stackIndex));
	INT itemIndex = data[stackIndex + 3]; // first child
	ASSERT(itemIndex > 0); // something to pop
	data[stackIndex + 3] = data[itemIndex + 2]; // top node's next is now on the top
}

bool Tree::stackEmpty(INT stackIndex)
{
	ASSERT(isStack(stackIndex));
	return data[stackIndex + 3] < 0;
}
INT Tree::stackSize(INT stackIndex)
{
	ASSERT(isStack(stackIndex));
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
// 
// ------------- MAP BEGIN

bool Tree::getBool(bool& out, const char* varName)
{
	INT index = getIndexOfData(varName, NODE_BOOL);
	if (index < 0) return false;
	return readBool(out, data.ptr(index));
}
bool Tree::getDouble(double& out, const char* varName)
{
	INT index = getIndexOfData(varName, NODE_DOUBLE);
	if (index < 0) return false;
	return readDouble(out, data.ptr(index));
}
bool Tree::getInt(int& out, const char* varName)
{
	INT index = getIndexOfData(varName, NODE_INTEGER);
	if (index < 0) return false;
	return readInt(out, data.ptr(index));
}
bool Tree::getChars(char*&out, const char* varName)
{
	INT index = getIndexOfData(varName, NODE_TEXT);
	if (index < 0) return false;
	return readChars(out, data.ptr(index));
}

INT Tree::getIndexOfData(const char* varName, INT dataType)
{
	ASSERT(getType(0) == NODE_SUBTREE);
	INT length = (INT)strlen(varName);
	TreeIterator it(*this);
	if (!it.hasChild()) return -1;
	it.toChild();

	int tmp[MAX_VAR_NAME_DATA_LENGTH]; // avoid "new"
	Array<INT>varNameData(tmp, MAX_VAR_NAME_DATA_LENGTH);
	varNameData[0] = length;
	bytesToInts((const unsigned char*)varName, 0, varNameData, 1, length);

	do
	{
		it.toChild(); // first child is the name
		if (it.matchTextData(varNameData.ptr()))
		{
			it.next(); // found! move forward to data
			if (getType(it.index) != dataType) return -1;
			return it.index;
		}
		it.toParent();
	}
	while(it.next());
	return -1; // not found
}

// ------------- MAP END


INT Tree::getType(INT index)
{
	return data[index] & TAG_MASK;
}
INT Tree::getNodeSize(INT index)
{
	return data[index] & 0x00ffffff;
}
bool paula::Tree::isSubtree(INT nodeIndex)
{
	return isSubtreeTag(getType(nodeIndex));
}

bool Tree::isStack(INT nodeIndex)
{
	return getType(nodeIndex) == NODE_STACK;
}

bool paula::Tree::isSubtreeTag(INT tag)
{
	return (tag & 0xf0ffffff) == 0;
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
		LOG.println("TREE is CLEAR");
		return;
	}
	for(int n=0; n<top; n++)
	{
		LOG.print(n);
		LOG.print(": ");
		LOG.printHex(data[n]);
		LOG.print("    ");
		LOG.print(data[n]);
		LOG.print("\n");
	}
}

void paula::Tree::print()
{
	if (isClear())
	{
		LOG.println("TREE is CLEAR");
		return;
	}
	TreeIterator it(*this);
	LOG.print(it).endl();
	printSubtree(it);
	//printNode(0, 0);

	//printData();
}
void Tree::printSubtree(TreeIterator& it)
{
	if (!it.hasChild()) return;
	it.toChild();
	do
	{	// print index
		LOG.print(it.index).print(": ");
		for (int n=0; n<it.getDepth(); n++) LOG.print("  ");
		LOG.print(it).endl();
		if (it.hasChild()) printSubtree(it);
	} while(it.next());
	it.toParent();
}
void Tree::printCompact(TreeIterator&it)
{
	if (!it.hasChild()) return;
	it.toChild();
	LOG.print("(");
	do
	{
		LOG.print(it).endl();
		if (it.hasChild()) printCompact(it);
		if (it.hasNext()) LOG.print(" ");
	} while(it.next());
	LOG.print(")");
	it.toParent();
}

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
	//ASSERT(!tree.isClear());
}
TreeIterator::TreeIterator(Tree& _tree, INT _index) :
	tree(_tree),
	index(_index),
	depth(0)
{
	ASSERT(!tree.isClear());
}

Var TreeIterator::var() const
{
	return Var(tree.data.ptr(index));
}

void TreeIterator::printTree(bool compact)
{
	if (compact) tree.printCompact(*this);
	else tree.printSubtree(*this);
}

bool TreeIterator::next()
{
	if (!hasNext()) return false;
	index = tree.data[index+2];
	return true;
}

void TreeIterator::toChild()
{
	ASSERT(hasChild());
	index = tree.data[index+3];
	depth++;
}

void TreeIterator::toParent()
{
	ASSERT(hasParent());
	index = tree.data[index+1];
	depth--;
}

INT TreeIterator::getDepth()
{
	return depth;
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
bool TreeIterator::isNextType(INT nodeType)
{
	if (!hasNext()) return NODE_UNDEFINED;
	INT nextIndex = tree.data[index+2];
	return tree.getType(nextIndex) == nodeType;
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
	return tree.getNodeSize(index);
}

void TreeIterator::overwrite(TreeIterator& src)
{
	ASSERT(type() == src.type() && size() == src.size());

	// copy actual data
	for(INT i=3; i<=size(); i++)
	{
		VRB(LOG.print("overwrite value: ").print(src.tree.data[src.index + i]).endl();)
		tree.data[index + i] = src.tree.data[src.index + i];
	}
}
CHAR TreeIterator::getOp()
{
	ASSERT(isType(NODE_OPERATOR));
	return (CHAR)(tree.data[index + 3]);
}

bool TreeIterator::getBool()
{
	ASSERT(isType(NODE_BOOL));
	return tree.data[index + 3] != 0;
}
INT TreeIterator::getInt()
{
	INT out;
	bool found = readInt(out, tree.data.ptr(index));
	ASSERT(found);
	return out;
//	ASSERT(isType(NODE_INTEGER));
//	return tree.data[index + 3];
}

const char* TreeIterator::getText()
{
	ASSERT(isTextType());
	LOG.print("TEXT [").print(tree.data[index+3]).print("] ");
	auto ptr = (tree.data.ptr() + index + 4);
	return (char *)ptr;
}

bool TreeIterator::matchTextData(INT* data)
{
	// TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

	ASSERT(isTextType());

	//LOG.println("match: "<<getText());

	int size = tree.getNodeSize(index);
	
	// compare, starting from char count

	for (INT i=3; i<size; i++)
	{
		//LOG.println("matchTextData: compare "<<data[i-3]<<" vs "<<tree.data[index+i]);
		if (data[i-3] != tree.data[index+i]) return false;
	}
	return true;
}


INT* TreeIterator::getTextData()
{
	ASSERT(isTextType());
	return tree.data.ptr() + index + 3;
}

}