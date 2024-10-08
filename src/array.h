#ifndef _ARRAY_H_
#define _ARRAY_H_
#include "utils.h"
#include "stream.h"

namespace paula { namespace core
{

template <class T> class Array
{
protected:

	T*				data;
	int				size; // size of reserved array; 0 if not allocated (, negative on error?)
	bool			destructorClear;


public:

	explicit Array() :
		data(0),
		size(0),
		destructorClear(false)
	{
	}
	explicit Array(int s) :
		data(new T[s]),
		size(s),
		destructorClear(true)
	{
	}
	explicit Array(T * a, int s) :
		data(a),
		size(s),
		destructorClear(false)
	{
	}

	~Array()
	{
		if (destructorClear) clear();
	}

	void clear()
	{
		if (data != 0)
		{
			delete[] data;
			data = 0;
			size = 0;
		}
	}

	void copyFrom(const Array<T>& src)
	{
		if (src.size != size)
		{
			ASSERT(false);
			return;
		}
		for (int i=0; i<size; i++) data[i] = src.data[i];
	}

	void reset(int s)
	{
		delete[] data;
		data = new T[s];
		size = s;
		destructorClear = true;
	}
	bool inRange(int i) const
	{
		return i >= 0 && i < size;
	}

	void fill(T a)
	{
		for (int i = 0; i < size; i++)
			data[i] = a;
	}


	// ACCESSORS

	T * ptr() const
	{
		return data;
	}
	T * ptr(int i) const
	{
		ASSERT_MSG(inRange(i), "ARRAY_OUT_OF_RANGE");
		return data + i;
	}
	T & get(int i)
	{
		ASSERT_MSG(inRange(i), "ARRAY_OUT_OF_RANGE");
		return data[i];
	}
	T & operator[] (int i)
	{
		ASSERT_MSG(inRange(i), "ARRAY_OUT_OF_RANGE");
		return data[i];
	}
	int length()
	{
		return size;
	}
};
}}
#endif // _ARRAY_H_
