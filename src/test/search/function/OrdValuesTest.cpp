/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "FunctionFixture.h"
#include "IndexSearcher.h"
#include "ValueSource.h"
#include "OrdFieldSource.h"
#include "ReverseOrdFieldSource.h"
#include "ValueSourceQuery.h"
#include "QueryUtils.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "Document.h"
#include "IndexReader.h"
#include "DocValues.h"
#include "VariantUtils.h"

using namespace Lucene;

/// Test search based on OrdFieldSource and ReverseOrdFieldSource.
///
/// Tests here create an index with a few documents, each having an indexed "id" field.
/// The ord values of this field are later used for scoring.
///
/// The order tests use Hits to verify that docs are ordered as expected.
///
/// The exact score tests use TopDocs top to verify the exact score.
class OrdValuesTest : public FunctionFixture {
public:
    OrdValuesTest() : FunctionFixture(false) {
    }

    virtual ~OrdValuesTest() {
    }

public:
    void doTestRank(const String& field, bool inOrder) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        ValueSourcePtr vs;
        if (inOrder) {
            vs = newLucene<OrdFieldSource>(field);
        } else {
            vs = newLucene<ReverseOrdFieldSource>(field);
        }
        QueryPtr q = newLucene<ValueSourceQuery>(vs);
        QueryUtils::check(q, s);
        Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(N_DOCS, h.size());
        String prevID = inOrder ?
                        L"IE" : // greater than all ids of docs in this test ("ID0001", etc.)
                        L"IC"; // smaller than all ids of docs in this test ("ID0001", etc.)
        for (int32_t i = 0; i < h.size(); ++i) {
            String resID = s->doc(h[i]->doc)->get(ID_FIELD);
            if (inOrder) {
                EXPECT_TRUE(resID.compare(prevID) < 0);
            } else {
                EXPECT_TRUE(resID.compare(prevID) > 0);
            }
            prevID = resID;
        }
    }

    void doTestExactScore(const String& field, bool inOrder) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        ValueSourcePtr vs;
        if (inOrder) {
            vs = newLucene<OrdFieldSource>(field);
        } else {
            vs = newLucene<ReverseOrdFieldSource>(field);
        }
        QueryPtr q = newLucene<ValueSourceQuery>(vs);
        TopDocsPtr td = s->search(q, FilterPtr(),1000);
        EXPECT_EQ(N_DOCS, td->totalHits);
        Collection<ScoreDocPtr> sd = td->scoreDocs;
        for (int32_t i = 0; i < sd.size(); ++i) {
            double score = sd[i]->score;
            String id = s->getIndexReader()->document(sd[i]->doc)->get(ID_FIELD);
            double expectedScore = N_DOCS - i;
            EXPECT_NEAR(expectedScore, score, TEST_SCORE_TOLERANCE_DELTA);
            String expectedId = inOrder ?
                                id2String(N_DOCS - i) : // in-order ==> larger  values first
                                id2String(i + 1); // reverse  ==> smaller values first
            EXPECT_EQ(expectedId, id);
        }
    }

    void doTestCaching(const String& field, bool inOrder) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        CollectionValue innerArray = VariantUtils::null();
        bool warned = false; // print warning once.
        for (int32_t i = 0; i < 10; ++i) {
            ValueSourcePtr vs;
            if (inOrder) {
                vs = newLucene<OrdFieldSource>(field);
            } else {
                vs = newLucene<ReverseOrdFieldSource>(field);
            }
            ValueSourceQueryPtr q = newLucene<ValueSourceQuery>(vs);
            Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
            try {
                EXPECT_EQ(N_DOCS, h.size());
                Collection<IndexReaderPtr> readers = s->getIndexReader()->getSequentialSubReaders();
                for (int32_t j = 0; j < readers.size(); ++j) {
                    IndexReaderPtr reader = readers[j];
                    if (i == 0) {
                        innerArray = q->valSrc->getValues(reader)->getInnerArray();
                    } else {
                        EXPECT_TRUE(equalCollectionValues(innerArray, q->valSrc->getValues(reader)->getInnerArray()));
                    }
                }
            } catch (UnsupportedOperationException&) {
                if (!warned) {
                    // std::cout << "WARNING: Cannot fully test values of " << StringUtils::toUTF8(q->toString());
                    warned = true;
                }
            }
        }

        // verify that different values are loaded for a different field
        String field2 = INT_FIELD;
        EXPECT_NE(field, field2); // otherwise this test is meaningless.
        ValueSourcePtr vs;
        if (inOrder) {
            vs = newLucene<OrdFieldSource>(field2);
        } else {
            vs = newLucene<ReverseOrdFieldSource>(field2);
        }
        ValueSourceQueryPtr q = newLucene<ValueSourceQuery>(vs);
        Collection<ScoreDocPtr> h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(N_DOCS, h.size());
        Collection<IndexReaderPtr> readers = s->getIndexReader()->getSequentialSubReaders();
        for (int32_t j = 0; j < readers.size(); ++j) {
            IndexReaderPtr reader = readers[j];
            try {
                EXPECT_TRUE(!equalCollectionValues(innerArray, q->valSrc->getValues(reader)->getInnerArray()));
            } catch (UnsupportedOperationException&) {
                if (!warned) {
                    // std::cout << "WARNING: Cannot fully test values of " << StringUtils::toUTF8(q->toString());
                    warned = true;
                }
            }
        }

        // verify new values are reloaded (not reused) for a new reader
        s = newLucene<IndexSearcher>(dir, true);
        if (inOrder) {
            vs = newLucene<OrdFieldSource>(field);
        } else {
            vs = newLucene<ReverseOrdFieldSource>(field);
        }
        q = newLucene<ValueSourceQuery>(vs);
        h = s->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(N_DOCS, h.size());
        readers = s->getIndexReader()->getSequentialSubReaders();
        for (int32_t j = 0; j < readers.size(); ++j) {
            IndexReaderPtr reader = readers[j];
            try {
                EXPECT_TRUE(!equalCollectionValues(innerArray, q->valSrc->getValues(reader)->getInnerArray()));
            } catch (UnsupportedOperationException&) {
                if (!warned) {
                    // std::cout << "WARNING: Cannot fully test values of " << StringUtils::toUTF8(q->toString());
                    warned = true;
                }
            }
        }
    }
};

TEST_F(OrdValuesTest, testOrdFieldRank) {
    doTestRank(ID_FIELD, true);
}

TEST_F(OrdValuesTest, testReverseOrdFieldRank) {
    doTestRank(ID_FIELD, false);
}

TEST_F(OrdValuesTest, testOrdFieldExactScore) {
    doTestExactScore(ID_FIELD, true);
}

TEST_F(OrdValuesTest, testReverseOrdFieldExactScore) {
    doTestExactScore(ID_FIELD, false);
}

TEST_F(OrdValuesTest, testCachingOrd) {
    doTestCaching(ID_FIELD, true);
}

TEST_F(OrdValuesTest, testCachingReverseOrd) {
    doTestCaching(ID_FIELD, false);
}
