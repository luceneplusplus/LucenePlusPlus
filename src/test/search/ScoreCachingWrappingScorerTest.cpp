/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Scorer.h"
#include "TopScoreDocCollector.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "ScoreCachingWrappingScorer.h"

using namespace Lucene;

typedef LuceneTestFixture ScoreCachingWrappingScorerTest;

namespace TestGetScores {

class SimpleScorer : public Scorer {
public:
    SimpleScorer(Collection<double> scores) : Scorer(SimilarityPtr()) {
        this->scores = scores;
        idx = 0;
        doc = -1;
    }

    virtual ~SimpleScorer() {
    }

public:
    int32_t idx;
    int32_t doc;
    Collection<double> scores;

public:
    virtual double score() {
        // Advance idx on purpose, so that consecutive calls to score will get different results.
        // This is to emulate computation of a score.  If ScoreCachingWrappingScorer is used, this
        // should not be called more than once per document.
        return idx == scores.size() ? std::numeric_limits<double>::quiet_NaN() : scores[idx++];
    }

    virtual int32_t docID() {
        return doc;
    }

    virtual int32_t nextDoc() {
        return ++doc < scores.size() ? doc : DocIdSetIterator::NO_MORE_DOCS;
    }

    virtual int32_t advance(int32_t target) {
        doc = target;
        return doc < scores.size() ? doc : DocIdSetIterator::NO_MORE_DOCS;
    }
};

DECLARE_SHARED_PTR(ScoreCachingCollector)

class ScoreCachingCollector : public Collector {
public:
    ScoreCachingCollector(int32_t numToCollect) {
        idx = 0;
        mscores = Collection<double>::newInstance(numToCollect);
    }

    virtual ~ScoreCachingCollector() {
    }

public:
    int32_t idx;
    ScorerPtr scorer;
    Collection<double> mscores;

public:
    virtual void collect(int32_t doc) {
        // just a sanity check to avoid IOOB.
        if (idx == mscores.size()) {
            return;
        }

        // just call score() a couple of times and record the score.
        mscores[idx] = scorer->score();
        mscores[idx] = scorer->score();
        mscores[idx] = scorer->score();
        ++idx;
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
    }

    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = newLucene<ScoreCachingWrappingScorer>(scorer);
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

TEST_F(ScoreCachingWrappingScorerTest, testGetScores) {
    Collection<double> scores = Collection<double>::newInstance();
    scores.add(0.7767749);
    scores.add(1.7839992);
    scores.add(8.9925785);
    scores.add(7.9608946);
    scores.add(0.07948637);
    scores.add(2.6356435);
    scores.add(7.4950366);
    scores.add(7.1490803);
    scores.add(8.108544);
    scores.add(4.961808);
    scores.add(2.2423935);
    scores.add(7.285586);
    scores.add(4.6699767);

    ScorerPtr s = newLucene<TestGetScores::SimpleScorer>(scores);
    TestGetScores::ScoreCachingCollectorPtr scc = newLucene<TestGetScores::ScoreCachingCollector>(scores.size());
    scc->setScorer(s);

    // We need to iterate on the scorer so that its doc() advances.
    int32_t doc;
    while ((doc = s->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
        scc->collect(doc);
    }

    for (int32_t i = 0; i < scores.size(); ++i) {
        EXPECT_EQ(scores[i], scc->mscores[i]);
    }
}
