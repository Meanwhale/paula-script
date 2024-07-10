#pragma once
#include "defs.h"
#include "array.h"
namespace paula
{
	// OUTPUT

	class IOutputStream
	{
	public:
		virtual void close() = 0;
		virtual bool closed() = 0;
	};

	class BinaryOut : public IOutputStream
	{
		// TODO if needed
	};

	class POut : public IOutputStream
	{
	public:
		// paula printing interface
		
		virtual const POut& print(char) const = 0; // eg. std::cout<<c; return *this;
		virtual const POut& print(const char*) const = 0;
		virtual const POut& print(long) const = 0;
		virtual const POut& print(double) const = 0;

		const POut& print(int x) const;
		const POut& print(float x) const;
		const POut& print(bool x) const;
		const POut& println(const char*) const;
		const POut& endl() const;

		// TODO
		// printHex(x)
		// printCharSymbol(x)
		// printIntsToText(x*)
	};

	class STDOut : public POut
	{
	public:
		// Inherited via POut
		void close() override;
		bool closed() override;
		const POut& print(char) const override;
		const POut& print(const char*) const override;
		const POut& print(long) const override;
		const POut& print(double) const override;
	};

	// INPUT

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