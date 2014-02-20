/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "NumericRangeQuery.h"
#include "NumericRangeFilter.h"
#include "TopDocs.h"
#include "MultiTermQuery.h"
#include "Sort.h"
#include "MatchAllDocsQuery.h"
#include "Document.h"
#include "BooleanQuery.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "NumericField.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopFieldDocs.h"
#include "DocIdSet.h"
#include "Random.h"
#include "NumericUtils.h"
#include "TermRangeQuery.h"
#include "SortField.h"
#include "QueryUtils.h"
#include "FilteredTermEnum.h"
#include "Term.h"

using namespace Lucene;

class NumericRangeQuery64Test : public LuceneTestFixture {
public:
    NumericRangeQuery64Test() {
        static bool setupRequired = true;
        if (setupRequired) {
            setup();
            setupRequired = false;
        }
    }

    virtual ~NumericRangeQuery64Test() {
    }

protected:
    // distance of entries
    static const int64_t distance;

    // shift the starting of the values to the left, to also have negative values
    static const int64_t startOffset;

    // number of docs to generate for testing
    static const int32_t noDocs;

    static RAMDirectoryPtr directory;
    static IndexSearcherPtr searcher;

protected:
    /// One-time setup to initialise static members
    void setup() {
        // set the theoretical maximum term count for 8bit (see docs for the number)
        BooleanQuery::setMaxClauseCount(7 * 255 * 2 + 255);

        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

        NumericFieldPtr field8 = newLucene<NumericField>(L"field8", 8, Field::STORE_YES, true);
        NumericFieldPtr field6 = newLucene<NumericField>(L"field6", 6, Field::STORE_YES, true);
        NumericFieldPtr field4 = newLucene<NumericField>(L"field4", 4, Field::STORE_YES, true);
        NumericFieldPtr field2 = newLucene<NumericField>(L"field2", 2, Field::STORE_YES, true);
        NumericFieldPtr fieldNoTrie = newLucene<NumericField>(L"field" + StringUtils::toString(INT_MAX), INT_MAX, Field::STORE_YES, true);
        NumericFieldPtr ascfield8 = newLucene<NumericField>(L"ascfield8", 8, Field::STORE_NO, true);
        NumericFieldPtr ascfield6 = newLucene<NumericField>(L"ascfield6", 6, Field::STORE_NO, true);
        NumericFieldPtr ascfield4 = newLucene<NumericField>(L"ascfield4", 4, Field::STORE_NO, true);
        NumericFieldPtr ascfield2 = newLucene<NumericField>(L"ascfield2", 2, Field::STORE_NO, true);

        DocumentPtr doc = newLucene<Document>();

        // add fields, that have a distance to test general functionality
        doc->add(field8);
        doc->add(field6);
        doc->add(field4);
        doc->add(field2);
        doc->add(fieldNoTrie);

        // add ascending fields with a distance of 1, beginning at -noDocs/2 to test the correct splitting of range and inclusive/exclusive
        doc->add(ascfield8);
        doc->add(ascfield6);
        doc->add(ascfield4);
        doc->add(ascfield2);

        // Add a series of noDocs docs with increasing int values
        for (int32_t l = 0; l < noDocs; ++l) {
            int64_t val = distance * l + startOffset;
            field8->setLongValue(val);
            field6->setLongValue(val);
            field4->setLongValue(val);
            field2->setLongValue(val);
            fieldNoTrie->setLongValue(val);

            val = l - (noDocs / 2);
            ascfield8->setLongValue(val);
            ascfield6->setLongValue(val);
            ascfield4->setLongValue(val);
            ascfield2->setLongValue(val);
            writer->addDocument(doc);
        }

        writer->optimize();
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }

public:
    /// test for both constant score and boolean query, the other tests only use the constant score mode
    void testRange(int32_t precisionStep) {
        String field = L"field" + StringUtils::toString(precisionStep);
        int32_t count = 3000;
        int64_t lower = (distance * 3 / 2) + startOffset;
        int64_t upper = lower + count * distance + (distance / 3);

        NumericRangeQueryPtr q = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, true);
        NumericRangeFilterPtr f = NumericRangeFilter::newLongRange(field, precisionStep, lower, upper, true, true);
        int32_t lastTerms = 0;
        for (uint8_t i = 0; i < 3; ++i) {
            TopDocsPtr topDocs;
            int32_t terms;
            String type;
            q->clearTotalNumberOfTerms();
            f->clearTotalNumberOfTerms();
            switch (i) {
            case 0:
                type = L" (constant score filter rewrite)";
                q->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
                topDocs = searcher->search(q, FilterPtr(), noDocs, Sort::INDEXORDER());
                terms = q->getTotalNumberOfTerms();
                break;
            case 1:
                type = L" (constant score boolean rewrite)";
                q->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE());
                topDocs = searcher->search(q, FilterPtr(), noDocs, Sort::INDEXORDER());
                terms = q->getTotalNumberOfTerms();
                break;
            case 2:
                type = L" (filter)";
                topDocs = searcher->search(newLucene<MatchAllDocsQuery>(), f, noDocs, Sort::INDEXORDER());
                terms = f->getTotalNumberOfTerms();
                break;
            default:
                return;
            }
            // std::cout << "Found " << terms << " distinct terms in range for field '" << field << "'" << type << ".";
            Collection<ScoreDocPtr> sd = topDocs->scoreDocs;
            EXPECT_TRUE(sd);
            EXPECT_EQ(count, sd.size());
            DocumentPtr doc = searcher->doc(sd[0]->doc);
            EXPECT_EQ(StringUtils::toString(2 * distance + startOffset), doc->get(field));
            doc = searcher->doc(sd[sd.size() - 1]->doc);
            EXPECT_EQ(StringUtils::toString((1 + count) * distance + startOffset), doc->get(field));
            if (i > 0) {
                EXPECT_EQ(lastTerms, terms);
            }
            lastTerms = terms;
        }
    }

    void testLeftOpenRange(int32_t precisionStep) {
        String field = L"field" + StringUtils::toString(precisionStep);
        int32_t count = 3000;
        // int32_t count = 10;
        int64_t upper = (count - 1) * distance + (distance / 3) + startOffset;
        NumericRangeQueryPtr q = NumericRangeQuery::newLongRange(field, precisionStep, LLONG_MIN, upper, true, true);
        TopDocsPtr topDocs = searcher->search(q, FilterPtr(), noDocs, Sort::INDEXORDER());
        Collection<ScoreDocPtr> sd = topDocs->scoreDocs;
        EXPECT_TRUE(sd);
        EXPECT_EQ(count, sd.size());
        DocumentPtr doc = searcher->doc(sd[0]->doc);
        EXPECT_EQ(StringUtils::toString(startOffset), doc->get(field));
        doc = searcher->doc(sd[sd.size() - 1]->doc);
        EXPECT_EQ(StringUtils::toString((count - 1) * distance + startOffset), doc->get(field));
    }

    void testRightOpenRange(int32_t precisionStep) {
        String field = L"field" + StringUtils::toString(precisionStep);
        int32_t count = 3000;
        int64_t lower = (count - 1) * distance + (distance / 3) + startOffset;
        NumericRangeQueryPtr q = NumericRangeQuery::newLongRange(field, precisionStep, lower, LLONG_MAX, true, true);
        TopDocsPtr topDocs = searcher->search(q, FilterPtr(), noDocs, Sort::INDEXORDER());
        Collection<ScoreDocPtr> sd = topDocs->scoreDocs;
        EXPECT_TRUE(sd);
        EXPECT_EQ(noDocs - count, sd.size());
        DocumentPtr doc = searcher->doc(sd[0]->doc);
        EXPECT_EQ(StringUtils::toString(count * distance + startOffset), doc->get(field));
        doc = searcher->doc(sd[sd.size() - 1]->doc);
        EXPECT_EQ(StringUtils::toString((noDocs - 1) * distance + startOffset), doc->get(field));
    }

    void testRandomTrieAndClassicRangeQuery(int32_t precisionStep) {
        RandomPtr rnd = newLucene<Random>();
        String field = L"field" + StringUtils::toString(precisionStep);
        int32_t termCountT = 0;
        int32_t termCountC = 0;
        for (int32_t i = 0; i < 50; ++i) {
            int64_t lower = (int64_t)(rnd->nextDouble() * noDocs * distance) + startOffset;
            int64_t upper = (int64_t)(rnd->nextDouble() * noDocs * distance) + startOffset;
            if (lower > upper) {
                std::swap(lower, upper);
            }
            // test inclusive range
            NumericRangeQueryPtr tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, true);
            TermRangeQueryPtr cq = newLucene<TermRangeQuery>(field, NumericUtils::longToPrefixCoded(lower), NumericUtils::longToPrefixCoded(upper), true, true);
            TopDocsPtr tTopDocs = searcher->search(tq, 1);
            TopDocsPtr cTopDocs = searcher->search(cq, 1);
            EXPECT_EQ(cTopDocs->totalHits, tTopDocs->totalHits);
            termCountT += tq->getTotalNumberOfTerms();
            termCountC += cq->getTotalNumberOfTerms();
            // test exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, false, false);
            cq = newLucene<TermRangeQuery>(field, NumericUtils::longToPrefixCoded(lower), NumericUtils::longToPrefixCoded(upper), false, false);
            tTopDocs = searcher->search(tq, 1);
            cTopDocs = searcher->search(cq, 1);
            EXPECT_EQ(cTopDocs->totalHits, tTopDocs->totalHits);
            termCountT += tq->getTotalNumberOfTerms();
            termCountC += cq->getTotalNumberOfTerms();
            // test left exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, false, true);
            cq = newLucene<TermRangeQuery>(field, NumericUtils::longToPrefixCoded(lower), NumericUtils::longToPrefixCoded(upper), false, true);
            tTopDocs = searcher->search(tq, 1);
            cTopDocs = searcher->search(cq, 1);
            EXPECT_EQ(cTopDocs->totalHits, tTopDocs->totalHits);
            termCountT += tq->getTotalNumberOfTerms();
            termCountC += cq->getTotalNumberOfTerms();
            // test right exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, false);
            cq = newLucene<TermRangeQuery>(field, NumericUtils::longToPrefixCoded(lower), NumericUtils::longToPrefixCoded(upper), true, false);
            tTopDocs = searcher->search(tq, 1);
            cTopDocs = searcher->search(cq, 1);
            EXPECT_EQ(cTopDocs->totalHits, tTopDocs->totalHits);
            termCountT += tq->getTotalNumberOfTerms();
            termCountC += cq->getTotalNumberOfTerms();
        }
        if (precisionStep == INT_MAX) {
            EXPECT_EQ(termCountT, termCountC);
        }
    }

    void testRangeSplit(int32_t precisionStep) {
        RandomPtr rnd = newLucene<Random>();
        String field = L"ascfield" + StringUtils::toString(precisionStep);
        // 50 random tests
        for (int32_t i = 0; i < 50; ++i) {
            int64_t lower = (int64_t)(rnd->nextDouble() * noDocs - noDocs / 2.0);
            int64_t upper = (int64_t)(rnd->nextDouble() * noDocs - noDocs / 2.0);
            if (lower > upper) {
                std::swap(lower, upper);
            }
            // test inclusive range
            QueryPtr tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, true);
            TopDocsPtr tTopDocs = searcher->search(tq, 1);
            EXPECT_EQ(upper - lower + 1, tTopDocs->totalHits);
            // test exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, false, false);
            tTopDocs = searcher->search(tq, 1);
            EXPECT_EQ(std::max(upper - lower - 1, (int64_t)0), tTopDocs->totalHits);
            // test left exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, false, true);
            tTopDocs = searcher->search(tq, 1);
            EXPECT_EQ(upper - lower, tTopDocs->totalHits);
            // test right exclusive range
            tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, false);
            tTopDocs = searcher->search(tq, 1);
            EXPECT_EQ(upper - lower, tTopDocs->totalHits);
        }
    }

    void testDoubleRange(int32_t precisionStep) {
        String field = L"ascfield" + StringUtils::toString(precisionStep);
        int64_t lower = -1000;
        int64_t upper = +2000;

        QueryPtr tq = NumericRangeQuery::newDoubleRange(field, precisionStep, NumericUtils::sortableLongToDouble(lower), NumericUtils::sortableLongToDouble(upper), true, true);
        TopDocsPtr tTopDocs = searcher->search(tq, 1);
        EXPECT_EQ(upper - lower + 1, tTopDocs->totalHits);

        FilterPtr tf = NumericRangeFilter::newDoubleRange(field, precisionStep, NumericUtils::sortableLongToDouble(lower), NumericUtils::sortableLongToDouble(upper), true, true);
        tTopDocs = searcher->search(newLucene<MatchAllDocsQuery>(), tf, 1);
        EXPECT_EQ(upper - lower + 1, tTopDocs->totalHits);
    }

    void testSorting(int32_t precisionStep) {
        RandomPtr rnd = newLucene<Random>();
        String field = L"field" + StringUtils::toString(precisionStep);
        // 10 random tests, the index order is ascending, so using a reverse sort field should return descending documents
        for (int32_t i = 0; i < 10; ++i) {
            int64_t lower = (int64_t)(rnd->nextDouble() * noDocs * distance) + startOffset;
            int64_t upper = (int64_t)(rnd->nextDouble() * noDocs * distance) + startOffset;
            if (lower > upper) {
                std::swap(lower, upper);
            }
            QueryPtr tq = NumericRangeQuery::newLongRange(field, precisionStep, lower, upper, true, true);
            TopDocsPtr topDocs = searcher->search(tq, FilterPtr(), noDocs, newLucene<Sort>(newLucene<SortField>(field, SortField::LONG, true)));
            if (topDocs->totalHits == 0) {
                continue;
            }
            Collection<ScoreDocPtr> sd = topDocs->scoreDocs;
            EXPECT_TRUE(sd);
            int64_t last = StringUtils::toInt(searcher->doc(sd[0]->doc)->get(field));
            for (int32_t j = 1; j < sd.size(); ++j) {
                int64_t act = StringUtils::toLong(searcher->doc(sd[j]->doc)->get(field));
                EXPECT_TRUE(last > act);
                last = act;
            }
        }
    }
};

// distance of entries
const int64_t NumericRangeQuery64Test::distance = 66666;

// shift the starting of the values to the left, to also have negative values
const int64_t NumericRangeQuery64Test::startOffset = (int64_t)-1 << 31;

// number of docs to generate for testing
const int32_t NumericRangeQuery64Test::noDocs = 10000;

RAMDirectoryPtr NumericRangeQuery64Test::directory;
IndexSearcherPtr NumericRangeQuery64Test::searcher;

TEST_F(NumericRangeQuery64Test, testRange_8bit) {
    testRange(8);
}

TEST_F(NumericRangeQuery64Test, testRange_6bit) {
    testRange(6);
}

TEST_F(NumericRangeQuery64Test, testRange_4bit) {
    testRange(4);
}

TEST_F(NumericRangeQuery64Test, testRange_2bit) {
    testRange(2);
}

TEST_F(NumericRangeQuery64Test, testInverseRange) {
    NumericRangeFilterPtr f = NumericRangeFilter::newLongRange(L"field8", 8, 1000, -1000, true, true);
    EXPECT_EQ(f->getDocIdSet(searcher->getIndexReader()), DocIdSet::EMPTY_DOCIDSET());
    f = NumericRangeFilter::newLongRange(L"field8", 8, LLONG_MAX, LLONG_MIN, false, false);
    EXPECT_EQ(f->getDocIdSet(searcher->getIndexReader()), DocIdSet::EMPTY_DOCIDSET());
    f = NumericRangeFilter::newLongRange(L"field8", 8, LLONG_MIN, LLONG_MIN, false, false);
    EXPECT_EQ(f->getDocIdSet(searcher->getIndexReader()), DocIdSet::EMPTY_DOCIDSET());
}

TEST_F(NumericRangeQuery64Test, testOneMatchQuery) {
    NumericRangeQueryPtr q = NumericRangeQuery::newLongRange(L"ascfield8", 8, 1000, 1000, true, true);
    EXPECT_EQ(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE(), q->getRewriteMethod());
    TopDocsPtr topDocs = searcher->search(q, noDocs);
    Collection<ScoreDocPtr> sd = topDocs->scoreDocs;
    EXPECT_TRUE(sd);
    EXPECT_EQ(1, sd.size());
}

TEST_F(NumericRangeQuery64Test, testLeftOpenRange_8bit) {
    testLeftOpenRange(8);
}

TEST_F(NumericRangeQuery64Test, testLeftOpenRange_6bit) {
    testLeftOpenRange(6);
}

TEST_F(NumericRangeQuery64Test, testLeftOpenRange_4bit) {
    testLeftOpenRange(4);
}

TEST_F(NumericRangeQuery64Test, testLeftOpenRange_2bit) {
    testLeftOpenRange(2);
}

TEST_F(NumericRangeQuery64Test, testRightOpenRange_8bit) {
    testRightOpenRange(8);
}

TEST_F(NumericRangeQuery64Test, testRightOpenRange_6bit) {
    testRightOpenRange(6);
}

TEST_F(NumericRangeQuery64Test, testRightOpenRange_4bit) {
    testRightOpenRange(4);
}

TEST_F(NumericRangeQuery64Test, testRightOpenRange_2bit) {
    testRightOpenRange(2);
}

TEST_F(NumericRangeQuery64Test, testRandomTrieAndClassicRangeQuery_8bit) {
    testRandomTrieAndClassicRangeQuery(8);
}

TEST_F(NumericRangeQuery64Test, testRandomTrieAndClassicRangeQuery_6bit) {
    testRandomTrieAndClassicRangeQuery(6);
}

TEST_F(NumericRangeQuery64Test, testRandomTrieAndClassicRangeQuery_4bit) {
    testRandomTrieAndClassicRangeQuery(4);
}

TEST_F(NumericRangeQuery64Test, testRandomTrieAndClassicRangeQuery_2bit) {
    testRandomTrieAndClassicRangeQuery(2);
}

TEST_F(NumericRangeQuery64Test, testRandomTrieAndClassicRangeQuery_NoTrie) {
    testRandomTrieAndClassicRangeQuery(INT_MAX);
}

TEST_F(NumericRangeQuery64Test, testRangeSplit_8bit) {
    testRangeSplit(8);
}

TEST_F(NumericRangeQuery64Test, testRangeSplit_6bit) {
    testRangeSplit(6);
}

TEST_F(NumericRangeQuery64Test, testRangeSplit_4bit) {
    testRangeSplit(4);
}

TEST_F(NumericRangeQuery64Test, testRangeSplit_2bit) {
    testRangeSplit(2);
}

TEST_F(NumericRangeQuery64Test, testDoubleRange_8bit) {
    testDoubleRange(8);
}

TEST_F(NumericRangeQuery64Test, testDoubleRange_6bit) {
    testDoubleRange(6);
}

TEST_F(NumericRangeQuery64Test, testDoubleRange_4bit) {
    testDoubleRange(4);
}

TEST_F(NumericRangeQuery64Test, testDoubleRange_2bit) {
    testDoubleRange(2);
}

TEST_F(NumericRangeQuery64Test, testSorting_8bit) {
    testSorting(8);
}

TEST_F(NumericRangeQuery64Test, testSorting_6bit) {
    testSorting(6);
}

TEST_F(NumericRangeQuery64Test, testSorting_4bit) {
    testSorting(4);
}

TEST_F(NumericRangeQuery64Test, testSorting_2bit) {
    testSorting(2);
}

TEST_F(NumericRangeQuery64Test, testEqualsAndHash) {
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test1", 4, 10, 20, true, true));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test2", 4, 10, 20, false, true));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test3", 4, 10, 20, true, false));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test4", 4, 10, 20, false, false));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test5", 4, 10, LLONG_MAX, true, true));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test6", 4, LLONG_MIN, 20, true, true));
    QueryUtils::checkHashEquals(NumericRangeQuery::newLongRange(L"test7", 4, LLONG_MIN, LLONG_MAX, true, true));
    QueryUtils::checkEqual(NumericRangeQuery::newLongRange(L"test8", 4, 10, 20, true, true), NumericRangeQuery::newLongRange(L"test8", 4, 10, 20, true, true));
    QueryUtils::checkUnequal(NumericRangeQuery::newLongRange(L"test9", 4, 10, 20, true, true), NumericRangeQuery::newLongRange(L"test9", 8, 10, 20, true, true));
    QueryUtils::checkUnequal(NumericRangeQuery::newLongRange(L"test10a", 4, 10, 20, true, true), NumericRangeQuery::newLongRange(L"test10b", 4, 10, 20, true, true));
    QueryUtils::checkUnequal(NumericRangeQuery::newLongRange(L"test11", 4, 10, 20, true, true), NumericRangeQuery::newLongRange(L"test11", 4, 20, 10, true, true));
    QueryUtils::checkUnequal(NumericRangeQuery::newLongRange(L"test12", 4, 10, 20, true, true), NumericRangeQuery::newLongRange(L"test12", 4, 10, 20, false, true));
    QueryUtils::checkUnequal(NumericRangeQuery::newLongRange(L"test13", 4, 10, 20, true, true), NumericRangeQuery::newDoubleRange(L"test13", 4, 10.0, 20.0, true, true));
}
