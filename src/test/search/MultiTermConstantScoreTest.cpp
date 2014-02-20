/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTestRangeFilterFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "QueryUtils.h"
#include "Term.h"
#include "TermRangeQuery.h"
#include "MultiTermQuery.h"
#include "PrefixQuery.h"
#include "WildcardQuery.h"
#include "Collator.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "Scorer.h"
#include "Collector.h"
#include "BooleanQuery.h"

using namespace Lucene;

class MultiTermConstantScoreTest : public BaseTestRangeFilterFixture {
public:
    MultiTermConstantScoreTest() {
        Collection<String> data = newCollection<String>(
                                      L"A 1 2 3 4 5 6",
                                      L"Z       4 5 6",
                                      L"",
                                      L"B   2   4 5 6",
                                      L"Y     3   5 6",
                                      L"",
                                      L"C     3     6",
                                      L"X       4 5 6"
                                  );

        small = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(small, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        for (int32_t i = 0; i < data.size(); ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"all", L"all", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            if (!data[i].empty()) {
                doc->add(newLucene<Field>(L"data", data[i], Field::STORE_YES, Field::INDEX_ANALYZED));
            }
            writer->addDocument(doc);
        }

        writer->optimize();
        writer->close();
    }

    virtual ~MultiTermConstantScoreTest() {
    }

public:
    static const double SCORE_COMP_THRESH;

    DirectoryPtr small;

public:
    QueryPtr csrq(const String& f, const String& l, const String& h, bool il, bool ih) {
        TermRangeQueryPtr query = newLucene<TermRangeQuery>(f, l, h, il, ih);
        query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
        return query;
    }

    QueryPtr csrq(const String& f, const String& l, const String& h, bool il, bool ih, const RewriteMethodPtr& method) {
        TermRangeQueryPtr query = newLucene<TermRangeQuery>(f, l, h, il, ih);
        query->setRewriteMethod(method);
        return query;
    }

    QueryPtr csrq(const String& f, const String& l, const String& h, bool il, bool ih, const CollatorPtr& c) {
        TermRangeQueryPtr query = newLucene<TermRangeQuery>(f, l, h, il, ih, c);
        query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
        return query;
    }

    QueryPtr cspq(const TermPtr& prefix) {
        PrefixQueryPtr query = newLucene<PrefixQuery>(prefix);
        query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
        return query;
    }

    QueryPtr cswcq(const TermPtr& wild) {
        WildcardQueryPtr query = newLucene<WildcardQuery>(wild);
        query->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
        return query;
    }
};

/// threshold for comparing doubles
const double MultiTermConstantScoreTest::SCORE_COMP_THRESH = 1e-6f;

TEST_F(MultiTermConstantScoreTest, testBasics) {
    QueryUtils::check(csrq(L"data", L"1", L"6", true, true));
    QueryUtils::check(csrq(L"data", L"A", L"Z", true, true));
    QueryUtils::checkUnequal(csrq(L"data", L"1", L"6", true, true), csrq(L"data", L"A", L"Z", true, true));

    QueryUtils::check(cspq(newLucene<Term>(L"data", L"p*u?")));
    QueryUtils::checkUnequal(cspq(newLucene<Term>(L"data", L"pre*")), cspq(newLucene<Term>(L"data", L"pres*")));

    QueryUtils::check(cswcq(newLucene<Term>(L"data", L"p")));
    QueryUtils::checkUnequal(cswcq(newLucene<Term>(L"data", L"pre*n?t")), cswcq(newLucene<Term>(L"data", L"pr*t?j")));
}

TEST_F(MultiTermConstantScoreTest, testBasicsRngCollating) {
    CollatorPtr c = newLucene<Collator>(std::locale());
    QueryUtils::check(csrq(L"data", L"1", L"6", true, true, c));
    QueryUtils::check(csrq(L"data", L"A", L"Z", true, true, c));
    QueryUtils::checkUnequal(csrq(L"data", L"1", L"6", true, true, c), csrq(L"data", L"A", L"Z", true, true, c));
}

TEST_F(MultiTermConstantScoreTest, testEqualScores) {
    IndexReaderPtr reader = IndexReader::open(small, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    // some hits match more terms then others, score should be the same
    Collection<ScoreDocPtr> result = search->search(csrq(L"data", L"1", L"6", true, true), FilterPtr(), 1000)->scoreDocs;
    int32_t numHits = result.size();
    EXPECT_EQ(6, numHits);
    double score = result[0]->score;
    for (int32_t i = 1; i < numHits; ++i) {
        EXPECT_EQ(score, result[i]->score);
    }

    result = search->search(csrq(L"data", L"1", L"6", true, true, MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE()), FilterPtr(), 1000)->scoreDocs;
    numHits = result.size();
    EXPECT_EQ(6, numHits);
    for (int32_t i = 0; i < numHits; ++i) {
        EXPECT_EQ(score, result[i]->score);
    }
}

namespace TestBoost {

class TestCollector : public Collector {
public:
    TestCollector() {
        base = 0;
    }

    virtual ~TestCollector() {
    }

protected:
    int32_t base;
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(1.0, scorer->score());
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

TEST_F(MultiTermConstantScoreTest, testBoost) {
    IndexReaderPtr reader = IndexReader::open(small, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    // test for correct application of query normalization
    // must use a non score normalizing method for this.
    QueryPtr q = csrq(L"data", L"1", L"6", true, true);
    q->setBoost(100);

    search->search(q, FilterPtr(), newLucene<TestBoost::TestCollector>());

    // Ensure that boosting works to score one clause of a query higher than another.
    QueryPtr q1 = csrq(L"data", L"A", L"A", true, true); // matches document #0
    q1->setBoost(0.1);
    QueryPtr q2 = csrq(L"data", L"Z", L"Z", true, true); // matches document #1
    BooleanQueryPtr bq = newLucene<BooleanQuery>(true);
    bq->add(q1, BooleanClause::SHOULD);
    bq->add(q2, BooleanClause::SHOULD);

    Collection<ScoreDocPtr> hits = search->search(bq, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits[0]->doc);
    EXPECT_EQ(0, hits[1]->doc);
    EXPECT_TRUE(hits[0]->score > hits[1]->score);

    q1 = csrq(L"data", L"A", L"A", true, true, MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE()); // matches document #0
    q1->setBoost(0.1);
    q2 = csrq(L"data", L"Z", L"Z", true, true, MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE()); // matches document #1
    bq = newLucene<BooleanQuery>(true);
    bq->add(q1, BooleanClause::SHOULD);
    bq->add(q2, BooleanClause::SHOULD);

    hits = search->search(bq, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits[0]->doc);
    EXPECT_EQ(0, hits[1]->doc);
    EXPECT_TRUE(hits[0]->score > hits[1]->score);

    q1 = csrq(L"data", L"A", L"A", true, true); // matches document #0
    q1->setBoost(10.0);
    q2 = csrq(L"data", L"Z", L"Z", true, true); // matches document #1
    bq = newLucene<BooleanQuery>(true);
    bq->add(q1, BooleanClause::SHOULD);
    bq->add(q2, BooleanClause::SHOULD);

    hits = search->search(bq, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits[0]->doc);
    EXPECT_EQ(1, hits[1]->doc);
    EXPECT_TRUE(hits[0]->score > hits[1]->score);
}

TEST_F(MultiTermConstantScoreTest, testBooleanOrderUnAffected) {
    IndexReaderPtr reader = IndexReader::open(small, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    // first do a regular TermRangeQuery which uses term expansion so docs with more terms in range get higher scores

    QueryPtr rq = newLucene<TermRangeQuery>(L"data", L"1", L"4", true, true);

    Collection<ScoreDocPtr> expected = search->search(rq, FilterPtr(), 1000)->scoreDocs;
    int32_t numHits = expected.size();

    // now do a boolean where which also contains a ConstantScoreRangeQuery and make sure hte order is the same
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(rq, BooleanClause::MUST);
    q->add(csrq(L"data", L"1", L"6", true, true), BooleanClause::MUST);

    Collection<ScoreDocPtr> actual = search->search(q, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(numHits, actual.size());
    for (int32_t i = 0; i < numHits; ++i) {
        EXPECT_EQ(expected[i]->doc, actual[i]->doc);
    }
}

TEST_F(MultiTermConstantScoreTest, testRangeQueryId) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t medId = ((maxId - minId) / 2);

    String nullMin = StringUtils::toString(INT_MIN);
    String nullMax = StringUtils::toString(INT_MAX);

    String minIP = pad(minId);
    String maxIP = pad(maxId);
    String medIP = pad(medId);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    // test id, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(csrq(L"id", minIP, maxIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, true, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(csrq(L"id", medIP, maxIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(csrq(L"id", medIP, maxIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(csrq(L"id", minIP, medIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    result = search->search(csrq(L"id", minIP, medIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(csrq(L"id", minIP, nullMax, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", nullMin, maxIP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", minIP, nullMax, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", nullMin, maxIP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", medIP, maxIP, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(csrq(L"id", minIP, medIP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(csrq(L"id", minIP, minIP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", minIP, minIP, false, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", medIP, medIP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", medIP, medIP, false, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", maxIP, maxIP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", maxIP, maxIP, false, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", minIP, minIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", minIP, minIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", nullMin, minIP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", nullMin, minIP, false, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", maxIP, maxIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", maxIP, maxIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", maxIP, nullMax, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", maxIP, nullMax, true, false, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", medIP, medIP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", medIP, medIP, true, true, MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(MultiTermConstantScoreTest, testRangeQueryIdCollating) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t medId = ((maxId - minId) / 2);

    String nullMin = StringUtils::toString(INT_MIN);
    String nullMax = StringUtils::toString(INT_MAX);

    String minIP = pad(minId);
    String maxIP = pad(maxId);
    String medIP = pad(medId);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    CollatorPtr c = newLucene<Collator>(std::locale());

    // test id, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(csrq(L"id", minIP, maxIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", minIP, maxIP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(csrq(L"id", medIP, maxIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(csrq(L"id", minIP, medIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(csrq(L"id", minIP, nullMax, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", nullMin, maxIP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"id", minIP, nullMax, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", nullMin, maxIP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"id", medIP, maxIP, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(csrq(L"id", minIP, medIP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(csrq(L"id", minIP, minIP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(csrq(L"id", medIP, medIP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(csrq(L"id", maxIP, maxIP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"id", minIP, minIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"id", nullMin, minIP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", maxIP, maxIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"id", maxIP, nullMax, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"id", medIP, medIP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(MultiTermConstantScoreTest, testRangeQueryRand) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    String nullMin = StringUtils::toString(INT_MIN);
    String nullMax = StringUtils::toString(INT_MAX);

    String minRP = pad(signedIndex->minR);
    String maxRP = pad(signedIndex->maxR);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    // test extremes, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(csrq(L"rand", minRP, maxRP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    // unbounded

    result = search->search(csrq(L"rand", minRP, nullMax, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", nullMin, maxRP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", minRP, nullMax, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", nullMin, maxRP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    // very small sets

    result = search->search(csrq(L"rand", minRP, minRP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(csrq(L"rand", maxRP, maxRP, false, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"rand", minRP, minRP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"rand", nullMin, minRP, false, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"rand", maxRP, maxRP, true, true), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"rand", maxRP, nullMax, true, false), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(MultiTermConstantScoreTest, testRangeQueryRandCollating) {
    // using the unsigned index because collation seems to ignore hyphens
    IndexReaderPtr reader = IndexReader::open(unsignedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    String nullMin = StringUtils::toString(INT_MIN);
    String nullMax = StringUtils::toString(INT_MAX);

    String minRP = pad(unsignedIndex->minR);
    String maxRP = pad(unsignedIndex->maxR);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    CollatorPtr c = newLucene<Collator>(std::locale());

    // test extremes, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(csrq(L"rand", minRP, maxRP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", minRP, maxRP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    // unbounded

    result = search->search(csrq(L"rand", minRP, nullMax, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", nullMin, maxRP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(csrq(L"rand", minRP, nullMax, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(csrq(L"rand", nullMin, maxRP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    // very small sets

    result = search->search(csrq(L"rand", minRP, minRP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(csrq(L"rand", maxRP, maxRP, false, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(csrq(L"rand", minRP, minRP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"rand", nullMin, minRP, false, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(csrq(L"rand", maxRP, maxRP, true, true, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(csrq(L"rand", maxRP, nullMax, true, false, c), FilterPtr(), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}
