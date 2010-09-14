/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputStreamReader.h"
#include "BufferedReader.h"
#include "UTF8Stream.h"

namespace Lucene
{
    InputStreamReader::InputStreamReader(ReaderPtr reader)
    {
        this->reader = reader;
        this->decoder = newLucene<UTF8Decoder>(newLucene<BufferedReader>(reader, 1024));
    }
    
    InputStreamReader::~InputStreamReader()
    {
    }
    
    int32_t InputStreamReader::read()
    {
        int32_t buffer;
        return read((wchar_t*)&buffer, 0, 1) == READER_EOF ? READER_EOF : buffer;
    }
    
    int32_t InputStreamReader::read(wchar_t* b, int32_t offset, int32_t length)
    {
        return decoder->decode(b + offset, length);
    }
    
    void InputStreamReader::close()
    {
        reader->close();
    }
    
    bool InputStreamReader::markSupported()
    {
        return false;
    }
    
    void InputStreamReader::reset()
    {
        reader->reset();
    }
}
