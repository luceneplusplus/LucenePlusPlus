/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Searcher.h"
#include "Collector.h"

namespace Lucene
{
	/// Implements search over a set of Searchables.
	///
	/// Applications usually need only call the inherited {@link #search(QueryPtr, int32_t)} or {@link 
	/// #search(QueryPtr, FilterPtr, int32_t)} methods.
	class LPPAPI MultiSearcher : public Searcher
	{
	public:
		/// Creates a searcher which searches searchers.
		MultiSearcher(Collection<SearchablePtr> searchables);
		
		virtual ~MultiSearcher();
	
		LUCENE_CLASS(MultiSearcher);
	
	protected:
		Collection<SearchablePtr> searchables;
		Collection<int32_t> starts;
		int32_t _maxDoc;
	
	public:
		using Searcher::search;
		
		/// Return the array of {@link Searchable}s this searches.
		Collection<SearchablePtr> getSearchables();
		
		virtual void close();
		virtual int32_t docFreq(TermPtr term);
		virtual DocumentPtr doc(int32_t n);
		virtual DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector);
		
		/// Returns index of the searcher for document n in the array used to construct this searcher.
		int32_t subSearcher(int32_t n);
		
		/// Returns the document number of document n within its sub-index.
		int32_t subDoc(int32_t n);
		
		virtual int32_t maxDoc();
		virtual TopDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n);
		virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort);
		virtual void search(WeightPtr weight, FilterPtr filter, CollectorPtr results);
		virtual QueryPtr rewrite(QueryPtr query);
		virtual ExplanationPtr explain(WeightPtr weight, int32_t doc);
		
	protected:
		Collection<int32_t> getStarts();
		
		/// Create weight in multiple index scenario.
		///
		/// Distributed query processing is done in the following steps:
		/// 1. rewrite query.
		/// 2. extract necessary terms.
		/// 3. collect dfs for these terms from the Searchables.
		/// 4. create query weight using aggregate dfs.
		/// 5. distribute that weight to Searchables.
		/// 6. merge results.
		///
		/// Steps 1-4 are done here, 5+6 in the search() methods
		///
		/// @return rewritten queries
		virtual WeightPtr createWeight(QueryPtr query);
	};
	
	/// Document Frequency cache acting as a Dummy-Searcher.  This class is not a full-fledged Searcher, but 
	/// only supports the methods necessary to initialize Weights.
	class LPPAPI CachedDfSource : public Searcher
	{
	public:
		CachedDfSource(MapTermInt dfMap, int32_t maxDoc, SimilarityPtr similarity);
		virtual ~CachedDfSource();
	
		LUCENE_CLASS(CachedDfSource);
	
	protected:
		MapTermInt dfMap; // Map from Terms to corresponding doc freqs
		int32_t _maxDoc; // document count
	
	public:
		virtual int32_t docFreq(TermPtr term);
		virtual Collection<int32_t> docFreqs(Collection<TermPtr> terms);
		virtual int32_t maxDoc();
		virtual QueryPtr rewrite(QueryPtr query);
		virtual void close();
		virtual DocumentPtr doc(int32_t n);
		virtual DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector);
		virtual ExplanationPtr explain(WeightPtr weight, int32_t doc);
		virtual void search(WeightPtr weight, FilterPtr filter, CollectorPtr results);
		virtual TopDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n);
		virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort);
	};

	/// A subclass for searching a single searchable
	class LPPAPI MultiSearcherCallableNoSort : public LuceneObject
	{
	public:
		MultiSearcherCallableNoSort(SynchronizePtr lock, SearchablePtr searchable, WeightPtr weight, FilterPtr filter, int32_t nDocs, 
									HitQueuePtr hq, int32_t i, Collection<int32_t> starts);
		virtual ~MultiSearcherCallableNoSort();
	
		LUCENE_CLASS(MultiSearcherCallableNoSort);
	
	protected:
		SynchronizePtr lock;
		SearchablePtr searchable;
		WeightPtr weight;
		FilterPtr filter;
		int32_t nDocs;
		int32_t i;
		HitQueuePtr hq;
		Collection<int32_t> starts;
	
	public:
		TopDocsPtr call();
	};
	
	/// A subclass for searching a single searchable
	class LPPAPI MultiSearcherCallableWithSort : public LuceneObject
	{
	public:
		MultiSearcherCallableWithSort(SynchronizePtr lock, SearchablePtr searchable, WeightPtr weight, FilterPtr filter, 
									  int32_t nDocs, FieldDocSortedHitQueuePtr hq, SortPtr sort, int32_t i, Collection<int32_t> starts);
		virtual ~MultiSearcherCallableWithSort();
	
		LUCENE_CLASS(MultiSearcherCallableWithSort);
	
	protected:
		SynchronizePtr lock;
		SearchablePtr searchable;
		WeightPtr weight;
		FilterPtr filter;
		int32_t nDocs;
		int32_t i;
		FieldDocSortedHitQueuePtr hq;
		Collection<int32_t> starts;
		SortPtr sort;
	
	public:
		TopFieldDocsPtr call();
	};
	
	class LPPAPI MultiSearcherCollector : public Collector
	{
	public:
		MultiSearcherCollector(CollectorPtr collector, int32_t start);
		virtual ~MultiSearcherCollector();
		
		LUCENE_CLASS(MultiSearcherCollector);
	
	protected:
		CollectorPtr collector;
		int32_t start;
	
	public:
		virtual void setScorer(ScorerPtr scorer);
		virtual void collect(int32_t doc);
		virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
		virtual bool acceptsDocsOutOfOrder();
	};
}
