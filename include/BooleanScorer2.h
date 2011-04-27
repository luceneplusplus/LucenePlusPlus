/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOLEANSCORER2_H
#define BOOLEANSCORER2_H

#include "Scorer.h"

namespace Lucene
{
    /// See the description in BooleanScorer, comparing BooleanScorer & BooleanScorer2
    ///
    /// An alternative to BooleanScorer that also allows a minimum number of optional scorers that should match.
    /// Implements skipTo(), and has no limitations on the numbers of added scorers.
    /// Uses ConjunctionScorer, DisjunctionScorer, ReqOptScorer and ReqExclScorer.
    class BooleanScorer2 : public Scorer
    {
    public:
        /// Creates a {@link Scorer} with the given similarity and lists of required, prohibited and optional 
        /// scorers. In no required scorers are added, at least one of the optional scorers will have to match
        /// during the search.
        ///
        /// @param weight The BooleanWeight to be used.
        /// @param disableCoord If this parameter is true, coordination level matching 
        /// ({@link Similarity#coord(int, int)}) is not used.
        /// @param similarity The similarity to be used.
        /// @param minNrShouldMatch The minimum number of optional added scorers that should match during the search. 
        /// In case no required scorers are added, at least one of the optional scorers will have to match during 
        /// the search.
        /// @param required The list of required scorers.
        /// @param prohibited The list of prohibited scorers.
        /// @param optional The list of optional scorers.
        BooleanScorer2(WeightPtr weight, bool disableCoord, SimilarityPtr similarity, int32_t minNrShouldMatch, 
                       Collection<ScorerPtr> required, Collection<ScorerPtr> prohibited, Collection<ScorerPtr> optional, 
                       int32_t maxCoord);
        
        virtual ~BooleanScorer2();
    
        LUCENE_CLASS(BooleanScorer2);
    
    protected:
        Collection<ScorerPtr> requiredScorers;
        Collection<ScorerPtr> optionalScorers;
        Collection<ScorerPtr> prohibitedScorers;
        
        bool disableCoord;
        SimilarityPtr similarity;        
        CoordinatorPtr coordinator;
        
        /// The scorer to which all scoring will be delegated, except for computing and using the coordination factor.
        ScorerPtr countingSumScorer;
        
        int32_t minNrShouldMatch;
        int32_t doc;
        
    public:
        virtual void initialize();
        
        /// Scores and collects all matching documents.
        /// @param collector The collector to which all matching documents are passed through.
        virtual void score(CollectorPtr collector);
        
        virtual bool score(CollectorPtr collector, int32_t max, int32_t firstDocID);
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual double score();
        virtual double freq();
        virtual int32_t advance(int32_t target);
    
    protected:
        ScorerPtr countingDisjunctionSumScorer(Collection<ScorerPtr> scorers, int32_t minNrShouldMatch);
        ScorerPtr countingConjunctionSumScorer(bool disableCoord, SimilarityPtr similarity, Collection<ScorerPtr> requiredScorers);
        ScorerPtr dualConjunctionSumScorer(bool disableCoord, SimilarityPtr similarity, ScorerPtr req1, ScorerPtr req2);
        
        /// Returns the scorer to be used for match counting and score summing.  Uses requiredScorers, optionalScorers 
        /// and prohibitedScorers.
        ScorerPtr makeCountingSumScorer(bool disableCoord, SimilarityPtr similarity);
        ScorerPtr makeCountingSumScorerNoReq(bool disableCoord, SimilarityPtr similarity);
        ScorerPtr makeCountingSumScorerSomeReq(bool disableCoord, SimilarityPtr similarity);
        
        /// Returns the scorer to be used for match counting and score summing.  Uses the given required scorer and 
        /// the prohibitedScorers.
        /// @param requiredCountingSumScorer A required scorer already built.
        ScorerPtr addProhibitedScorers(ScorerPtr requiredCountingSumScorer);
        
        virtual void visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor);
        
        friend class CountingDisjunctionSumScorer;
        friend class CountingConjunctionSumScorer;
        friend class Coordinator;
    };
}

#endif
