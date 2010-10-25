/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "OpenBitSetDISI.h"

namespace Lucene
{
    OpenBitSetDISI::OpenBitSetDISI(DocIdSetIteratorPtr disi, int32_t maxSize) : OpenBitSet(maxSize)
    {
        inPlaceOr(disi);
    }
    
    OpenBitSetDISI::OpenBitSetDISI(int32_t maxSize) : OpenBitSet(maxSize)
    {
    }
    
    OpenBitSetDISI::~OpenBitSetDISI()
    {
    }
    
    void OpenBitSetDISI::inPlaceOr(DocIdSetIteratorPtr disi)
    {
        int32_t doc;
        int32_t _size = size();
        while ((doc = disi->nextDoc()) < _size)
            set(doc);
    }
    
    void OpenBitSetDISI::inPlaceAnd(DocIdSetIteratorPtr disi)
    {
        int32_t bitSetDoc = nextSetBit(0);
        int32_t disiDoc;
        while (bitSetDoc != -1 && (disiDoc = disi->advance(bitSetDoc)) != DocIdSetIterator::NO_MORE_DOCS)
        {
            clear(bitSetDoc, disiDoc);
            bitSetDoc = nextSetBit(disiDoc + 1);
        }
        if (bitSetDoc != -1)
            clear(bitSetDoc, size());
    }
    
    void OpenBitSetDISI::inPlaceNot(DocIdSetIteratorPtr disi)
    {
        int32_t doc;
        int32_t _size = size();
        while ((doc = disi->nextDoc()) < _size)
            clear(doc);
    }
    
    void OpenBitSetDISI::inPlaceXor(DocIdSetIteratorPtr disi)
    {
        int32_t doc;
        int32_t _size = size();
        while ((doc = disi->nextDoc()) < _size)
            flip(doc);
    }
}
