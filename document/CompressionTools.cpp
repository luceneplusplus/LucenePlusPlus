/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressionTools.h"
#include "UnicodeUtils.h"
#include "zlib.h"

namespace Lucene
{
    const int32_t CompressionTools::COMPRESS_BUFFER = 4096;
    
    CompressionTools::~CompressionTools()
    {
    }
    
    ByteArray CompressionTools::compress(uint8_t* value, int32_t offset, int32_t length, int32_t compressionLevel)
    {
        z_stream stream;
        stream.zalloc = zlibAlloc;
        stream.zfree = zlibFree;
        stream.opaque = Z_NULL;

        if (deflateInit(&stream, compressionLevel) != Z_OK)
        {
            boost::throw_exception(CompressionException(L"deflateInit failure:" + StringUtils::toString(stream.msg == NULL ? "" : stream.msg)));
            return ByteArray();
        }

        stream.avail_in = length;
        stream.next_in = value + offset;
        
        // No guarantee that the compressed data will be smaller than the uncompressed data
        stream.avail_out = length + COMPRESS_BUFFER;
        
        ByteArray compress(ByteArray::newInstance(stream.avail_out));
        stream.next_out = compress.get();
        
        bool success = (deflate(&stream, Z_FINISH) == Z_STREAM_END);
        String deflateMessage(StringUtils::toString(stream.msg == NULL ? "" : stream.msg));
        
        compress.resize(stream.total_out);
        deflateEnd(&stream);
        
        if (!success)
            boost::throw_exception(CompressionException(L"deflate failure:" + deflateMessage));
        
        return compress;
    }
    
    ByteArray CompressionTools::compress(uint8_t* value, int32_t offset, int32_t length)
    {
        return compress(value, offset, length, Z_BEST_COMPRESSION);
    }
    
    ByteArray CompressionTools::compress(ByteArray value)
    {
        return compress(value.get(), 0, value.length(), Z_BEST_COMPRESSION);
    }
    
    ByteArray CompressionTools::compressString(const String& value)
    {
        return compressString(value, Z_BEST_COMPRESSION);
    }
    
    ByteArray CompressionTools::compressString(const String& value, int32_t compressionLevel)
    {
        UTF8ResultPtr utf8Result(newLucene<UTF8Result>());
        StringUtils::toUTF8(value.c_str(), value.length(), utf8Result);
        return compress(utf8Result->result.get(), 0, utf8Result->length, compressionLevel);
    }
    
    ByteArray CompressionTools::decompress(ByteArray value)
    {
        z_stream stream;
        stream.avail_in = 0;
        stream.next_in = Z_NULL;
        stream.zalloc = zlibAlloc;
        stream.zfree = zlibFree;
        stream.opaque = Z_NULL;

        if (inflateInit(&stream) != Z_OK)
        {
            boost::throw_exception(CompressionException(L"inflateInit failure:" + StringUtils::toString(stream.msg == NULL ? "" : stream.msg)));
            return ByteArray();
        }

        stream.avail_in = value.length();
        stream.next_in = value.get();
        
        int32_t allocSize = COMPRESS_BUFFER;
        ByteArray decompress(ByteArray::newInstance(allocSize));
        
        int32_t position = 0;
        int32_t inflateCode = Z_OK;
        bool success = true;
        String inflateMessage;
        
        while (inflateCode != Z_STREAM_END)
        {
            decompress.resize(allocSize);
            
            stream.avail_out = allocSize - position;
            stream.next_out = decompress.get() + position;
            
            inflateCode = inflate(&stream, Z_NO_FLUSH);
            
            if (inflateCode != Z_OK && inflateCode != Z_STREAM_END)
            {
                success = false;
                inflateMessage = StringUtils::toString(stream.msg == NULL ? "" : stream.msg);
                break;
            }
            
            position = stream.total_out;			
            allocSize <<= 1;
        }
        
        decompress.resize(stream.total_out);
        inflateEnd(&stream);
        
        if (!success)
            boost::throw_exception(CompressionException(L"inflate failure:" + inflateMessage));
        
        return decompress;
    }
    
    String CompressionTools::decompressString(ByteArray value)
    {
        ByteArray bytes(decompress(value));
        return StringUtils::toUnicode(bytes.get(), bytes.length());
    }
    
    void* CompressionTools::zlibAlloc(void* opaque, unsigned int items, unsigned int size)
    {
        return (void*)AllocMemory((size_t)(items * size));
    }
    
    void CompressionTools::zlibFree(void* opaque, void* buffer)
    {
        FreeMemory((uint8_t*)buffer);
    }
}
