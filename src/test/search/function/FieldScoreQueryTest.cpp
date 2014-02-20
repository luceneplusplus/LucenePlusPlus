/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "FunctionFixture.h"
#include "IndexSearcher.h"
#include "FieldScoreQuery.h"
#include "QueryUtils.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "Document.h"
#include "IndexReader.h"
#include "ValueSource.h"
#include "DocValues.h"
#include "VariantUtils.h"

using namespace Lucene;

/// Tests here create an index with a few documents, each having an int value indexed
/// field and a double value indexed field.  The values of these fields are later used
/// for scoring.
///
/// The rank tests use Hits to verify that docs are ordered (by score) as expected.
///
/// The exact score tests use TopDocs top to verify the exact score.
class FieldScoreQueryTest : public FunctionFixture {
public:
    FieldScoreQueryTest() : FunctionFixture(true) {
    }

    virtual ~FieldScoreQueryTest() {
    }

public:
    /// Test that FieldScoreQuery returns docs in expected order.
    void doTestRank(const String& field, FieldScoreQuery::Type tp) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        QueryPtr q = newLucene<FieldScoreQuery>(field,tp);

        QueryUtils::check(q, s);
        Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(N_DOCS, h.size());
        String prevID = L"ID" + StringUtils::toString(N_DOCS + 1); // greater than all ids of docs in this test
        for (int32_t i = 0; i < h.size(); ++i) {
            String resID = s->doc(h[i]->doc)->get(ID_FIELD);
            EXPECT_TRUE(resID.compare(prevID) < 0);
            prevID = resID;
        }
    }

    /// Test that FieldScoreQuery returns docs with expected score.
    void doTestExactScore(const String& field, FieldScoreQuery::Type tp) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        QueryPtr q = newLucene<FieldScoreQuery>(field, tp);
        TopDocsPtr td = s->search(q, FilterPtr(), 1000);
        EXPECT_EQ(N_DOCS, td->totalHits);
        Collection<ScoreDocPtr> sd = td->scoreDocs;
        for (int32_t i = 0; i < sd.size(); ++i) {
            double score = sd[i]->score;
            String id = s->getIndexReader()->document(sd[i]->doc)->get(ID_FIELD);
            double expectedScore = expectedFieldScore(id); // "ID7" --> 7.0
            EXPECT_NEAR(expectedScore, score, TEST_SCORE_TOLERANCE_DELTA);
        }
    }

    /// Test that values loaded for FieldScoreQuery are cached properly and consumes
    /// the proper RAM resources.
    void doTestCaching(const String& field, FieldScoreQuery::Type tp) {
        // prepare expected array types for comparison
        HashMap<FieldScoreQuery::Type, CollectionValue> expectedArrayTypes = HashMap<FieldScoreQuery::Type, CollectionValue>::newInstance();
        expectedArrayTypes.put(FieldScoreQuery::BYTE, Collection<uint8_t>::newInstance());
        expectedArrayTypes.put(FieldScoreQuery::INT, Collection<int32_t>::newInstance());
        expectedArrayTypes.put(FieldScoreQuery::DOUBLE, Collection<double>::newInstance());

        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        Collection<CollectionValue> innerArray = Collection<CollectionValue>::newInstance(s->getIndexReader()->getSequentialSubReaders().size());

        bool warned = false; // print warning once.
        for (int32_t i = 0; i < 10; ++i) {
            FieldScoreQueryPtr q = newLucene<FieldScoreQuery>(field, tp);
            Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
            EXPECT_EQ(N_DOCS, h.size());
            Collection<IndexReaderPtr> readers = s->getIndexReader()->getSequentialSubReaders();
            for (int32_t j = 0; j < readers.size(); ++j) {
                IndexReaderPtr reader = readers[j];
                try {
                    if (i == 0) {
                        innerArray[j] = q->valSrc->getValues(reader)->getInnerArray();
                        EXPECT_TRUE(VariantUtils::equalsType(innerArray[j], expectedArrayTypes.get(tp)));
                    } else {
                        EXPECT_TRUE(VariantUtils::equals(innerArray[j], q->valSrc->getValues(reader)->getInnerArray()));
                    }
                } catch (UnsupportedOperationException&) {
                    if (!warned) {
                        // std::cout << "WARNING: Cannot fully test values of " << StringUtils::toUTF8(q->toString());
                        warned = true;
                    }
                }
            }
        }

        // verify new values are reloaded (not reused) for a new reader
        s = newLucene<IndexSearcher>(dir, true);
        FieldScoreQueryPtr q = newLucene<FieldScoreQuery>(field, tp);
        Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(N_DOCS, h.size());
        Collection<IndexReaderPtr> readers = s->getIndexReader()->getSequentialSubReaders();
        for (int32_t j = 0; j < readers.size(); ++j) {
            IndexReaderPtr reader = readers[j];
            try {
                EXPECT_TRUE(!equalCollectionValues(innerArray[j], q->valSrc->getValues(reader)->getInnerArray()));
            } catch (UnsupportedOperationException&) {
                if (!warned) {
                    // std::cout << "WARNING: Cannot fully test values of " << StringUtils::toUTF8(q->toString());
                    warned = true;
                }
            }
        }
    }
};

/// Test that FieldScoreQuery of Type.BYTE returns docs in expected order.
TEST_F(FieldScoreQueryTest, testRankByte) {
    // INT field values are small enough to be parsed as byte
    doTestRank(INT_FIELD, FieldScoreQuery::BYTE);
}

/// Test that FieldScoreQuery of Type.INT returns docs in expected order.
TEST_F(FieldScoreQueryTest, testRankInt) {
    doTestRank(INT_FIELD, FieldScoreQuery::INT);
}

/// Test that FieldScoreQuery of Type.DOUBLE returns docs in expected order.
TEST_F(FieldScoreQueryTest, testRankDouble) {
    // INT field can be parsed as double
    doTestRank(INT_FIELD, FieldScoreQuery::DOUBLE);
    // same values, but in double format
    doTestRank(DOUBLE_FIELD, FieldScoreQuery::DOUBLE);
}

/// Test that FieldScoreQuery of Type.BYTE returns the expected scores.
TEST_F(FieldScoreQueryTest, testExactScoreByte) {
    // INT field values are small enough to be parsed as byte
    doTestExactScore(INT_FIELD, FieldScoreQuery::BYTE);
}

/// Test that FieldScoreQuery of Type.INT returns the expected scores.
TEST_F(FieldScoreQueryTest, testExactScoreInt) {
    // INT field values are small enough to be parsed as byte
    doTestExactScore(INT_FIELD, FieldScoreQuery::INT);
}

/// Test that FieldScoreQuery of Type.DOUBLE returns the expected scores.
TEST_F(FieldScoreQueryTest, testExactScoreDouble) {
    // INT field can be parsed as double
    doTestExactScore(INT_FIELD, FieldScoreQuery::DOUBLE);
    // same values, but in double format
    doTestExactScore(DOUBLE_FIELD, FieldScoreQuery::DOUBLE);
}

/// Test that FieldScoreQuery of Type.BYTE caches/reuses loaded values and consumes
/// the proper RAM resources.
TEST_F(FieldScoreQueryTest, testCachingByte) {
    // INT field values are small enough to be parsed as byte
    doTestCaching(INT_FIELD, FieldScoreQuery::BYTE);
}

/// Test that FieldScoreQuery of Type.INT caches/reuses loaded values and consumes
/// the proper RAM resources.
TEST_F(FieldScoreQueryTest, testCachingInt) {
    // INT field values are small enough to be parsed as byte
    doTestCaching(INT_FIELD, FieldScoreQuery::INT);
}

/// Test that FieldScoreQuery of Type.DOUBLE caches/reuses loaded values and consumes
/// the proper RAM resources.
TEST_F(FieldScoreQueryTest, testCachingDouble) {
    // INT field values can be parsed as float
    doTestCaching(INT_FIELD, FieldScoreQuery::DOUBLE);
    // same values, but in double format
    doTestCaching(DOUBLE_FIELD, FieldScoreQuery::DOUBLE);
}
