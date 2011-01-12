/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CONSTANTSCOREQUERY_H
#define CONSTANTSCOREQUERY_H

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"

namespace Lucene
{
    /// A query that wraps a filter and simply returns a constant score equal to the query boost for every 
    /// document in the filter.
    class LPPAPI ConstantScoreQuery : public Query
    {
    public:
        ConstantScoreQuery(FilterPtr filter);
        virtual ~ConstantScoreQuery();
    
        LUCENE_CLASS(ConstantScoreQuery);
    
    protected:
        FilterPtr filter;
    
    public:
        using Query::toString;
        
        /// Returns the encapsulated filter
        FilterPtr getFilter();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        virtual void extractTerms(SetTerm terms);
        
        virtual WeightPtr createWeight(SearcherPtr searcher);
        
        /// Prints a user-readable version of this query.
        virtual String toString(const String& field);
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        friend class ConstantWeight;
        friend class ConstantScorer;
    };
    
    class LPPAPI ConstantWeight : public Weight
    {
    public:
        ConstantWeight(ConstantScoreQueryPtr constantScorer, SearcherPtr searcher);
        virtual ~ConstantWeight();
    
        LUCENE_CLASS(ConstantWeight);
    
    protected:
        ConstantScoreQueryPtr constantScorer;
        SimilarityPtr similarity;
        double queryNorm;
        double queryWeight;
    
    public:
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
    };
    
    class LPPAPI ConstantScorer : public Scorer
    {
    public:
        ConstantScorer(ConstantScoreQueryPtr constantScorer, SimilarityPtr similarity, IndexReaderPtr reader, WeightPtr w);
        virtual ~ConstantScorer();
    
        LUCENE_CLASS(ConstantScorer);
    
    public:
        DocIdSetIteratorPtr docIdSetIterator;
        double theScore;
        int32_t doc;
    
    public:
        virtual int32_t nextDoc();
        virtual int32_t docID();
        virtual double score();
        virtual int32_t advance(int32_t target);
    };
}

#endif
