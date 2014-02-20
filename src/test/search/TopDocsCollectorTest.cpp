/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "KeywordAnalyzer.h"
#include "Document.h"
#include "TopDocsCollector.h"
#include "MatchAllDocsQuery.h"
#include "IndexSearcher.h"
#include "HitQueue.h"
#include "TopDocs.h"
#include "ScoreDoc.h"

using namespace Lucene;

class MyTopsDocCollector : public TopDocsCollector {
public:
    MyTopsDocCollector(int32_t size, Collection<double> scores) : TopDocsCollector(newLucene<HitQueue>(size, false)) {
        this->scores = scores;
        this->idx = 0;
        this->base = 0;
    }

    virtual ~MyTopsDocCollector() {
    }

protected:
    int32_t idx;
    int32_t base;
    Collection<double> scores;

protected:
    virtual TopDocsPtr newTopDocs(Collection<ScoreDocPtr> results, int32_t start) {
        if (!results) {
            return EMPTY_TOPDOCS();
        }

        double maxScore = std::numeric_limits<double>::quiet_NaN();
        if (start == 0) {
            maxScore = results[0]->score;
        } else {
            for (int32_t i = pq->size(); i > 1; --i) {
                pq->pop();
            }
            maxScore = boost::dynamic_pointer_cast<ScoreDoc>(pq->pop())->score;
        }
        return newLucene<TopDocs>(totalHits, results, maxScore);
    }

    virtual void collect(int32_t doc) {
        ++totalHits;
        pq->addOverflow(newLucene<ScoreDoc>(doc + base, scores[idx++]));
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual void setScorer(const ScorerPtr& scorer) {
        // Don't do anything. Assign scores in random
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

class TopDocsCollectorTest : public LuceneTestFixture {
public:
    TopDocsCollectorTest() {
        MAX_SCORE = 9.17561;

        // Scores array to be used by MyTopDocsCollector. If it is changed, MAX_SCORE must also change.
        const double _scores[] = {
            0.7767749, 1.7839992, 8.9925785, 7.9608946, 0.07948637, 2.6356435,
            7.4950366, 7.1490803, 8.108544, 4.961808, 2.2423935, 7.285586,
            4.6699767, 2.9655676, 6.953706, 5.383931, 6.9916306, 8.365894,
            7.888485, 8.723962, 3.1796896, 0.39971232, 1.3077754, 6.8489285,
            9.17561, 5.060466, 7.9793315, 8.601509, 4.1858315, 0.28146625
        };
        scores = Collection<double>::newInstance(_scores, _scores + SIZEOF_ARRAY(_scores));

        dir = newLucene<RAMDirectory>();

        // populate an index with 30 documents, this should be enough for the test.
        // The documents have no content - the test uses MatchAllDocsQuery().
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
        for (int32_t i = 0; i < 30; ++i) {
            writer->addDocument(newLucene<Document>());
        }
        writer->close();
    }

    virtual ~TopDocsCollectorTest() {
        dir->close();
    }

protected:
    DirectoryPtr dir;
    Collection<double> scores;
    double MAX_SCORE;

public:
    TopDocsCollectorPtr doSearch(int32_t numResults) {
        QueryPtr q = newLucene<MatchAllDocsQuery>();
        IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
        TopDocsCollectorPtr tdc = newLucene<MyTopsDocCollector>(numResults, scores);
        searcher->search(q, tdc);
        searcher->close();
        return tdc;
    }
};

TEST_F(TopDocsCollectorTest, testInvalidArguments) {
    int32_t numResults = 5;
    TopDocsCollectorPtr tdc = doSearch(numResults);

    // start < 0
    EXPECT_EQ(0, tdc->topDocs(-1)->scoreDocs.size());

    // start > pq.size()
    EXPECT_EQ(0, tdc->topDocs(numResults + 1)->scoreDocs.size());

    // start == pq.size()
    EXPECT_EQ(0, tdc->topDocs(numResults)->scoreDocs.size());

    // howMany < 0
    EXPECT_EQ(0, tdc->topDocs(0, -1)->scoreDocs.size());

    // howMany == 0
    EXPECT_EQ(0, tdc->topDocs(0, 0)->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testZeroResults) {
    TopDocsCollectorPtr tdc = newLucene<MyTopsDocCollector>(5, scores);
    EXPECT_EQ(0, tdc->topDocs(0, 1)->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testFirstResultsPage) {
    TopDocsCollectorPtr tdc = doSearch(15);
    EXPECT_EQ(10, tdc->topDocs(0, 10)->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testSecondResultsPages) {
    TopDocsCollectorPtr tdc = doSearch(15);

    // ask for more results than are available
    EXPECT_EQ(5, tdc->topDocs(10, 10)->scoreDocs.size());

    // ask for 5 results (exactly what there should be
    tdc = doSearch(15);
    EXPECT_EQ(5, tdc->topDocs(10, 5)->scoreDocs.size());

    // ask for less results than there are
    tdc = doSearch(15);
    EXPECT_EQ(4, tdc->topDocs(10, 4)->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testGetAllResults) {
    TopDocsCollectorPtr tdc = doSearch(15);
    EXPECT_EQ(15, tdc->topDocs()->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testGetResultsFromStart) {
    TopDocsCollectorPtr tdc = doSearch(15);
    // should bring all results
    EXPECT_EQ(15, tdc->topDocs(0)->scoreDocs.size());

    tdc = doSearch(15);
    // get the last 5 only.
    EXPECT_EQ(5, tdc->topDocs(10)->scoreDocs.size());
}

TEST_F(TopDocsCollectorTest, testMaxScore) {
    // ask for all results
    TopDocsCollectorPtr tdc = doSearch(15);
    TopDocsPtr td = tdc->topDocs();
    EXPECT_EQ(MAX_SCORE, td->maxScore);

    // ask for 5 last results
    tdc = doSearch(15);
    td = tdc->topDocs(10);
    EXPECT_EQ(MAX_SCORE, td->maxScore);
}

/// This does not test the PQ's correctness, but whether topDocs() implementations
/// return the results in decreasing score order.
TEST_F(TopDocsCollectorTest, testResultsOrder) {
    TopDocsCollectorPtr tdc = doSearch(15);
    Collection<ScoreDocPtr> sd = tdc->topDocs()->scoreDocs;

    EXPECT_EQ(MAX_SCORE, sd[0]->score);
    for (int32_t i = 1; i < sd.size(); ++i) {
        EXPECT_TRUE(sd[i - 1]->score >= sd[i]->score);
    }
}
