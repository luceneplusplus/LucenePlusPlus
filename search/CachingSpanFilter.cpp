/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CachingSpanFilter.h"
#include "SpanFilterResult.h"
#include "IndexReader.h"

namespace Lucene
{
    CachingSpanFilter::CachingSpanFilter(SpanFilterPtr filter)
    {
        this->filter = filter;
        this->lock = newInstance<Synchronize>();
    }
    
    CachingSpanFilter::~CachingSpanFilter()
    {
    }
    
    DocIdSetPtr CachingSpanFilter::getDocIdSet(IndexReaderPtr reader)
    {
        SpanFilterResultPtr result(getCachedResult(reader));
        return result ? result->getDocIdSet() : DocIdSetPtr();
    }
    
    SpanFilterResultPtr CachingSpanFilter::getCachedResult(IndexReaderPtr reader)
    {
        {
            SyncLock syncLock(lock);
            if (!cache)
                cache = WeakMapIndexReaderSpanFilterResult::newInstance();
            SpanFilterResultPtr cached(cache.get(reader));
            if (cached)
                return cached;
        }
        
        SpanFilterResultPtr result(filter->bitSpans(reader));
        {
            SyncLock syncLock(lock);
            cache.put(reader, result);
        }
        
        return result;
    }
    
    SpanFilterResultPtr CachingSpanFilter::bitSpans(IndexReaderPtr reader)
    {
        return getCachedResult(reader);
    }
    
    String CachingSpanFilter::toString()
    {
        return L"CachingSpanFilter(" + filter->toString() + L")";
    }
    
    bool CachingSpanFilter::equals(LuceneObjectPtr other)
    {
        if (SpanFilter::equals(other))
            return true;
        
        CachingSpanFilterPtr otherCachingSpanFilter(boost::dynamic_pointer_cast<CachingSpanFilter>(other));
        if (!otherCachingSpanFilter)
            return false;
        
        return this->filter->equals(otherCachingSpanFilter->filter);
    }
    
    int32_t CachingSpanFilter::hashCode()
    {
        return filter->hashCode() ^ 0x1117bf25;
    }
}
