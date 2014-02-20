/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Scorer.h"
#include "TopScoreDocCollector.h"
#include "PositiveScoresOnlyCollector.h"
#include "TopDocs.h"
#include "ScoreDoc.h"

using namespace Lucene;

typedef LuceneTestFixture PositiveScoresOnlyCollectorTest;

namespace TestNegativeScores {

class SimpleScorer : public Scorer {
public:
    SimpleScorer(Collection<double> scores) : Scorer(SimilarityPtr()) {
        this->scores = scores;
        idx = -1;
    }

    virtual ~SimpleScorer() {
    }

public:
    int32_t idx;
    Collection<double> scores;

public:
    virtual double score() {
        return idx == scores.size() ? std::numeric_limits<double>::quiet_NaN() : scores[idx];
    }

    virtual int32_t docID() {
        return idx;
    }

    virtual int32_t nextDoc() {
        return ++idx != scores.size() ? idx : DocIdSetIterator::NO_MORE_DOCS;
    }

    virtual int32_t advance(int32_t target) {
        idx = target;
        return idx < scores.size() ? idx : DocIdSetIterator::NO_MORE_DOCS;
    }
};

}

TEST_F(PositiveScoresOnlyCollectorTest, testNegativeScores) {
    // The scores must have positive as well as negative values
    Collection<double> scores = Collection<double>::newInstance();
    scores.add(0.7767749);
    scores.add(-1.7839992);
    scores.add(8.9925785);
    scores.add(7.9608946);
    scores.add(-0.07948637);
    scores.add(2.6356435);
    scores.add(7.4950366);
    scores.add(7.1490803);
    scores.add(-8.108544);
    scores.add(4.961808f);
    scores.add(2.2423935);
    scores.add(-7.285586);
    scores.add(4.6699767);

    // The Top*Collectors previously filtered out documents with <= scores. This behaviour has changed.
    // This test checks that if PositiveOnlyScoresFilter wraps one of these collectors, documents with
    // <= 0 scores are indeed filtered.

    int32_t numPositiveScores = 0;
    for (int32_t i = 0; i < scores.size(); ++i) {
        if (scores[i] > 0) {
            ++numPositiveScores;
        }
    }

    ScorerPtr s = newLucene<TestNegativeScores::SimpleScorer>(scores);
    TopDocsCollectorPtr tdc = TopScoreDocCollector::create(scores.size(), true);
    CollectorPtr c = newLucene<PositiveScoresOnlyCollector>(tdc);
    c->setScorer(s);
    while (s->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        c->collect(0);
    }

    TopDocsPtr td = tdc->topDocs();
    Collection<ScoreDocPtr> sd = td->scoreDocs;
    EXPECT_EQ(numPositiveScores, td->totalHits);
    for (int32_t i = 0; i < sd.size(); ++i) {
        EXPECT_TRUE(sd[i]->score > 0);
    }
}
