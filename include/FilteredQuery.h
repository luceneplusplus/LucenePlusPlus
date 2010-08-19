/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"

namespace Lucene
{
	/// A query that applies a filter to the results of another query.
	///
	/// Note: the bits are retrieved from the filter each time this query is used in a search - use a 
	/// CachingWrapperFilter to avoid regenerating the bits every time.
	///
	/// @see CachingWrapperFilter
	class LPPAPI FilteredQuery : public Query
	{
	public:
		/// Constructs a new query which applies a filter to the results of the original query. 
		/// Filter::getDocIdSet() will be called every time this query is used in a search.
		/// @param query Query to be filtered, cannot be null.
		/// @param filter Filter to apply to query results, cannot be null.
		FilteredQuery(QueryPtr query, FilterPtr filter);
		
		virtual ~FilteredQuery();
	
		LUCENE_CLASS(FilteredQuery);
	
	public:
		QueryPtr query;
		FilterPtr filter;
	
	public:
		using Query::toString;
		
		/// Returns a Weight that applies the filter to the enclosed query's Weight.
		/// This is accomplished by overriding the Scorer returned by the Weight.
		virtual WeightPtr createWeight(SearcherPtr searcher);
		
		/// Rewrites the wrapped query.
		virtual QueryPtr rewrite(IndexReaderPtr reader);
		
		QueryPtr getQuery();
		FilterPtr getFilter();
		
		virtual void extractTerms(SetTerm terms);
		
		/// Prints a user-readable version of this query.
		virtual String toString(const String& field);
		
		virtual bool equals(LuceneObjectPtr other);
		virtual int32_t hashCode();
		virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
		
		friend class FilteredQueryWeight;
	};
	
	class LPPAPI FilteredQueryWeight : public Weight
	{
	public:
		FilteredQueryWeight(FilteredQueryPtr query, WeightPtr weight, SimilarityPtr similarity);
		virtual ~FilteredQueryWeight();
		
		LUCENE_CLASS(FilteredQueryWeight);
	
	protected:
		FilteredQueryPtr query;
		WeightPtr weight;
		SimilarityPtr similarity;
		double value;
	
	public:
		virtual double getValue();
		virtual double sumOfSquaredWeights();
		virtual void normalize(double norm);
		virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
		virtual QueryPtr getQuery();
		virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
		
		friend class FilteredQueryWeightScorer;
	};
	
	class LPPAPI FilteredQueryWeightScorer : public Scorer
	{
	public:
		FilteredQueryWeightScorer(FilteredQueryWeightPtr weight, ScorerPtr scorer, DocIdSetIteratorPtr docIdSetIterator, SimilarityPtr similarity);
		virtual ~FilteredQueryWeightScorer();
		
		LUCENE_CLASS(FilteredQueryWeightScorer);
	
	protected:
		FilteredQueryWeightPtr weight;
		ScorerPtr scorer;
		DocIdSetIteratorPtr docIdSetIterator;
		int32_t doc;
	
	public:
		virtual int32_t nextDoc();
		virtual int32_t docID();
		virtual int32_t advance(int32_t target);
		virtual double score();
	
	protected:
		int32_t advanceToCommon(int32_t scorerDoc, int32_t disiDoc);
	};
}
