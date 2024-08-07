#pragma once
#include "defs.h"
#ifndef PAULA_MINI
#include <fstream>
#endif

namespace paula
{
	namespace core { class TreeIterator; }
	class Var;

	// OUTPUT

	class IOutputStream
	{
	public:
		virtual void flush()  const = 0;
		virtual void close()  const = 0;
		virtual bool closed() const = 0;
	};

	class POut : public IOutputStream
	{
	public:

		virtual void flush()  const = 0;

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

		const POut& print(const core::TreeIterator& x) const;

		const POut& print(const Var& x) const;

		// TODO
		// printIntsToText(x*)
	};

	class NullPrint
	{
		// dummy printer that does nothing.
		// use to get debug prints optimized away on release.
	public:
		void flush() const;
		const NullPrint& print(const char*) const;
		const NullPrint& print(double) const; // take care of all numbers and bool
		const NullPrint& printHex(INT i) const;
		const NullPrint& printCharSymbol(CHAR c) const;
		const NullPrint& print(const Error* a) const;
		const NullPrint& println(const char*) const;
		const NullPrint& print(const core::TreeIterator& x) const;
		const NullPrint& endl() const;
	};
	class STDOut : public POut
	{
	public:
		// Inherited via POut
		void flush()  const override;
		void close()  const override;
		bool closed() const override;
		const POut& print(char) const override;
		const POut& print(const char*) const override;
		const POut& print(long) const override;
		const POut& print(double) const override;
	};
	class STDErr : public POut
	{
	public:
		// Inherited via POut
		void flush()  const override;
		void close()  const override;
		bool closed() const  override;
		const POut& print(char) const override;
		const POut& print(const char*) const override;
		const POut& print(long) const override;
		const POut& print(double) const override;
	};

	// INPUT

	class IInputStream
	{
	public:
		virtual bool read(BYTE&) = 0; // we don't know we're at end before reaching it
		virtual void close() = 0;
	};

	class CharInput : public IInputStream
	{
	public:
		CharInput(const char *);
		bool read(BYTE&) override;
		void close() override;
	private:
		const char * str;
		INT i;
	};

	class StandardInput : public IInputStream
	{
	public:
		StandardInput() {};
		// Inherited via IInputStream
		bool read(BYTE&) override;
		void close() override;
	};

#ifndef PAULA_MINI
	class FileInput : public IInputStream
	{
	private:
		std::ifstream file; // The internal ifstream object
		FileInput() = delete;
	public:
		static bool exists(const std::string& name);

		explicit FileInput(const char *);
		~FileInput();
		// Inherited via IInputStream
		bool read(BYTE&) override;
		void close() override;
		const bool found;
	};
#endif
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