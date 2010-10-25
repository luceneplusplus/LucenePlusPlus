/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef OPENBITSET_H
#define OPENBITSET_H

#include "DocIdBitSet.h"

namespace Lucene
{
    class LPPAPI OpenBitSet : public DocIdBitSet
    {
    public:
        OpenBitSet(uint32_t numBits = 64);
        virtual ~OpenBitSet();
    
        LUCENE_CLASS(OpenBitSet);
    
    public:
        virtual DocIdSetIteratorPtr iterator();
        virtual bool isCacheable();
        
        void clear();
        void clear(uint32_t bitIndex);
        void clear(uint32_t fromIndex, uint32_t toIndex);
        void fastClear(uint32_t bitIndex);
        void set(uint32_t bitIndex);
        void set(uint32_t fromIndex, uint32_t toIndex);
        void fastSet(uint32_t bitIndex);
        void flip(uint32_t bitIndex);
        void flip(uint32_t fromIndex, uint32_t toIndex);
        void fastFlip(uint32_t bitIndex);
        bool flipAndGet(uint32_t bitIndex);
        uint32_t size();
        bool isEmpty();
        bool get(uint32_t bitIndex);
        bool fastGet(uint32_t bitIndex);
        bool getAndSet(uint32_t bitIndex);
        int32_t nextSetBit(uint32_t fromIndex);
        void andBitSet(OpenBitSetPtr set);
        void orBitSet(OpenBitSetPtr set);
        void xorBitSet(OpenBitSetPtr set);
        void andNotBitSet(OpenBitSetPtr set);
        bool intersectsBitSet(OpenBitSetPtr set);
        uint32_t cardinality();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Returns the popcount or cardinality of the intersection of the two sets.  Neither set is modified.
        static uint64_t intersectionCount(OpenBitSetPtr first, OpenBitSetPtr second);
        
        /// Returns the popcount or cardinality of the union of the two sets.  Neither set is modified.
        static uint64_t unionCount(OpenBitSetPtr first, OpenBitSetPtr second);
        
        /// Returns the popcount or cardinality of "a and not b" or "intersection(a, not(b))". 
        /// Neither set is modified.
        static uint64_t andNotCount(OpenBitSetPtr first, OpenBitSetPtr second);
        
        /// Returns the popcount or cardinality of the exclusive-or of the two sets.  Neither set is modified.
        static uint64_t xorCount(OpenBitSetPtr first, OpenBitSetPtr second);
    };
}

#endif
