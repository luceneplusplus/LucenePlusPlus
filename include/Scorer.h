/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SCORER_H
#define SCORER_H

#include "DocIdSetIterator.h"
#include "BooleanClause.h"

namespace Lucene
{
    /// Common scoring functionality for different types of queries.
    ///
    /// A Scorer iterates over documents matching a query in increasing order of doc Id.
    ///
    /// Document scores are computed using a given Similarity implementation.
    ///
    /// NOTE: The values NEGATIVE_INFINITY and POSITIVE_INFINITY are not valid scores.  Certain collectors 
    /// (eg {@link TopScoreDocCollector}) will not properly collect hits with these scores.
    class LPPAPI Scorer : public DocIdSetIterator
    {
    public:
        /// Constructs a Scorer
        /// @param weight The scorers Weight.
        Scorer(WeightPtr weight);
        
        /// Constructs a Scorer.
        /// @param similarity The Similarity implementation used by this scorer.
        /// @deprecated Use {@link #Scorer(Weight)} instead.
        Scorer(SimilarityPtr similarity);
        
        /// Constructs a Scorer
        /// @param similarity The Similarity implementation used by this scorer.
        /// @param weight The scorers Weight
        /// @deprecated Use {@link #Scorer(Weight)} instead.
        Scorer(SimilarityPtr similarity, WeightPtr weight);
        
        virtual ~Scorer();
    
        LUCENE_CLASS(Scorer);
    
    public:
        SimilarityPtr similarity;
        WeightPtr weight;
    
    public:
        /// Returns the Similarity implementation used by this scorer. 
        /// @deprecated Store any Similarity you might need privately in your implementation instead.
        SimilarityPtr getSimilarity();
        
        /// Scores and collects all matching documents.
        /// @param collector The collector to which all matching documents are passed.
        virtual void score(CollectorPtr collector);
        
        /// Returns the score of the current document matching the query.  Initially invalid, until {@link 
        /// #nextDoc()} or {@link #advance(int32_t)} is called the first time, or when called from within
        /// {@link Collector#collect}.
        virtual double score() = 0;
        
        /// Returns number of matches for the current document. This returns a double (not int) because
        /// SloppyPhraseScorer discounts its freq according to how "sloppy" the match was.
        virtual double freq();
        
        /// Call this to gather details for all sub-scorers for this query. This can be used, in 
        /// conjunction with a custom {@link Collector} to gather details about how each sub-query matched 
        /// the current hit.
        /// @param visitor a callback executed for each sub-scorer
        virtual void visitScorers(ScorerVisitorPtr visitor);
    
        /// Collects matching documents in a range.  Hook for optimization.
        /// Note, firstDocID is added to ensure that {@link #nextDoc()} was called before this method.
        ///
        /// @param collector The collector to which all matching documents are passed.
        /// @param max Do not score documents past this.
        /// @param firstDocID The first document ID (ensures {@link #nextDoc()} is called before this method.
        /// @return true if more matching documents may remain.
        virtual bool score(CollectorPtr collector, int32_t max, int32_t firstDocID);
        
        /// {@link Scorer} subclasses should implement this method if the subclass itself contains 
        /// multiple scorers to support gathering details for sub-scorers via {@link ScorerVisitor}
        ///
        /// Note: this method will throw {@link UnsupportedOperationException} if no associated {@link 
        /// Weight} instance is provided to {@link #Scorer(Weight)}
        virtual void visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor);
    };
    
    /// A callback to gather information from a scorer and its sub-scorers. Each the top-level scorer 
    /// as well as each of its sub-scorers are passed to either one of the visit methods depending on their 
    /// boolean relationship in the query.
    class LPPAPI ScorerVisitor : public DocIdSetIterator
    {
    public:
        virtual ~ScorerVisitor();
        LUCENE_CLASS(ScorerVisitor);

    public:
        /// Invoked for all optional scorer 
        /// @param parent the parent query of the child query or null if the child is a top-level query
        /// @param child the query of the currently visited scorer
        /// @param scorer the current scorer
        virtual void visitOptional(QueryPtr parent, QueryPtr child, ScorerPtr scorer);
        
        /// Invoked for all required scorer 
        /// @param parent the parent query of the child query or null if the child is a top-level query
        /// @param child the query of the currently visited scorer
        /// @param scorer the current scorer
        virtual void visitRequired(QueryPtr parent, QueryPtr child, ScorerPtr scorer);
        
        /// Invoked for all prohibited scorer 
        /// @param parent the parent query of the child query or null if the child is a top-level query
        /// @param child the query of the currently visited scorer
        /// @param scorer the current scorer
        virtual void visitProhibited(QueryPtr parent, QueryPtr child, ScorerPtr scorer);
    };
}

#endif
