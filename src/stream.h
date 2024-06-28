#pragma once
#include "defs.h"
#include "array.h"
namespace paula
{
	class IInputStream
	{
	public:
		virtual CHAR read() = 0;
		virtual bool end() = 0;
		virtual void close() = 0;
	};

	class CharInputStream : public IInputStream
	{
	public:
		CharInputStream(const char *);
		CHAR read() override;
		bool end() override;
		void close() override;
	private:
		const char * str;
		INT i;
	};
	class BufferInputStream : public IInputStream
	{
	public:
		BufferInputStream(Array<CHAR>&, INT start, INT last);
		CHAR read() override;
		bool end() override;
		void close() override;
	private:
		Array<CHAR> str;
		INT i, last;
	};
}