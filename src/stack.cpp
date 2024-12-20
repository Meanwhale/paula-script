#include "stack.h"
#include "tree.h"
#include "args.h"
#include "node_types.h"

using namespace paula;
using namespace paula::core;

// ------------- STACK BEGIN

/*
STACK STRUCTURE:
Items have tag (type and size) and links to previous (higher index) and next (lower index).
'top' points to the tag of last added item or root if the stack is empty.
REASON: similar to Tree data structure.

		index		item

		i_0			ROOT	[previous=itemA, next=-1]
		i_1			itemA	[previous=itemB, next=ROOT]
top		i_2			itemB	[previous=-1,	 next=itemA]

Memory layout (" - - " is data)

		ROOT - - -  item A tag - - - - - - - - item B tag - - - - - -| free space
		^                                      ^
index:	0                                      top

DATA NODE:
TAG (node type, size)
"PARENT" = next pushed
"NEXT" = last pushed, it.next() goes there, smaller index
DATA [...]
START:
0:					1:					2:
NODE_STACK_ROOT		PREVIOUS = -1		NEXT = -1																			top=0
PUSH INT:
0:					1:					2:					3:				4:				5:				6:
NODE_STACK_ROOT		PREVIOUS = 3		NEXT = -1			NODE_INT. s=3	PREVIOUS=-1		NEXT=0=root		value			top=3
*/

Stack::Stack(INT size) : data(size)
{
	clear();
}
void Stack::clear()
{
	count = 0;
	top = 0;
	data[0] = NODE_STACK_ROOT | 2; // size = 2, previous and next, for consistency
	data[1] = -1; // previous
	data[2] = -1; // next
}
void core::Stack::addNode(INT tag, INT size)
{
	INT oldTop = top; // next (for new item) points to old top
	ASSERT(data[top + 1] < 0); // check that "previous" (up the stack) hasn't been set
	top += (data[top] & SIZE_MASK) + 1; // move 'top' to stack top and add item there
	data[top] = tag | size; // node data, size + 1 for tag
	data[top+1] = -1; // no "previous" yet
	data[top+2] = oldTop; // old top

	data[oldTop + 1] = top; // previous for old top
	count++;
}
INT Stack::previous(INT index)
{
	return data[index + 1];
}
INT Stack::next(INT index)
{
	return data[index + 2];
}
void Stack::pop()
{
	VRB(LOG.print("-------- pop").endl();printData(););
	ASSERT((data[top] & NODE_TYPE_MASK) != NODE_STACK_ROOT);
	ASSERT(previous(top) < 0); // check there's not previous
	ASSERT(next(top) >= 0); // check there's next

	// decrease top index
	top = next(top);
	data[top+1] = -1; // no "previous" yet
	count--;
}
void Stack::pushInt(INT value)
{
	addNode(NODE_INTEGER, 3);
	data[top + 3] = value;
	VRB(LOG.print("-------- push int").endl();printData(););
}
void Stack::pushBool(bool value)
{
	addNode(NODE_BOOL, 3);
	data[top + 3] = value ? 1 : 0;
}
void Stack::pushData(const INT* src)
{
	INT type = (*src) & NODE_TYPE_MASK;
	INT size = (*src) & SIZE_MASK;

	addNode(type, size);
	for (INT i = 3; i <= size; i++) // i=3 skip tag and links (see tree)
	{
		data[top+i] = *(src + i);
	}
}
void Stack::pushText(const char* text)
{
	const unsigned char* bytes = (const unsigned char*)text;
	INT numBytes = (INT)strlen(text);
	INT intsSize = textDataSize(numBytes);

	addNode(NODE_TEXT, 3 + intsSize);

	data[top+3] = numBytes;

	bytesToInts(bytes, 0, data, top+4, numBytes);
}
void Stack::pushData(TreeIterator& src)
{
	auto ptr = src.tree.data.ptr(src.index);
	pushData(ptr);
}
INT Stack::popInt()
{
	ASSERT((data[top] & NODE_TYPE_MASK) == NODE_INTEGER);
	INT value = data[top + 3];
	pop();
	return value;
}


bool Stack::stackEmpty()
{
	return top == 0;
}
INT Stack::itemCount()
{
	return count;
}

INT* paula::core::Stack::topPtr()
{
	return data.ptr(top);
}

Var paula::core::Stack::topVar()
{
	return Var(data.ptr(top));
}

void paula::core::Stack::printData()
{
	INT i=0;
	INT topSize = data[top] & SIZE_MASK;
	for(; i<=top + topSize; i++)
	{
		LOG.print(i).print(": ").print(data[i]).endl();
	}
}

void paula::core::Stack::printValues()
{
	StackIterator argIt(*this);
	LOG.print("args stack");
	do
	{
		LOG.print("\n - ").print(argIt.var());
	}
	while(argIt.next());
	LOG.endl();
}

StackIterator::StackIterator(Stack&_stack) : stack(_stack), ptr(_stack.topPtr())
{
}
paula::core::StackIterator::StackIterator(Stack&_stack, INT*base) : stack(_stack), ptr(base)
{
}
paula::core::StackIterator::StackIterator(StackIterator&src) : stack(src.stack), ptr(src.ptr)
{
}
Var StackIterator::var()
{
	return Var(ptr);
}
bool StackIterator::next()
{
	// move pointer to next. if next is root, return false.
	INT nextIndex = *(ptr + 2);
	ptr = stack.data.ptr(nextIndex);
	return type() != NODE_STACK_ROOT;
}

INT paula::core::StackIterator::type()
{
	return (*ptr) & NODE_TYPE_MASK;
}
