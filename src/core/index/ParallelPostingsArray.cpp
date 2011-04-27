/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ParallelPostingsArray.h"
#include "MiscUtils.h"

namespace Lucene
{
    const int32_t ParallelPostingsArray::BYTES_PER_POSTING = 3 * sizeof(int32_t);
    
    ParallelPostingsArray::ParallelPostingsArray(int32_t size)
    {
        this->size = size;
        textStarts = IntArray::newInstance(size);
        intStarts = IntArray::newInstance(size);
        byteStarts = IntArray::newInstance(size);
    }
    
    ParallelPostingsArray::~ParallelPostingsArray()
    {
    }
    
    int32_t ParallelPostingsArray::bytesPerPosting()
    {
        return BYTES_PER_POSTING;
    }
    
    ParallelPostingsArrayPtr ParallelPostingsArray::newInstance(int32_t size)
    {
        return newLucene<ParallelPostingsArray>(size);
    }
    
    ParallelPostingsArrayPtr ParallelPostingsArray::grow()
    {
        int32_t newSize = MiscUtils::oversize(size + 1, bytesPerPosting());
        ParallelPostingsArrayPtr newArray(newInstance(newSize));
        copyTo(newArray, size);
        return newArray;
    }
    
    void ParallelPostingsArray::copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy)
    {
        MiscUtils::arrayCopy(textStarts.get(), 0, toArray->textStarts.get(), 0, numToCopy);
        MiscUtils::arrayCopy(intStarts.get(), 0, toArray->intStarts.get(), 0, numToCopy);
        MiscUtils::arrayCopy(byteStarts.get(), 0, toArray->byteStarts.get(), 0, numToCopy);
    }
}
