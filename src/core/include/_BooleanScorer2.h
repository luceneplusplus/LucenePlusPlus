/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _BOOLEANSCORER2_H
#define _BOOLEANSCORER2_H

#include "DisjunctionSumScorer.h"
#include "ConjunctionScorer.h"

namespace Lucene
{
    class Coordinator : public LuceneObject
    {
    public:
        Coordinator(BooleanScorer2Ptr scorer);
        virtual ~Coordinator();
        
        LUCENE_CLASS(Coordinator);
    
    public:
        BooleanScorer2WeakPtr _scorer;
        DoubleArray coordFactors;
        int32_t maxCoord; // to be increased for each non prohibited scorer
        int32_t nrMatchers; // to be increased by score() of match counting scorers.
    
    public:
        void init(SimilarityPtr sim, bool disableCoord); // use after all scorers have been added.
        
        friend class BooleanScorer2;
    };
    
    /// Count a scorer as a single match.
    class SingleMatchScorer : public Scorer
    {
    public:
        SingleMatchScorer(ScorerPtr scorer, CoordinatorPtr coordinator);
        virtual ~SingleMatchScorer();
        
        LUCENE_CLASS(SingleMatchScorer);
    
    protected:
        ScorerPtr scorer;
        CoordinatorPtr coordinator;
        int32_t lastScoredDoc;
        double lastDocScore;
    
    public:
        virtual double score();
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual int32_t advance(int32_t target);
    };
    
    class CountingDisjunctionSumScorer : public DisjunctionSumScorer
    {
    public:
        CountingDisjunctionSumScorer(BooleanScorer2Ptr scorer, WeightPtr weight, Collection<ScorerPtr> subScorers, int32_t minimumNrMatchers);
        virtual ~CountingDisjunctionSumScorer();
        
        LUCENE_CLASS(CountingDisjunctionSumScorer);
    
    protected:
        BooleanScorer2WeakPtr _scorer;
        int32_t lastScoredDoc;
        
        // Save the score of lastScoredDoc, so that we don't compute it more than once in score().
        double lastDocScore;
    
    public:
        virtual double score();
        
        friend class BooleanScorer2;
    };
    
    class CountingConjunctionSumScorer : public ConjunctionScorer
    {
    public:
        CountingConjunctionSumScorer(BooleanScorer2Ptr scorer, WeightPtr weight, double coord, Collection<ScorerPtr> scorers);
        virtual ~CountingConjunctionSumScorer();
        
        LUCENE_CLASS(CountingConjunctionSumScorer);
    
    protected:
        BooleanScorer2WeakPtr _scorer;
        int32_t lastScoredDoc;
        int32_t requiredNrMatchers;
        
        // Save the score of lastScoredDoc, so that we don't compute it more than once in score().
        double lastDocScore;
    
    public:
        virtual double score();
    };
}

#endif
