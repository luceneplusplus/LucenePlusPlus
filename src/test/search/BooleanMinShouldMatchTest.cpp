/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "QueryUtils.h"
#include "WildcardQuery.h"
#include "Random.h"

using namespace Lucene;

class BooleanMinShouldMatchTest : public LuceneTestFixture {
public:
    BooleanMinShouldMatchTest() {
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

        index = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(index, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
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

        r = IndexReader::open(index, true);
        s = newLucene<IndexSearcher>(r);
    }

    virtual ~BooleanMinShouldMatchTest() {
    }

public:
    DirectoryPtr index;
    IndexReaderPtr r;
    IndexSearcherPtr s;

public:
    void verifyNrHits(const QueryPtr& q, int32_t expected) {
        Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(expected, h.size());
        QueryUtils::check(q, s);
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

    void minNrCB(const RandomPtr& rnd, const BooleanQueryPtr& q) {
        Collection<BooleanClausePtr> c = q->getClauses();
        int32_t opt = 0;
        for (int32_t i = 0; i < c.size(); ++i) {
            if (c[i]->getOccur() == BooleanClause::SHOULD) {
                ++opt;
            }
        }
        q->setMinimumNumberShouldMatch(rnd->nextInt(opt + 2));
    }
};

TEST_F(BooleanMinShouldMatchTest, testAllOptional) {
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    for (int32_t i = 1; i <= 4; ++i) {
        q->add(newLucene<TermQuery>(newLucene<Term>(L"data", StringUtils::toString(i))), BooleanClause::SHOULD);
    }
    q->setMinimumNumberShouldMatch(2); // match at least two of 4
    verifyNrHits(q, 2);
}

TEST_F(BooleanMinShouldMatchTest, testOneReqAndSomeOptional) {
    // one required, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

    verifyNrHits(q, 5);
}

TEST_F(BooleanMinShouldMatchTest, testSomeReqAndSomeOptional) {
    // two required, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

    verifyNrHits(q, 5);
}

TEST_F(BooleanMinShouldMatchTest, testOneProhibAndSomeOptional) {
    // one prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testSomeProhibAndSomeOptional) {
    // two prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"C")), BooleanClause::MUST_NOT);

    q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testOneReqOneProhibAndSomeOptional) {
    // one required, one prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testSomeReqOneProhibAndSomeOptional) {
    // two required, one prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testOneReqSomeProhibAndSomeOptional) {
    // one required, two prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"C")), BooleanClause::MUST_NOT);

    q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testSomeReqSomeProhibAndSomeOptional) {
    // two required, two prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"C")), BooleanClause::MUST_NOT);

    q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testMinHigherThenNumOptional) {
    // two required, two prohibited, some optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"5")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"4")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST_NOT);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"1")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"C")), BooleanClause::MUST_NOT);

    q->setMinimumNumberShouldMatch(90); // 90 of 4 optional

    verifyNrHits(q, 0);
}

TEST_F(BooleanMinShouldMatchTest, testMinEqualToNumOptional) {
    // two required, two optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"6")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::SHOULD);

    q->setMinimumNumberShouldMatch(2); // 2 of 2 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testOneOptionalEqualToMin) {
    // two required, one optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"3")), BooleanClause::SHOULD);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::MUST);

    q->setMinimumNumberShouldMatch(1); // 1 of 1 optional

    verifyNrHits(q, 1);
}

TEST_F(BooleanMinShouldMatchTest, testNoOptionalButMin) {
    // two required, no optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);
    q->add(newLucene<TermQuery>(newLucene<Term>(L"data", L"2")), BooleanClause::MUST);

    q->setMinimumNumberShouldMatch(1); // 1 of 0 optional

    verifyNrHits(q, 0);
}

TEST_F(BooleanMinShouldMatchTest, testNoOptionalButMin2) {
    // one required, no optional
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"all", L"all")), BooleanClause::MUST);

    q->setMinimumNumberShouldMatch(1); // 1 of 0 optional

    verifyNrHits(q, 0);
}

TEST_F(BooleanMinShouldMatchTest, testRandomQueries) {
    RandomPtr rnd = newLucene<Random>(17);

    String field = L"data";
    Collection<String> vals = Collection<String>::newInstance();
    vals.add(L"1");
    vals.add(L"2");
    vals.add(L"3");
    vals.add(L"4");
    vals.add(L"5");
    vals.add(L"6");
    vals.add(L"A");
    vals.add(L"Z");
    vals.add(L"B");
    vals.add(L"Y");
    vals.add(L"Z");
    vals.add(L"X");
    vals.add(L"foo");
    int32_t maxLev = 4;

    // increase number of iterations for more complete testing
    for (int32_t i = 0; i < 1000; ++i) {
        int32_t lev = rnd->nextInt(maxLev);
        int32_t seed = rnd->nextInt();

        RandomPtr rndQuery = newLucene<Random>();
        rndQuery->setSeed(seed);
        BooleanQueryPtr q1 = randBoolQuery(rndQuery, true, lev, field, vals);
        rndQuery->setSeed(seed);
        BooleanQueryPtr q2 = randBoolQuery(rndQuery, true, lev, field, vals);

        // only set minimumNumberShouldMatch on the top level query since setting at a lower level can change the score.
        minNrCB(rnd, q2);

        // Can't use Hits because normalized scores will mess things up.
        // The non-sorting version of search() that returns TopDocs will not normalize scores.
        TopDocsPtr top1 = s->search(q1, FilterPtr(), 100);
        TopDocsPtr top2 = s->search(q2, FilterPtr(), 100);

        QueryUtils::check(q1, s);
        QueryUtils::check(q2, s);

        // The constrained query should be a superset to the unconstrained query.
        EXPECT_TRUE(top2->totalHits <= top1->totalHits);

        for (int32_t hit = 0; hit < top2->totalHits; ++hit) {
            int32_t id = top2->scoreDocs[hit]->doc;
            double score = top2->scoreDocs[hit]->score;
            bool found = false;
            // find this doc in other hits
            for (int32_t other = 0; other < top1->totalHits; ++other) {
                if (top1->scoreDocs[other]->doc == id) {
                    found = true;
                    double otherScore = top1->scoreDocs[other]->score;
                    // check if scores match
                    EXPECT_NEAR(otherScore, score, 1.0e-6f);
                }
            }

            // check if subset
            EXPECT_TRUE(found);
        }
    }
}
