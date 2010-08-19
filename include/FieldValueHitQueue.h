/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "HitQueueBase.h"
#include "ScoreDoc.h"

namespace Lucene
{
	/// A hit queue for sorting by hits by terms in more than one field.  Uses FieldCache::DEFAULT for maintaining
	/// internal term lookup tables.
	/// @see Searcher#search(QueryPtr, FilterPtr, int32_t, SortPtr)
	/// @see FieldCache
	class LPPAPI FieldValueHitQueue : public HitQueueBase
	{
	public:
		FieldValueHitQueue(Collection<SortFieldPtr> fields, int32_t size);
		virtual ~FieldValueHitQueue();
	
		LUCENE_CLASS(FieldValueHitQueue);
	
	protected:
		/// Stores the sort criteria being used.
		Collection<SortFieldPtr> fields;
		Collection<FieldComparatorPtr> comparators;
		Collection<int32_t> reverseMul;
	
	public:
		/// Creates a hit queue sorted by the given list of fields.
		/// @param fields SortField array we are sorting by in priority order (highest priority first); cannot 
		/// be null or empty.
		/// @param size The number of hits to retain. Must be greater than zero.
		static FieldValueHitQueuePtr create(Collection<SortFieldPtr> fields, int32_t size);
		
		Collection<FieldComparatorPtr> getComparators();
		Collection<int32_t> getReverseMul();
		
		/// Given a queue Entry, creates a corresponding FieldDoc that contains the values used to sort the given 
		/// document.  These values are not the raw values out of the index, but the internal representation of 
		/// them.  This is so the given search hit can be collated by a MultiSearcher with other search hits.
		/// @param entry The Entry used to create a FieldDoc
		/// @return The newly created FieldDoc
		/// @see Searchable#search(WeightPtr, FilterPtr, int32_t, SortPtr)
		FieldDocPtr fillFields(FieldValueHitQueueEntryPtr entry);
		
		/// Returns the SortFields being used by this hit queue.
		Collection<SortFieldPtr> getFields();
	};
	
	class LPPAPI FieldValueHitQueueEntry : public ScoreDoc
	{
	public:
		FieldValueHitQueueEntry(int32_t slot, int32_t doc, double score);
		virtual ~FieldValueHitQueueEntry();
		
		LUCENE_CLASS(FieldValueHitQueueEntry);
	
	public:
		int32_t slot;
	
	public:
		virtual String toString();
	};
	
	/// An implementation of {@link FieldValueHitQueue} which is optimized in case there is just one comparator.
	class LPPAPI OneComparatorFieldValueHitQueue : public FieldValueHitQueue
	{
	public:
		OneComparatorFieldValueHitQueue(Collection<SortFieldPtr> fields, int32_t size);
		virtual ~OneComparatorFieldValueHitQueue();
		
		LUCENE_CLASS(OneComparatorFieldValueHitQueue);
	
	public:
		FieldComparatorPtr comparator;
		int32_t oneReverseMul;
	
	protected:
		virtual bool lessThan(const ScoreDocPtr& first, const ScoreDocPtr& second);
	};
	
	/// An implementation of {@link FieldValueHitQueue} which is optimized in case there is more than one comparator.
	class LPPAPI MultiComparatorsFieldValueHitQueue : public FieldValueHitQueue
	{
	public:
		MultiComparatorsFieldValueHitQueue(Collection<SortFieldPtr> fields, int32_t size);
		virtual ~MultiComparatorsFieldValueHitQueue();
		
		LUCENE_CLASS(MultiComparatorsFieldValueHitQueue);
	
	protected:
		virtual bool lessThan(const ScoreDocPtr& first, const ScoreDocPtr& second);
	};
}
