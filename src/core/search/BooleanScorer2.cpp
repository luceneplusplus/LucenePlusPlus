/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "BooleanScorer2.h"
#include "_BooleanScorer2.h"
#include "ReqOptSumScorer.h"
#include "ReqExclScorer.h"
#include "Similarity.h"
#include "Collector.h"
#include "Weight.h"

namespace Lucene
{
    BooleanScorer2::BooleanScorer2(WeightPtr weight, bool disableCoord, SimilarityPtr similarity, int32_t minNrShouldMatch, 
                                   Collection<ScorerPtr> required, Collection<ScorerPtr> prohibited, Collection<ScorerPtr> optional,
                                   int32_t maxCoord) : Scorer(weight)
    {
        this->minNrShouldMatch = minNrShouldMatch;
        this->coordinator->maxCoord = maxCoord;

        this->disableCoord = disableCoord;
        this->similarity = similarity;
        
        this->optionalScorers = optional;
        this->requiredScorers = required;    
        this->prohibitedScorers = prohibited;

        this->doc = -1;
    }
    
    BooleanScorer2::~BooleanScorer2()
    {
    }
    
    void BooleanScorer2::initialize()
    {
        if (minNrShouldMatch < 0)
            boost::throw_exception(IllegalArgumentException(L"Minimum number of optional scorers should not be negative"));

        coordinator = newLucene<Coordinator>(shared_from_this());
        coordinator->init(similarity, disableCoord);
        countingSumScorer = makeCountingSumScorer(disableCoord, similarity);
    }
    
    ScorerPtr BooleanScorer2::countingDisjunctionSumScorer(Collection<ScorerPtr> scorers, int32_t minNrShouldMatch)
    {
        // each scorer from the list counted as a single matcher
        return newLucene<CountingDisjunctionSumScorer>(shared_from_this(), weight, scorers, minNrShouldMatch);
    }
    
    ScorerPtr BooleanScorer2::countingConjunctionSumScorer(bool disableCoord, SimilarityPtr similarity, Collection<ScorerPtr> requiredScorers)
    {
        // each scorer from the list counted as a single matcher
        return newLucene<CountingConjunctionSumScorer>(shared_from_this(), weight, disableCoord ? 1.0 : similarity->coord(requiredScorers.size(), requiredScorers.size()), requiredScorers);
    }
    
    ScorerPtr BooleanScorer2::dualConjunctionSumScorer(bool disableCoord, SimilarityPtr similarity, ScorerPtr req1, ScorerPtr req2)
    {
        Collection<ScorerPtr> scorers(newCollection<ScorerPtr>(req1, req2));
        
        // All scorers match, so Similarity::getDefault() always has 1 as the coordination factor.
        // Therefore the sum of the scores of two scorers is used as score.
        return newLucene<ConjunctionScorer>(weight, disableCoord ? 1.0 : similarity->coord(2, 2), scorers);
    }
    
    ScorerPtr BooleanScorer2::makeCountingSumScorer(bool disableCoord, SimilarityPtr similarity)
    {
        return requiredScorers.empty() ? makeCountingSumScorerNoReq(disableCoord, similarity) : makeCountingSumScorerSomeReq(disableCoord, similarity);
    }
    
    ScorerPtr BooleanScorer2::makeCountingSumScorerNoReq(bool disableCoord, SimilarityPtr similarity)
    {
        // minNrShouldMatch optional scorers are required, but at least 1
        int32_t nrOptRequired = minNrShouldMatch < 1 ? 1 : minNrShouldMatch;
        ScorerPtr requiredCountingSumScorer;
        if (optionalScorers.size() > nrOptRequired)
            requiredCountingSumScorer = countingDisjunctionSumScorer(optionalScorers, nrOptRequired);
        else if (optionalScorers.size() == 1)
            requiredCountingSumScorer = newLucene<SingleMatchScorer>(optionalScorers[0], coordinator);
        else
            requiredCountingSumScorer = countingConjunctionSumScorer(disableCoord, similarity, optionalScorers);
        return addProhibitedScorers(requiredCountingSumScorer);
    }
    
    ScorerPtr BooleanScorer2::makeCountingSumScorerSomeReq(bool disableCoord, SimilarityPtr similarity)
    {
        if (optionalScorers.size() == minNrShouldMatch) // all optional scorers also required.
        {
            Collection<ScorerPtr> allReq(Collection<ScorerPtr>::newInstance(requiredScorers.begin(), requiredScorers.end()));
            allReq.addAll(optionalScorers.begin(), optionalScorers.end());
            return addProhibitedScorers(countingConjunctionSumScorer(disableCoord, similarity, allReq));
        }
        else // optionalScorers.size() > minNrShouldMatch, and at least one required scorer
        {
            ScorerPtr requiredCountingSumScorer = requiredScorers.size() == 1 ? newLucene<SingleMatchScorer>(requiredScorers[0], coordinator) : countingConjunctionSumScorer(disableCoord, similarity, requiredScorers);
            if (minNrShouldMatch > 0) // use a required disjunction scorer over the optional scorers
                return addProhibitedScorers(dualConjunctionSumScorer(disableCoord, similarity, requiredCountingSumScorer, countingDisjunctionSumScorer(optionalScorers, minNrShouldMatch)));
            else // minNrShouldMatch == 0
                return newLucene<ReqOptSumScorer>(addProhibitedScorers(requiredCountingSumScorer), optionalScorers.size() == 1 ? newLucene<SingleMatchScorer>(optionalScorers[0], coordinator) : countingDisjunctionSumScorer(optionalScorers, 1));
        }
    }
    
    ScorerPtr BooleanScorer2::addProhibitedScorers(ScorerPtr requiredCountingSumScorer)
    {
        return prohibitedScorers.empty() ? requiredCountingSumScorer : newLucene<ReqExclScorer>(requiredCountingSumScorer, (prohibitedScorers.size() == 1 ? prohibitedScorers[0] : newLucene<DisjunctionSumScorer>(weight, prohibitedScorers)));
    }
    
    void BooleanScorer2::score(CollectorPtr collector)
    {
        collector->setScorer(shared_from_this());
        while ((doc = countingSumScorer->nextDoc()) != NO_MORE_DOCS)
            collector->collect(doc);
    }
    
    bool BooleanScorer2::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        doc = firstDocID;
        collector->setScorer(shared_from_this());
        while (doc < max)
        {
            collector->collect(doc);
            doc = countingSumScorer->nextDoc();
        }
        return (doc != NO_MORE_DOCS);
    }
    
    int32_t BooleanScorer2::docID()
    {
        return doc;
    }

    int32_t BooleanScorer2::nextDoc()
    {
        doc = countingSumScorer->nextDoc();
        return doc;
    }
    
    double BooleanScorer2::score()
    {
        coordinator->nrMatchers = 0;
        double sum = countingSumScorer->score();
        return sum * coordinator->coordFactors[coordinator->nrMatchers];
    }
    
    double BooleanScorer2::freq()
    {
        return coordinator->nrMatchers;
    }
    
    int32_t BooleanScorer2::advance(int32_t target)
    {
        doc = countingSumScorer->advance(target);
        return doc;
    }
    
    void BooleanScorer2::visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor)
    {
        Scorer::visitSubScorers(parent, relationship, visitor);
        QueryPtr q(weight->getQuery());
        for (Collection<ScorerPtr>::iterator s = optionalScorers.begin(); s != optionalScorers.end(); ++s)
            (*s)->visitSubScorers(q, BooleanClause::SHOULD, visitor);
        for (Collection<ScorerPtr>::iterator s = prohibitedScorers.begin(); s != prohibitedScorers.end(); ++s)
            (*s)->visitSubScorers(q, BooleanClause::MUST_NOT, visitor);
        for (Collection<ScorerPtr>::iterator s = requiredScorers.begin(); s != requiredScorers.end(); ++s)
            (*s)->visitSubScorers(q, BooleanClause::MUST, visitor);
    }
    
    Coordinator::Coordinator(BooleanScorer2Ptr scorer)
    {
        _scorer = scorer;
        maxCoord = 0;
        nrMatchers = 0;
    }
    
    Coordinator::~Coordinator()
    {
    }
    
    void Coordinator::init(SimilarityPtr sim, bool disableCoord)
    {
        BooleanScorer2Ptr scorer(_scorer);
        coordFactors = DoubleArray::newInstance(scorer->optionalScorers.size() + scorer->requiredScorers.size() + 1);
        for (int32_t i = 0; i < coordFactors.size(); ++i)
            coordFactors[i] = disableCoord ? 1.0 : sim->coord(i, maxCoord);
    }
    
    SingleMatchScorer::SingleMatchScorer(ScorerPtr scorer, CoordinatorPtr coordinator) : Scorer(scorer->weight)
    {
        lastScoredDoc = -1;
        lastDocScore = std::numeric_limits<double>::quiet_NaN();
        this->scorer = scorer;
        this->coordinator = coordinator;
    }
    
    SingleMatchScorer::~SingleMatchScorer()
    {
    }
    
    double SingleMatchScorer::score()
    {
        int32_t doc = docID();
        if (doc >= lastScoredDoc)
        {
            if (doc > lastScoredDoc)
            {
                lastDocScore = scorer->score();
                lastScoredDoc = doc;
            }
            ++coordinator->nrMatchers;
        }
        return lastDocScore;
    }
    
    int32_t SingleMatchScorer::docID()
    {
        return scorer->docID();
    }
    
    int32_t SingleMatchScorer::nextDoc()
    {
        return scorer->nextDoc();
    }
    
    int32_t SingleMatchScorer::advance(int32_t target)
    {
        return scorer->advance(target);
    }
    
    CountingDisjunctionSumScorer::CountingDisjunctionSumScorer(BooleanScorer2Ptr scorer, WeightPtr weight, Collection<ScorerPtr> subScorers, int32_t minimumNrMatchers) : DisjunctionSumScorer(weight, subScorers, minimumNrMatchers)
    {
        _scorer = scorer;
        lastScoredDoc = -1;
        lastDocScore = std::numeric_limits<double>::quiet_NaN();
    }
    
    CountingDisjunctionSumScorer::~CountingDisjunctionSumScorer()
    {
    }
    
    double CountingDisjunctionSumScorer::score()
    {
        int32_t doc = docID();
        if (doc >= lastScoredDoc)
        {
            if (doc > lastScoredDoc)
            {
                lastDocScore = DisjunctionSumScorer::score();
                lastScoredDoc = doc;
            }
            BooleanScorer2Ptr(_scorer)->coordinator->nrMatchers += DisjunctionSumScorer::_nrMatchers;
        }
        return lastDocScore;
    }
    
    CountingConjunctionSumScorer::CountingConjunctionSumScorer(BooleanScorer2Ptr scorer, WeightPtr weight, double coord, Collection<ScorerPtr> scorers) : ConjunctionScorer(weight, coord, scorers)
    {
        _scorer = scorer;
        lastScoredDoc = -1;
        requiredNrMatchers = scorers.size();
        lastDocScore = std::numeric_limits<double>::quiet_NaN();
    }
    
    CountingConjunctionSumScorer::~CountingConjunctionSumScorer()
    {
    }
    
    double CountingConjunctionSumScorer::score()
    {
        int32_t doc = docID();
        if (doc >= lastScoredDoc)
        {
            if (doc > lastScoredDoc)
            {
                lastDocScore = ConjunctionScorer::score();
                lastScoredDoc = doc;
            }
            BooleanScorer2Ptr(_scorer)->coordinator->nrMatchers += requiredNrMatchers;
        }
        // All scorers match, so Similarity::getDefault() ConjunctionScorer::score() always has 1 as the 
        /// coordination factor.  Therefore the sum of the scores of the requiredScorers is used as score.
        return lastDocScore;
    }
}
