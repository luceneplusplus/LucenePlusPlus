/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Reader.h"

namespace Lucene
{
	/// Convenience class for reading arrays.
	template <typename TYPE>
	class ArrayReader : public Reader
	{
	public:
		ArrayReader(const TYPE* array, int32_t length)
		{
		    this->array = array;
		    this->_length = length;
		    this->position = 0;
		}
		
		virtual ~ArrayReader()
		{
		}
	
	protected:
		const TYPE* array;
		int32_t _length;
		int32_t position;
	
	public:
		virtual int32_t read()
		{
		    return position == _length ? READER_EOF : (int32_t)array[position++];
		}

		virtual int32_t read(wchar_t* buffer, int32_t offset, int32_t length)
		{
		    if (position >= _length)
                return READER_EOF;
            int32_t readLength = std::min(length, _length - position);
            std::copy(array + position, array + position + readLength, buffer + offset);
            position += readLength;
            return readLength;
		}
		
		virtual void close()
		{
		}
		
		virtual bool markSupported()
		{
		    return false;
		}
		
		virtual void reset()
		{
		    position = 0;
		}
		
		virtual int64_t length()
		{
		    return _length;
		}
	};
	
	/// Convenience class for reading byte arrays.
	class LPPAPI ByteArrayReader : public ArrayReader<uint8_t>
	{
	public:
		ByteArrayReader(ByteArray array);
		ByteArrayReader(const uint8_t* array, int32_t length);
		
		virtual ~ByteArrayReader();
		
		LUCENE_CLASS(ByteArrayReader);
    };
    
    /// Convenience class for reading char arrays.
	class LPPAPI CharArrayReader : public ArrayReader<wchar_t>
	{
	public:
		CharArrayReader(CharArray array);
		CharArrayReader(const wchar_t* array, int32_t length);
		
		virtual ~CharArrayReader();
		
		LUCENE_CLASS(CharArrayReader);
    };
}
