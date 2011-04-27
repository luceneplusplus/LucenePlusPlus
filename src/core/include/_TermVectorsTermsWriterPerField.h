/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TERMVECTORSTERMSWRITERPERFIELD_H
#define _TERMVECTORSTERMSWRITERPERFIELD_H

#include "ParallelPostingsArray.h"

namespace Lucene
{
    class TermVectorsPostingsArray : public ParallelPostingsArray
    {
    public:
        TermVectorsPostingsArray(int32_t size);
        virtual ~TermVectorsPostingsArray();
    
        LUCENE_CLASS(TermVectorsPostingsArray);
        
    public:
        IntArray freqs; // How many times this term occurred in the current doc
        IntArray lastOffsets; // Last offset we saw
        IntArray lastPositions; // Last position where this term occurred
    
    public:
        virtual int32_t bytesPerPosting();
        virtual ParallelPostingsArrayPtr newInstance(int32_t size);
        virtual void copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy);
    };
}

#endif
