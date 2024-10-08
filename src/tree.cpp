#include "tree.h"
#include "stream.h"
#include "args.h"
#include <cstring> 

namespace paula {
	namespace core
	{

		// layout:
		//		SUBTREE NODE:	TAG (node type, size), PARENT, NEXT, FIRST CHILD, LAST CHILD
		//		DATA NODE:		TAG (node type, size), PARENT, NEXT, DATA [...]
		// size = number of ints after the tag

		core::Tree::Tree(INT size) : data(size)
		{
			clear();
		}

		core::Tree::Tree(INT* a, int s) : data(a, s)
		{
		}

		INT core::Tree::node(INT tag, INT size)
		{
			return tag | size;
		}

		void core::Tree::insertToTree(INT parentIndex, INT tag, INT size)
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

			// size: num. items after the tag, includes parent and sibling indexes and the data.
			// eg. size = 3 (parent, next, data)

			data[top++] = node(tag, size);
			data[top++] = parentIndex;
			data[top++] = -1; // no siblings yet
		}
		void Tree::addData(INT parentIndex, Var src)
		{
			ASSERT(isSubtree(parentIndex));
			INT type = src.type();
			INT size = src.size();
			insertToTree(parentIndex, type, size);
			// copy actual data
			for (INT i = 3; i <= size; i++)
			{
				VRB(LOG.print("copy value: ").print(src.ptr[i]).endl();)
					data[top++] = src.ptr[i];
			}
		}
		void Tree::addData(INT parentIndex, TreeIterator& src)
		{
			addData(parentIndex, src.var());
		}

		void core::Tree::addOperatorNode(INT parentIndex, CHAR op)
		{
			insertToTree(parentIndex, NODE_OPERATOR, 3);
			data[top++] = charToInt(op);
		}

		void core::Tree::addDouble(INT parentIndex, double value)
		{
			insertToTree(parentIndex, NODE_DOUBLE, 4);

			LONG number = doubleToLongFormat(value);
			INT hi = longHighBits(number);
			INT low = longLowBits(number);
			data[top++] = hi;
			data[top++] = low;
		}

		void core::Tree::addRawTree(INT parentIndex, Tree& tree)
		{
			VRB(LOG.print("addRawTree. size:").print(tree.top).endl();)
				INT treeSize = tree.top;
			insertToTree(parentIndex, NODE_RAW_TREE, treeSize + 2);

			// copy tree data

			for (INT i = 0; i < treeSize; i++) data[top++] = tree.data[i];
		}

		void core::Tree::addInt(INT parentIndex, INT value)
		{
			insertToTree(parentIndex, NODE_INTEGER, 3);
			data[top++] = value;
		}

		void core::Tree::addBool(INT parentIndex, bool value)
		{
			insertToTree(parentIndex, NODE_BOOL, 3);
			data[top++] = value ? 1 : 0;
		}

		void Tree::addText(INT parentIndex, const char* text)
		{
			INT numBytes = (INT)strlen(text);
			addText(parentIndex, (const unsigned char*)text, 0, numBytes, NODE_NAME);
		}


		void Tree::addText(INT parentIndex, const unsigned char* bytes, INT firstByte, INT lastByte, INT nodeType)
		{
			// TEXT NODE:		TAG (node type, size), PARENT, NEXT, CHAR COUNT, TEXT DATA[n]

			INT numBytes = lastByte - firstByte;
			INT intsSize = textDataSize(numBytes);

			insertToTree(parentIndex, nodeType, 3 + intsSize);

			data[top++] = numBytes;

			bytesToInts(bytes, firstByte, data, top, numBytes);

			top += intsSize;
		}

		INT core::Tree::addSubtree(INT parentIndex, INT type)
		{
			ASSERT(isSubtreeTag(type));
			INT newSubtreeIndex = top;
			insertToTree(parentIndex, type, 4);
			data[top++] = -1; // first child
			data[top++] = -1; // last child
			return newSubtreeIndex;
		}

		bool Tree::hasCapacity(INT size)
		{
			return top + size < data.length();
		}
		// 
		// ------------- MAP BEGIN

		Var Tree::get(const char* varName)
		{
			INT dataIndex = getIndexOfData(varName);
			if (dataIndex < 0) return Var();
			return Var(data.ptr(dataIndex));
		}
		Var Tree::getAt(INT dataIndex)
		{
			if (dataIndex < 0) return Var();
			return Var(data.ptr(dataIndex));
		}

		/*bool Tree::getBool(bool& out, const char* varName)
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
		}*/

		INT Tree::getIndexOfData(const char* varName)
		{
			// assume tree is a _map_ i.e. list of name-value pairs.
			// convert chars to Paula-style text data (varNameData)
			// and search for name. return its pair if the name is found.

			ASSERT(getType(0) == NODE_SUBTREE);
			TreeIterator it(*this);
			if (!it.hasChild()) return -1;
			it.toChild();

			INT length = (INT)strlen(varName);
			int tmp[MAX_VAR_NAME_DATA_LENGTH];
			Array<INT>varNameData(tmp, MAX_VAR_NAME_DATA_LENGTH);
			varNameData[0] = length;
			bytesToInts((const unsigned char*)varName, 0, varNameData, 1, length);

			do
			{
				it.toChild(); // first child is the name
				if (matchTextData(it.getTextData(), varNameData.ptr()))
				{
					it.next(); // found! move forward to data
					//if (getType(it.index) != dataType) return -1;
					return it.index;
				}
				it.toParent();
			} while (it.next());
			return -1; // not found
		}

		// ------------- MAP END


		INT Tree::getType(INT index)
		{
			return data[index] & NODE_TYPE_MASK;
		}
		INT Tree::getNodeSize(INT index)
		{
			return data[index] & 0x00ffffff;
		}
		bool core::Tree::isSubtree(INT nodeIndex)
		{
			return isSubtreeTag(getType(nodeIndex));
		}

		bool core::Tree::isSubtreeTag(INT tag)
		{
			return (tag & 0xf0ffffff) == 0;
		}

		int core::Tree::nodeSize(INT node)
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

		void core::Tree::printData()
		{
			if (isClear())
			{
				LOG.println("TREE is CLEAR");
				return;
			}
			for (int n = 0; n < top; n++)
			{
				LOG.print(n);
				LOG.print(": ");
				LOG.printHex(data[n]);
				LOG.print("    ");
				LOG.print(data[n]);
				LOG.print("\n");
			}
		}

		void core::Tree::print()
		{
			if (isClear())
			{
				LOG.println("TREE is CLEAR");
				return;
			}
			TreeIterator it(*this);
			LOG.print(it).endl();
			printSubtree(it);
		}
		void Tree::printSubtree(TreeIterator& it)
		{
			if (!it.hasChild()) return;
			it.toChild();
			do
			{	// print index
				LOG.print(it.index).print(": ");
				for (int n = 0; n < it.getDepth(); n++) LOG.print("  ");
				LOG.print(it).endl();
				if (it.hasChild()) printSubtree(it);
			} while (it.next());
			it.toParent();
		}
		void Tree::printCompact(TreeIterator& it)
		{
			if (!it.hasChild()) return;
			it.toChild();
			LOG.print("(");
			do
			{
				LOG.print(it).endl();
				if (it.hasChild()) printCompact(it);
				if (it.hasNext()) LOG.print(" ");
			} while (it.next());
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
			index = tree.data[index + 2];
			return true;
		}

		void TreeIterator::toChild()
		{
			ASSERT(hasChild());
			index = tree.data[index + 3];
			depth++;
		}

		void TreeIterator::toParent()
		{
			ASSERT(hasParent());
			index = tree.data[index + 1];
			depth--;
		}

		void TreeIterator::jumpTo(INT i)
		{
			index = i;
		}

		INT TreeIterator::getDepth()
		{
			return depth;
		}

		bool TreeIterator::hasNext()
		{
			return tree.data[index + 2] > 0;
		}

		bool TreeIterator::hasChild()
		{
			if (tree.isSubtree(index))
			{
				INT childIndex = tree.data[index + 3];
				return childIndex > 0;
			}
			return false;
		}

		bool TreeIterator::hasParent()
		{
			return tree.data[index + 1] >= 0; // TODO: or index != 0 as only root has no parent...?
		}

		bool TreeIterator::isType(INT nodeType)
		{
			return tree.getType(index) == nodeType;
		}
		bool TreeIterator::isNextType(INT nodeType)
		{
			if (!hasNext()) return NODE_UNDEFINED;
			INT nextIndex = tree.data[index + 2];
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

		void TreeIterator::overwrite(Var src)
		{
			ASSERT(type() == src.type() && size() == src.size());

			// copy actual data
			for (INT i = 3; i <= size(); i++)
			{
				//VRB(LOG.print("overwrite value: ").print(src.tree.data[src.index + i]).endl();)
				tree.data[index + i] = src.ptr[i];
			}
		}


		INT* TreeIterator::getTextData()
		{
			ASSERT(isTextType());
			return tree.data.ptr() + index + 3;
		}
}
}