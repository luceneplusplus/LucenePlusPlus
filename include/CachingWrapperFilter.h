/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Filter.h"

namespace Lucene
{
	/// Wraps another filter's result and caches it.  The purpose is to allow filters to simply filter, and 
	/// then wrap with this class to add caching.
	class LPPAPI CachingWrapperFilter : public Filter
	{
	public:
		CachingWrapperFilter(FilterPtr filter);
		virtual ~CachingWrapperFilter();
	
		LUCENE_CLASS(CachingWrapperFilter);
	
	public:
		FilterPtr filter;
		
		/// A Filter cache
		WeakMapIndexReaderDocIdSet cache;
		SynchronizePtr lock;
	
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
}
