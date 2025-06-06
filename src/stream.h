#pragma once
#include "defs.h"
#include "array.h"
#include <fstream>

namespace paula
{
	namespace core { class TreeIterator; }
	class Var;

	// OUTPUT

	/**
	 * @brief Base class for output.
	 */
	class IOutputStream
	{
	public:
		virtual void flush()  = 0;
		virtual void close()  = 0;
		virtual bool closed() const = 0;
	};
	class BinaryOutputStream : public IOutputStream
	{
	public:
		void writeArray(INT*, INT size);
		virtual void write(INT i) = 0;
	};
	class StandardBinaryOutput : public BinaryOutputStream
	{
	public:
		virtual void flush()  override;
		virtual void close()  override;
		virtual bool closed() const override;
		void write(INT) override;
	};
	class FileBinaryOutput : public BinaryOutputStream
	{
	private:
		std::ofstream out;
	public:
		FileBinaryOutput(const char *);
		virtual void flush()  override;
		virtual void close()  override;
		virtual bool closed() const override;
		void write(INT) override;
	};
	/**
	 * @brief Base class for print output.
	 */
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

		const POut& print(const core::TreeIterator& x) const;

		const POut& print(const Var& x) const;

		// TODO
		// printIntsToText(x*)
	};

	/**
	 * @brief Dummy printer. Use to optimize out debug prints.
	 */
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
		const NullPrint& print(const Var& x) const;
		const NullPrint& endl() const;
	};
	/**
	 * @brief Print output using C++ std cout.
	 */
	class STDOut : public POut
	{
	public:
		// Inherited via POut
		void flush() override;
		void close() override;
		bool closed() const override;
		const POut& print(char) const override;
		const POut& print(const char*) const override;
		const POut& print(long) const override;
		const POut& print(double) const override;
	};
	/**
	* @brief Print output using C++ std err.
	*/
	class STDErr : public POut
	{
	public:
		// Inherited via POut
		void flush() override;
		void close() override;
		bool closed() const  override;
		const POut& print(char) const override;
		const POut& print(const char*) const override;
		const POut& print(long) const override;
		const POut& print(double) const override;
	};

	// INPUT

	/**
	 * @brief Base class for input streams.
	 */
	class IInputStream
	{
	public:
		bool readInt(INT&);
		virtual bool read(BYTE&) = 0; // we don't know we're at end before reaching it
		virtual void close() = 0;
	};
	/**
	 * @brief Input stream for constant strings.
	 */
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
	/**
	 * @brief Input stream for C++ std input.
	 */
	class StandardInput : public IInputStream
	{
	public:
		StandardInput() {};
		// Inherited via IInputStream
		bool read(BYTE&) override;
		void close() override;
	};
	class StandardBinaryInput : public IInputStream
	{
	public:
		StandardBinaryInput() {};
		// Inherited via IInputStream
		bool read(BYTE&) override;
		void close() override;
	};

#ifndef PAULA_MINI
	/**
	 * @brief Input stream for C++ std ifstream.
	 */
	class FileInput : public IInputStream
	{
	private:
		std::ifstream file; // The internal ifstream object
		FileInput() = delete;
	public:
		static bool exists(const std::string& name);

		explicit FileInput(const char *, bool binary);
		~FileInput();
		// Inherited via IInputStream
		bool read(BYTE&) override;
		void close() override;
	};


	// for testing

	class ArrayBinaryOutput : public BinaryOutputStream
	{
	private:
		INT i;
	public:
		core::Array<char>buffer;
		ArrayBinaryOutput();
		void flush() override;
		void close() override;
		bool closed() const override;
		void write(INT) override;
		INT getByteSize() const;
	};
	class ArrayBinaryInput : public IInputStream
	{
	private:
		int i, size;
		core::Array<char> &buffer;
	public:
		ArrayBinaryInput(core::Array<char> & _buffer, INT _size);
		bool read(BYTE&) override;
		bool readInt(INT&);
		void close() override;
	};
#endif
}