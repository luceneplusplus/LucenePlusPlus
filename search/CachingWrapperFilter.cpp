/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CachingWrapperFilter.h"
#include "OpenBitSetDISI.h"
#include "IndexReader.h"

namespace Lucene
{
    CachingWrapperFilter::CachingWrapperFilter(FilterPtr filter)
    {
        this->filter = filter;
        this->lock = newInstance<Synchronize>();
    }
    
    CachingWrapperFilter::~CachingWrapperFilter()
    {
    }
    
    DocIdSetPtr CachingWrapperFilter::docIdSetToCache(DocIdSetPtr docIdSet, IndexReaderPtr reader)
    {
        if (docIdSet->isCacheable())
            return docIdSet;
        else
        {
            DocIdSetIteratorPtr it(docIdSet->iterator());
            // null is allowed to be returned by iterator(), in this case we wrap with the empty set,
            // which is cacheable.
            return !it ? DocIdSet::EMPTY_DOCIDSET() : newLucene<OpenBitSetDISI>(it, reader->maxDoc());
        }
    }
    
    DocIdSetPtr CachingWrapperFilter::getDocIdSet(IndexReaderPtr reader)
    {
        {
            SyncLock syncLock(lock);
            if (!cache)
                cache = WeakMapIndexReaderDocIdSet::newInstance();
            DocIdSetPtr cached(cache.get(reader));
            if (cached)
                return cached;
        }
        
        DocIdSetPtr docIdSet(docIdSetToCache(filter->getDocIdSet(reader), reader));
        if (docIdSet)
        {
            SyncLock syncLock(lock);
            cache.put(reader, docIdSet);
        }
        
        return docIdSet;
    }
    
    String CachingWrapperFilter::toString()
    {
        return L"CachingWrapperFilter(" + filter->toString() + L")";
    }
    
    bool CachingWrapperFilter::equals(LuceneObjectPtr other)
    {
        if (Filter::equals(other))
            return true;
        
        CachingWrapperFilterPtr otherCachingWrapperFilter(boost::dynamic_pointer_cast<CachingWrapperFilter>(other));
        if (!otherCachingWrapperFilter)
            return false;
        
        return this->filter->equals(otherCachingWrapperFilter->filter);
    }
    
    int32_t CachingWrapperFilter::hashCode()
    {
        return filter->hashCode() ^ 0x1117bf25;
    }
}
