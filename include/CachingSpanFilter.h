/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SpanFilter.h"
#include "CachingWrapperFilter.h"

namespace Lucene
{
	/// Wraps another SpanFilter's result and caches it.  The purpose is to allow filters to simply filter, 
	/// and then wrap with this class to add caching.
	class LPPAPI CachingSpanFilter : public SpanFilter
	{
	public:
	    /// New deletions always result in a cache miss, by default ({@link CachingWrapperFilter#RECACHE}.
		CachingSpanFilter(SpanFilterPtr filter, CachingWrapperFilter::DeletesMode deletesMode = CachingWrapperFilter::DELETES_RECACHE);
		virtual ~CachingSpanFilter();
	
		LUCENE_CLASS(CachingSpanFilter);
	
	protected:
		SpanFilterPtr filter;		
		FilterCachePtr cache;
	
	public:
		// for testing
		int32_t hitCount;
		int32_t missCount;
	
	public:
		virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader);
		virtual SpanFilterResultPtr bitSpans(IndexReaderPtr reader);
		
		virtual String toString();
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();
	
	protected:
		SpanFilterResultPtr getCachedResult(IndexReaderPtr reader);
	};
	
	class LPPAPI FilterCacheSpanFilterResult : public FilterCache
	{
	public:
	    FilterCacheSpanFilterResult(CachingWrapperFilter::DeletesMode deletesMode);
	    virtual ~FilterCacheSpanFilterResult();
	    
	    LUCENE_CLASS(FilterCacheSpanFilterResult);

    protected:
        virtual LuceneObjectPtr mergeDeletes(IndexReaderPtr reader, LuceneObjectPtr value);
	};
}
