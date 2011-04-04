/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "CachingSpanFilter.h"
#include "_CachingSpanFilter.h"
#include "SpanFilterResult.h"
#include "IndexReader.h"

namespace Lucene
{
    CachingSpanFilter::CachingSpanFilter(SpanFilterPtr filter, CachingWrapperFilter::DeletesMode deletesMode)
    {
        this->filter = filter;
        if (deletesMode == CachingWrapperFilter::DELETES_DYNAMIC)
            boost::throw_exception(IllegalArgumentException(L"DeletesMode::DYNAMIC is not supported"));
        this->cache = newLucene<FilterCacheSpanFilterResult>(deletesMode);
        this->hitCount = 0;
        this->missCount = 0;
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
        LuceneObjectPtr coreKey = reader->getFieldCacheKey();
        LuceneObjectPtr delCoreKey = reader->hasDeletions() ? reader->getDeletesCacheKey() : coreKey;
        
        SpanFilterResultPtr result(boost::dynamic_pointer_cast<SpanFilterResult>(cache->get(reader, coreKey, delCoreKey)));
        if (result)
        {
            ++hitCount;
            return result;
        }
        
        ++missCount;
        result = filter->bitSpans(reader);
        
        cache->put(coreKey, delCoreKey, result);
        
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
    
    FilterCacheSpanFilterResult::FilterCacheSpanFilterResult(CachingWrapperFilter::DeletesMode deletesMode) : FilterCache(deletesMode)
    {
    }
    
    FilterCacheSpanFilterResult::~FilterCacheSpanFilterResult()
    {
    }
    
    LuceneObjectPtr FilterCacheSpanFilterResult::mergeDeletes(IndexReaderPtr reader, LuceneObjectPtr value)
    {
        boost::throw_exception(IllegalStateException(L"DeletesMode::DYNAMIC is not supported"));
        return LuceneObjectPtr();
    }
}
