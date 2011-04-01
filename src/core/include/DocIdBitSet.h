/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCIDBITSET_H
#define DOCIDBITSET_H

#include "DocIdSet.h"
#include "DocIdSetIterator.h"

namespace Lucene
{
    /// Simple DocIdSet and DocIdSetIterator backed by a BitSet
    class LPPAPI DocIdBitSet : public DocIdSet
    {
    public:
        DocIdBitSet();
        DocIdBitSet(BitSetPtr bitSet);
        
        virtual ~DocIdBitSet();
        
        LUCENE_CLASS(DocIdBitSet);
    
    protected:
        BitSetPtr bitSet;
    
    public:
        virtual DocIdSetIteratorPtr iterator();
        
        /// This DocIdSet implementation is cacheable.
        virtual bool isCacheable();
        
        /// Returns the underlying BitSet.
        BitSetPtr getBitSet();
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
    };
    
    class LPPAPI DocIdBitSetIterator : public DocIdSetIterator
    {
    public:
        DocIdBitSetIterator(BitSetPtr bitSet);
        virtual ~DocIdBitSetIterator();
        
        LUCENE_CLASS(DocIdBitSetIterator);
    
    protected:
        int32_t docId;
        BitSetPtr bitSet;
    
    public:
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual int32_t advance(int32_t target);
    };
}

#endif
