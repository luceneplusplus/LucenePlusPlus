/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BufferedReader.h"

namespace Lucene
{
    const int32_t BufferedReader::READER_BUFFER = 1024;
    
    BufferedReader::BufferedReader(ReaderPtr reader)
    {
        this->reader = reader;
        bufferStart = 0;
        bufferLength = 0;
        bufferPosition = 0;
    }
    
    BufferedReader::~BufferedReader()
    {
    }
    
    int32_t BufferedReader::read()
    {
        if (bufferPosition >= bufferLength)
            refill();
        return buffer[bufferPosition++];
    }
    
    int32_t BufferedReader::peek()
    {
        if (bufferPosition >= bufferLength)
            refill();
        return buffer[bufferPosition];
    }
    
    int32_t BufferedReader::read(wchar_t* b, int32_t offset, int32_t length)
    {
        int32_t readChars = 0;
        try
        {
            for (; readChars < length; ++readChars)
                b[readChars + offset] = (wchar_t)read();
        }
        catch (IOException&)
        {
        }
        return readChars;
    }
    
    String BufferedReader::readLine()
    {
        String line;
        try
        {
            wchar_t ch = (wchar_t)read();
            while (ch != L'\r' && ch != L'\n')
            {
                line += ch;
                ch = (wchar_t)read();
            }
            if (ch == '\r' && (wchar_t)peek() == L'\n')
                read();
        }
        catch (...)
        {
        }
        return line;
    }
    
    void BufferedReader::refill()
    {
        int32_t start = bufferStart + bufferPosition;
        int32_t end = start + READER_BUFFER;
        if (end > reader->length()) // don't read past EOF
            end = (int32_t)reader->length();
        int32_t newLength = (int32_t)(end - start);
        if (newLength <= 0)
            boost::throw_exception(IOException(L"Read past EOF"));
        if (!buffer)
            buffer = CharArray::newInstance(READER_BUFFER); // allocate buffer lazily
        reader->read(buffer.get(), 0, newLength);
        bufferLength = newLength;
        bufferStart = start;
        bufferPosition = 0;
    }

    void BufferedReader::close()
    {
        reader->close();
        bufferStart = 0;
        bufferLength = 0;
        bufferPosition = 0;
    }
    
    bool BufferedReader::markSupported()
    {
        return false;
    }
    
    void BufferedReader::reset()
    {
        reader->reset();
        bufferStart = 0;
        bufferLength = 0;
        bufferPosition = 0;
    }
}
