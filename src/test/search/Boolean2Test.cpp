/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "MockRAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexWriter.h"
#include "TopScoreDocCollector.h"
#include "TopFieldCollector.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "CheckHits.h"
#include "QueryParser.h"
#include "DefaultSimilarity.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "WildcardQuery.h"
#include "Sort.h"
#include "QueryUtils.h"
#include "PrefixQuery.h"
#include "Random.h"

using namespace Lucene;

/// Test BooleanQuery2 against BooleanQuery by overriding the standard query parser.
/// This also tests the scoring order of BooleanQuery.
class Boolean2Test : public LuceneTestFixture {
public:
    Boolean2Test() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        docFields = newCollection<String>(L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3", L"w1 xx w2 yy w3", L"w1 w3 xx w2 yy w3");
        for (int32_t i = 0; i < docFields.size(); ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(field, docFields[i], Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }

        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);

        // Make big index
        dir2 = newLucene<MockRAMDirectory>(directory);

        // First multiply small test index
        mulFactor = 1;
        int32_t docCount = 0;
        do {
            DirectoryPtr copy = newLucene<RAMDirectory>(dir2);
            IndexWriterPtr w = newLucene<IndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
            w->addIndexesNoOptimize(newCollection<DirectoryPtr>(copy));
            docCount = w->maxDoc();
            w->close();
            mulFactor *= 2;
        } while (docCount < 3000);

        IndexWriterPtr w = newLucene<IndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field2", L"xxx", Field::STORE_NO, Field::INDEX_ANALYZED));
        for (int32_t i = 0; i <NUM_EXTRA_DOCS / 2; ++i) {
            w->addDocument(doc);
        }
        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field2", L"big bad bug", Field::STORE_NO, Field::INDEX_ANALYZED));
        for (int32_t i = 0; i <NUM_EXTRA_DOCS / 2; ++i) {
            w->addDocument(doc);
        }
        // optimize to 1 segment
        w->optimize();
        reader = w->getReader();
        w->close();
        bigSearcher = newLucene<IndexSearcher>(reader);
    }

    virtual ~Boolean2Test() {
        reader->close();
        dir2->close();
    }

protected:
    IndexSearcherPtr searcher;
    IndexSearcherPtr bigSearcher;
    IndexReaderPtr reader;
    DirectoryPtr dir2;
    int32_t mulFactor;
    Collection<String> docFields;

public:
    static const int32_t NUM_EXTRA_DOCS;
    static const String field;

public:
    QueryPtr makeQuery(const String& queryText) {
        return newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, field, newLucene<WhitespaceAnalyzer>())->parse(queryText);
    }

    void queriesTest(const String& queryText, Collection<int32_t> expDocNrs) {
        QueryPtr query1 = makeQuery(queryText);
        TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(1000, false);
        searcher->search(query1, FilterPtr(), collector);
        Collection<ScoreDocPtr> hits1 = collector->topDocs()->scoreDocs;

        QueryPtr query2 = makeQuery(queryText); // there should be no need to parse again...
        collector = TopScoreDocCollector::create(1000, true);
        searcher->search(query2, FilterPtr(), collector);
        Collection<ScoreDocPtr> hits2 = collector->topDocs()->scoreDocs;

        EXPECT_EQ(mulFactor * collector->getTotalHits(), bigSearcher->search(query1, 1)->totalHits);

        CheckHits::checkHitsQuery(query2, hits1, hits2, expDocNrs);
    }

    /// Random rnd is passed in so that the exact same random query may be created more than once.
    BooleanQueryPtr randBoolQuery(const RandomPtr& rnd, bool allowMust, int32_t level, const String& field, Collection<String> vals) {
        BooleanQueryPtr current = newLucene<BooleanQuery>(rnd->nextInt() < 0);
        for (int32_t i = 0; i < rnd->nextInt(vals.size()) + 1; ++i) {
            int32_t qType = 0; // term query
            if (level > 0) {
                qType = rnd->nextInt(10);
            }
            QueryPtr q;
            if (qType < 3) {
                q = newLucene<TermQuery>(newLucene<Term>(field, vals[rnd->nextInt(vals.size())]));
            } else if (qType < 7) {
                q = newLucene<WildcardQuery>(newLucene<Term>(field, L"w*"));
            } else {
                q = randBoolQuery(rnd, allowMust, level - 1, field, vals);
            }

            int32_t r = rnd->nextInt(10);
            BooleanClause::Occur occur = BooleanClause::SHOULD;
            if (r < 2) {
                occur = BooleanClause::MUST_NOT;
            } else if (r < 5) {
                if (allowMust) {
                    occur = BooleanClause::MUST;
                } else {
                    occur = BooleanClause::SHOULD;
                }
            }

            current->add(q, occur);
        }
        return current;
    }
};

const String Boolean2Test::field = L"field";
const int32_t Boolean2Test::NUM_EXTRA_DOCS = 6000;

TEST_F(Boolean2Test, testQueries01) {
    String queryText = L"+w3 +xx";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries02) {
    String queryText = L"+w3 xx";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3, 1, 0);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries03) {
    String queryText = L"w3 xx";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3, 1, 0);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries04) {
    String queryText = L"w3 -xx";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(1, 0);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries05) {
    String queryText = L"+w3 -xx";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(1, 0);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries06) {
    String queryText = L"+w3 -xx -w5";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(1);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries07) {
    String queryText = L"-w3 -xx -w5";
    Collection<int32_t> expDocNrs = Collection<int32_t>::newInstance();
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries08) {
    String queryText = L"+w3 xx -w5";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3, 1);
    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testQueries09) {
    String queryText = L"+w3 +xx +w2 zz";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3);
    queriesTest(queryText, expDocNrs);
}

namespace TestQueries10 {

class OverlapSimilarity : public DefaultSimilarity {
public:
    virtual ~OverlapSimilarity() {
    }

public:
    virtual double coord(int32_t overlap, int32_t maxOverlap) {
        return (double)overlap / ((double)maxOverlap - 1.0);
    }
};

}

TEST_F(Boolean2Test, testQueries10) {
    String queryText = L"+w3 +xx +w2 zz";
    Collection<int32_t> expDocNrs = newCollection<int32_t>(2, 3);
    searcher->setSimilarity(newLucene<TestQueries10::OverlapSimilarity>());

    queriesTest(queryText, expDocNrs);
}

TEST_F(Boolean2Test, testRandomQueries) {
    RandomPtr rnd = newLucene<Random>(17);
    Collection<String> vals = newCollection<String>(L"w1", L"w2", L"w3", L"w4", L"w5", L"xx", L"yy", L"zzz");
    int32_t tot = 0;
    // increase number of iterations for more complete testing
    for (int32_t i = 0; i < 1000; ++i) {
        int32_t level = rnd->nextInt(3);
        BooleanQueryPtr q1 = randBoolQuery(rnd, rnd->nextInt() % 2 == 0, level, field, vals);

        // Can't sort by relevance since floating point numbers may not quite match up.
        SortPtr sort = Sort::INDEXORDER();
        QueryUtils::check(q1, searcher);

        TopFieldCollectorPtr collector = TopFieldCollector::create(sort, 1000, false, true, true, true);

        searcher->search(q1, FilterPtr(), collector);
        Collection<ScoreDocPtr> hits1 = collector->topDocs()->scoreDocs;

        collector = TopFieldCollector::create(sort, 1000, false, true, true, false);

        searcher->search(q1, FilterPtr(), collector);
        Collection<ScoreDocPtr> hits2 = collector->topDocs()->scoreDocs;
        tot += hits2.size();
        CheckHits::checkEqual(q1, hits1, hits2);

        BooleanQueryPtr q3 = newLucene<BooleanQuery>();
        q3->add(q1, BooleanClause::SHOULD);
        q3->add(newLucene<PrefixQuery>(newLucene<Term>(L"field2", L"b")), BooleanClause::SHOULD);
        TopDocsPtr hits4 = bigSearcher->search(q3, 1);
        EXPECT_EQ(mulFactor * collector->getTotalHits() + NUM_EXTRA_DOCS / 2, hits4->totalHits);
    }
}
