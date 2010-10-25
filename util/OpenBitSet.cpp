/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "OpenBitSet.h"
#include "BitSet.h"
#include "BitUtil.h"
#include "OpenBitSetIterator.h"

namespace Lucene
{
    OpenBitSet::OpenBitSet(uint32_t numBits)
    {
        bitSet = newLucene<BitSet>(numBits);
    }
    
    OpenBitSet::~OpenBitSet()
    {
    }
    
    DocIdSetIteratorPtr OpenBitSet::iterator()
    {
        return newLucene<OpenBitSetIterator>(shared_from_this());
    }
    
    bool OpenBitSet::isCacheable()
    {
        return true;
    }
    
    void OpenBitSet::clear()
    {
        bitSet->clear();
    }
    
    void OpenBitSet::clear(uint32_t bitIndex)
    {
        bitSet->clear(bitIndex);
    }
    
    void OpenBitSet::clear(uint32_t fromIndex, uint32_t toIndex)
    {
        bitSet->clear(fromIndex, toIndex);
    }
    
    void OpenBitSet::fastClear(uint32_t bitIndex)
    {
        bitSet->fastClear(bitIndex);
    }
    
    void OpenBitSet::set(uint32_t bitIndex)
    {
        bitSet->set(bitIndex);
    }
    
    void OpenBitSet::set(uint32_t fromIndex, uint32_t toIndex)
    {
        bitSet->set(fromIndex, toIndex);
    }
    
    void OpenBitSet::fastSet(uint32_t bitIndex)
    {
        bitSet->fastSet(bitIndex);
    }
        
    void OpenBitSet::flip(uint32_t bitIndex)
    {
        bitSet->flip(bitIndex);
    }
    
    void OpenBitSet::flip(uint32_t fromIndex, uint32_t toIndex)
    {
        bitSet->flip(fromIndex, toIndex);
    }
    
    void OpenBitSet::fastFlip(uint32_t bitIndex)
    {
        bitSet->fastFlip(bitIndex);
    }
    
    bool OpenBitSet::flipAndGet(uint32_t bitIndex)
    {
        bitSet->flip(bitIndex);
        return bitSet->get(bitIndex);
    }
    
    uint32_t OpenBitSet::size()
    {
        return bitSet->size();
    }
    
    bool OpenBitSet::isEmpty()
    {
        return bitSet->isEmpty();
    }
    
    bool OpenBitSet::get(uint32_t bitIndex)
    {
        return bitSet->get(bitIndex);
    }
    
    bool OpenBitSet::fastGet(uint32_t bitIndex)
    {
        return bitSet->fastGet(bitIndex);
    }
    
    bool OpenBitSet::getAndSet(uint32_t bitIndex)
    {
        bool prev = bitSet->get(bitIndex);
        bitSet->set(bitIndex);
        return prev;
    }
    
    int32_t OpenBitSet::nextSetBit(uint32_t fromIndex)
    {
        return bitSet->nextSetBit(fromIndex);
    }
    
    void OpenBitSet::andBitSet(OpenBitSetPtr set)
    {
        bitSet->andBitSet(set->bitSet);
    }
    
    void OpenBitSet::orBitSet(OpenBitSetPtr set)
    {
        bitSet->orBitSet(set->bitSet);
    }
    
    void OpenBitSet::xorBitSet(OpenBitSetPtr set)
    {
        bitSet->xorBitSet(set->bitSet);
    }
    
    void OpenBitSet::andNotBitSet(OpenBitSetPtr set)
    {
        bitSet->andNotBitSet(set->bitSet);
    }
    
    bool OpenBitSet::intersectsBitSet(OpenBitSetPtr set)
    {
        return bitSet->intersectsBitSet(set->bitSet);
    }
    
    uint32_t OpenBitSet::cardinality()
    {
        return bitSet->cardinality();
    }
    
    LuceneObjectPtr OpenBitSet::clone(LuceneObjectPtr other)
    {
        return DocIdBitSet::clone(other ? other : newLucene<OpenBitSet>());
    }
    
    uint64_t OpenBitSet::intersectionCount(OpenBitSetPtr first, OpenBitSetPtr second)
    {
        return BitUtil::pop_intersect(first->bitSet->getBits(), second->bitSet->getBits(), 0, std::min(first->bitSet->numBlocks(), second->bitSet->numBlocks()));
    }
    
    uint64_t OpenBitSet::unionCount(OpenBitSetPtr first, OpenBitSetPtr second)
    {
        uint64_t total = BitUtil::pop_union(first->bitSet->getBits(), second->bitSet->getBits(), 0, std::min(first->bitSet->numBlocks(), second->bitSet->numBlocks()));
        if (first->bitSet->numBlocks() < second->bitSet->numBlocks())
            total += BitUtil::pop_array(second->bitSet->getBits(), first->bitSet->numBlocks(), second->bitSet->numBlocks() - first->bitSet->numBlocks());
        else
            total += BitUtil::pop_array(first->bitSet->getBits(), second->bitSet->numBlocks(), first->bitSet->numBlocks() - second->bitSet->numBlocks());
        return total;
    }
    
    uint64_t OpenBitSet::andNotCount(OpenBitSetPtr first, OpenBitSetPtr second)
    {
        uint64_t total = BitUtil::pop_andnot(first->bitSet->getBits(), second->bitSet->getBits(), 0, std::min(first->bitSet->numBlocks(), second->bitSet->numBlocks()));
        if (first->bitSet->numBlocks() > second->bitSet->numBlocks())
            total += BitUtil::pop_array(first->bitSet->getBits(), second->bitSet->numBlocks(), first->bitSet->numBlocks() - second->bitSet->numBlocks());
        return total;
    }
    
    uint64_t OpenBitSet::xorCount(OpenBitSetPtr first, OpenBitSetPtr second)
    {
        uint64_t total = BitUtil::pop_xor(first->bitSet->getBits(), second->bitSet->getBits(), 0, std::min(first->bitSet->numBlocks(), second->bitSet->numBlocks()));
        if (first->bitSet->numBlocks() < second->bitSet->numBlocks())
            total += BitUtil::pop_array(second->bitSet->getBits(), first->bitSet->numBlocks(), second->bitSet->numBlocks() - first->bitSet->numBlocks());
        else
            total += BitUtil::pop_array(first->bitSet->getBits(), second->bitSet->numBlocks(), first->bitSet->numBlocks() - second->bitSet->numBlocks());
        return total;
    }
}
