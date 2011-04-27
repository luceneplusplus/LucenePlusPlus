/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PARALLELPOSTINGSARRAY_H
#define PARALLELPOSTINGSARRAY_H

#include "LuceneObject.h"

namespace Lucene
{
    class ParallelPostingsArray : public LuceneObject
    {
    public:
        ParallelPostingsArray(int32_t size);
        virtual ~ParallelPostingsArray();
        
        LUCENE_CLASS(ParallelPostingsArray);
            
    public:
        static const int32_t BYTES_PER_POSTING;

        int32_t size;
        IntArray textStarts;
        IntArray intStarts;
        IntArray byteStarts;
    
    public:
        virtual int32_t bytesPerPosting();
        virtual ParallelPostingsArrayPtr newInstance(int32_t size);
        virtual ParallelPostingsArrayPtr grow();
        virtual void copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy);
    };
}

#endif
