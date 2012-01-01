/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ScoreCachingWrappingScorer.h"

namespace Lucene
{
    ScoreCachingWrappingScorer::ScoreCachingWrappingScorer(ScorerPtr scorer) : Scorer(scorer->getSimilarity())
    {
        this->curDoc = -1;
        this->curScore = 0.0;
        this->scorer = scorer;
    }

    ScoreCachingWrappingScorer::~ScoreCachingWrappingScorer()
    {
    }

    bool ScoreCachingWrappingScorer::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        return scorer->score(collector, max, firstDocID);
    }

    SimilarityPtr ScoreCachingWrappingScorer::getSimilarity()
    {
        return scorer->getSimilarity();
    }

    double ScoreCachingWrappingScorer::score()
    {
        int32_t doc = scorer->docID();
        if (doc != curDoc)
        {
            curScore = scorer->score();
            curDoc = doc;
        }
        return curScore;
    }

    int32_t ScoreCachingWrappingScorer::docID()
    {
        return scorer->docID();
    }

    int32_t ScoreCachingWrappingScorer::nextDoc()
    {
        return scorer->nextDoc();
    }

    void ScoreCachingWrappingScorer::score(CollectorPtr collector)
    {
        scorer->score(collector);
    }

    int32_t ScoreCachingWrappingScorer::advance(int32_t target)
    {
        return scorer->advance(target);
    }
}
