/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArrayReader.h"

namespace Lucene
{
    ByteArrayReader::ByteArrayReader(ByteArray array) : ArrayReader<uint8_t>(array.get(), array.length())
    {
    }
    
    ByteArrayReader::ByteArrayReader(const uint8_t* array, int32_t length) : ArrayReader<uint8_t>(array, length)
    {
    }
    
    ByteArrayReader::~ByteArrayReader()
    {
    }
    
    CharArrayReader::CharArrayReader(CharArray array) : ArrayReader<wchar_t>(array.get(), array.length())
    {
    }
    
    CharArrayReader::CharArrayReader(const wchar_t* array, int32_t length) : ArrayReader<wchar_t>(array, length)
    {
    }
    
    CharArrayReader::~CharArrayReader()
    {
    }
}
