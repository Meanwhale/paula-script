#pragma once
#include "defs.h"
namespace paula
{
	// node types

	constexpr INT

		NODE_UNDEFINED			= 0xff000000,

		// tree types
		// subtree tag mask		= 0x0f000000  --> tree.cpp

		NODE_SUBTREE			= 0x01000000, // parser decides what kind or tree is it
		NODE_EXPR_TREE			= 0x02000000,
		NODE_STATEMENT_TREE		= 0x03000000,
		NODE_KV_TREE			= 0x07000000, // key-value pairs for map: text key that has a link to data

		// data types

		NODE_ANY_DATA			= 0x20000000,

		NODE_NAME				= 0x21000000, // variable, constant, or function name
		NODE_INTEGER			= 0x22000000, // 32-bit, e.g. 123
		NODE_DOUBLE				= 0x23000000, // 64-bit, e.g. 1.23
		NODE_TEXT				= 0x24000000, // string/const char*, e.g. "abc d"
		NODE_BOOL				= 0x25000000, // true = 1, false = 0

		NODE_STACK_ROOT			= 0x2e000000,
		NODE_VOID				= 0x2f000000,

		// controls tokens

		NODE_OPERATOR			= 0x31000000, // + - / *
		NODE_LOGICAL			= 0x32000000, // and or xor

		NODE_RAW_TREE			= 0x41000000, // raw tree data. copy to a new Tree object

		// masks
		NODE_TYPE_MASK			= 0xff000000,
		SIZE_MASK				= 0x00ffffff;
}