/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Filter.h"
#include "FilteredDocIdSet.h"

namespace Lucene
{
	
	
	/// Wraps another filter's result and caches it.  The purpose is to allow filters to simply filter, and 
	/// then wrap with this class to add caching.
	class LPPAPI CachingWrapperFilter : public Filter
	{
	public:
	    /// Specifies how new deletions against a reopened reader should be handled.
	    ///
	    /// The default is IGNORE, which means the cache entry will be re-used for a given segment, even when 
	    /// that segment has been reopened due to changes in deletions.  This is a big performance gain, 
	    /// especially with near-real-timer readers, since you don't hit a cache miss on every reopened reader 
	    /// for prior segments.
	    ///
	    /// However, in some cases this can cause invalid query results, allowing deleted documents to be 
	    /// returned.  This only happens if the main query does not rule out deleted documents on its own, 
	    /// such as a toplevel ConstantScoreQuery.  To fix this, use RECACHE to re-create the cached filter 
	    /// (at a higher per-reopen cost, but at faster subsequent search performance), or use DYNAMIC to 
	    /// dynamically intersect deleted docs (fast reopen time but some hit to search performance).
	    enum DeletesMode { DELETES_IGNORE, DELETES_RECACHE, DELETES_DYNAMIC };
	    
	    /// New deletes are ignored by default, which gives higher cache hit rate on reopened readers.  
	    /// Most of the time this is safe, because the filter will be AND'd with a Query that fully enforces 
	    /// deletions.  If instead you need this filter to always enforce deletions, pass either {@link 
	    /// DeletesMode#RECACHE} or {@link DeletesMode#DYNAMIC}.
		CachingWrapperFilter(FilterPtr filter, DeletesMode deletesMode = DELETES_IGNORE);
		
		virtual ~CachingWrapperFilter();
	
		LUCENE_CLASS(CachingWrapperFilter);
	
	public:
	    FilterPtr filter;
		
		/// A Filter cache
		FilterCachePtr cache;
		
		// for testing
		int32_t hitCount;
		int32_t missCount;
	
	protected:
		/// Provide the DocIdSet to be cached, using the DocIdSet provided by the wrapped Filter.
		///
		/// This implementation returns the given {@link DocIdSet}, if {@link DocIdSet#isCacheable} returns 
		/// true, else it copies the {@link DocIdSetIterator} into an {@link OpenBitSetDISI}.
		DocIdSetPtr docIdSetToCache(DocIdSetPtr docIdSet, IndexReaderPtr reader);
	
	public:
		virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader);
		
		virtual String toString();
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();
	};
	
	class LPPAPI FilterCache : public LuceneObject
	{
	public:
	    FilterCache(CachingWrapperFilter::DeletesMode deletesMode);
	    virtual ~FilterCache();
	    
	    LUCENE_CLASS(FilterCache);

    public:
	    WeakMapObjectObject cache;
	    CachingWrapperFilter::DeletesMode deletesMode;

	public:
	    virtual LuceneObjectPtr get(IndexReaderPtr reader, LuceneObjectPtr coreKey, LuceneObjectPtr delCoreKey);
	    virtual void put(LuceneObjectPtr coreKey, LuceneObjectPtr delCoreKey, LuceneObjectPtr value);
    
    protected:
        virtual LuceneObjectPtr mergeDeletes(IndexReaderPtr reader, LuceneObjectPtr value) = 0;
	};
	
	class LPPAPI FilterCacheDocIdSet : public FilterCache
	{
	public:
	    FilterCacheDocIdSet(CachingWrapperFilter::DeletesMode deletesMode);
	    virtual ~FilterCacheDocIdSet();
	    
	    LUCENE_CLASS(FilterCacheDocIdSet);

    protected:
        virtual LuceneObjectPtr mergeDeletes(IndexReaderPtr reader, LuceneObjectPtr value);
	};
	
	class LPPAPI FilteredCacheDocIdSet : public FilteredDocIdSet
	{
	public:
	    FilteredCacheDocIdSet(IndexReaderPtr reader, DocIdSetPtr innerSet);
	    virtual ~FilteredCacheDocIdSet();
	    
	    LUCENE_CLASS(FilteredCacheDocIdSet);
	
	protected:
	    IndexReaderPtr reader;
	
	protected:
	    virtual bool match(int32_t docid);
	};
}
