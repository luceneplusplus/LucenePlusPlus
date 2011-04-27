/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FREQPROXTERMSWRITERPERFIELD_H
#define _FREQPROXTERMSWRITERPERFIELD_H

#include "ParallelPostingsArray.h"

namespace Lucene
{
    class FreqProxPostingsArray : public ParallelPostingsArray
    {
    public:
        FreqProxPostingsArray(int32_t size);
        virtual ~FreqProxPostingsArray();
        
        LUCENE_CLASS(FreqProxPostingsArray);
            
    public:
        IntArray docFreqs; // # times this term occurs in the current doc
        IntArray lastDocIDs; // Last docID where this term occurred
        IntArray lastDocCodes; // Code for prior doc
        IntArray lastPositions; // Last position where this term occurred
    
    public:
        virtual ParallelPostingsArrayPtr newInstance(int32_t size);
        virtual void copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy);
    };
}

#endif
