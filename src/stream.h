#pragma once
#include "defs.h"
//#include "array.h"
namespace paula
{
	class TreeIterator;
	class Var;

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
		const POut& printHex(INT i) const;
		const POut& printCharSymbol(CHAR c) const;
		const POut& print(const Error* a) const;
		const POut& println(const char*) const;
		const POut& endl() const;

		const POut& print(const TreeIterator& x) const;

		const POut& print(const Var& x) const;

		// TODO
		// printIntsToText(x*)
	};

	class NullPrint
	{
		// dummy printer that does nothing.
		// use to get debug prints optimized away on release.
	public:
		const NullPrint& print(const char*) const;
		const NullPrint& print(double) const; // take care of all numbers and bool
		const NullPrint& printHex(INT i) const;
		const NullPrint& printCharSymbol(CHAR c) const;
		const NullPrint& print(const Error* a) const;
		const NullPrint& println(const char*) const;
		const NullPrint& endl() const;
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
	class STDErr : public POut
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

	//template <class CHAR> class Array;

	//class BufferInputStream : public IInputStream
	//{
	//public:
	//	BufferInputStream(Array<CHAR>&, INT start, INT last);
	//	CHAR read() override;
	//	bool end() override;
	//	void close() override;
	//private:
	//	Array<CHAR> str;
	//	INT i, last;
	//};
}