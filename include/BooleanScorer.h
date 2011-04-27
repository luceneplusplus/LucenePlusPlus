/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOLEANSCORER_H
#define BOOLEANSCORER_H

#include "Scorer.h"

namespace Lucene
{
    /// BooleanScorer uses a ~16k array to score windows of docs. So it scores docs 0-16k first, then docs 16-32k,
    /// etc. For each window it iterates through all query terms and accumulates a score in table[doc%16k]. It also 
    /// stores in the table a bitmask representing which terms contributed to the score.  Non-zero scores are chained 
    /// in a linked list. At the end of scoring each window it then iterates through the linked list and, if the 
    /// bitmask matches the boolean constraints, collects a hit.  For boolean queries with lots of frequent terms this 
    /// can be much faster, since it does not need to update a priority queue for each posting, instead performing 
    /// constant-time operations per posting.  The only downside is that it results in hits being delivered out-of-order 
    /// within the window, which means it cannot be nested within other scorers.  But it works well as a top-level scorer.
    ///
    /// The new BooleanScorer2 implementation instead works by merging priority queues of postings, albeit with some
    /// clever tricks.  For example, a pure conjunction (all terms required) does not require a priority queue. Instead it
    /// sorts the posting streams at the start, then repeatedly skips the first to to the last.  If the first ever equals
    /// the last, then there's a hit.  When some terms are required and some terms are optional, the conjunction can
    /// be evaluated first, then the optional terms can all skip to the match and be added to the score. Thus the 
    /// conjunction can reduce the number of priority queue updates for the optional terms.
    class BooleanScorer : public Scorer
    {
    public:
        BooleanScorer(WeightPtr weight, bool disableCoord, SimilarityPtr similarity, int32_t minNrShouldMatch, 
                      Collection<ScorerPtr> optionalScorers, Collection<ScorerPtr> prohibitedScorers, int32_t maxCoord);
        virtual ~BooleanScorer();
    
        LUCENE_CLASS(BooleanScorer);
    
    protected:
        SubScorerPtr scorers;
        BucketTablePtr bucketTable;
        DoubleArray coordFactors;
        int32_t prohibitedMask;
        int32_t nextMask;
        int32_t minNrShouldMatch;
        int32_t end;
        BucketPtr current;
        int32_t doc;
    
    protected:
        // firstDocID is ignored since nextDoc() initializes 'current'
        virtual bool score(CollectorPtr collector, int32_t max, int32_t firstDocID);
    
    public:
        virtual int32_t advance(int32_t target);
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual double score();
        virtual void score(CollectorPtr collector);
        virtual String toString();
        virtual void visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor);
    };
}

#endif
