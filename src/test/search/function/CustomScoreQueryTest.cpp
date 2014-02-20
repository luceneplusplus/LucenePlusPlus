/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "FunctionFixture.h"
#include "FieldScoreQuery.h"
#include "IndexSearcher.h"
#include "QueryParser.h"
#include "Query.h"
#include "CustomScoreQuery.h"
#include "CustomScoreProvider.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "ValueSourceQuery.h"
#include "Explanation.h"
#include "IndexReader.h"
#include "Document.h"
#include "FieldCache.h"

using namespace Lucene;

class CustomAddScoreProvider : public CustomScoreProvider {
public:
    CustomAddScoreProvider(const IndexReaderPtr& reader) : CustomScoreProvider(reader) {
    }

    virtual ~CustomAddScoreProvider() {
    }

public:
    virtual double customScore(int32_t doc, double subQueryScore, double valSrcScore) {
        return subQueryScore + valSrcScore;
    }

    virtual ExplanationPtr customExplain(int32_t doc, const ExplanationPtr& subQueryExpl, const ExplanationPtr& valSrcExpl) {
        double valSrcScore = valSrcExpl ? valSrcExpl->getValue() : 0.0;
        ExplanationPtr exp = newLucene<Explanation>(valSrcScore + subQueryExpl->getValue(), L"custom score: sum of:");
        exp->addDetail(subQueryExpl);
        if (valSrcExpl) {
            exp->addDetail(valSrcExpl);
        }
        return exp;
    }
};

class CustomAddQuery : public CustomScoreQuery {
public:
    CustomAddQuery(const QueryPtr& q, const ValueSourceQueryPtr& qValSrc) : CustomScoreQuery(q, qValSrc) {
    }

    virtual ~CustomAddQuery() {
    }

public:
    virtual String name() {
        return L"customAdd";
    }

protected:
    virtual CustomScoreProviderPtr getCustomScoreProvider(const IndexReaderPtr& reader) {
        return newLucene<CustomAddScoreProvider>(reader);
    }
};

class CustomMulAddScoreProvider : public CustomScoreProvider {
public:
    CustomMulAddScoreProvider(const IndexReaderPtr& reader) : CustomScoreProvider(reader) {
    }

    virtual ~CustomMulAddScoreProvider() {
    }

public:
    virtual double customScore(int32_t doc, double subQueryScore, Collection<double> valSrcScores) {
        if (valSrcScores.empty()) {
            return subQueryScore;
        }
        if (valSrcScores.size() == 1) {
            return subQueryScore + valSrcScores[0];
        }
        // confirm that skipping beyond the last doc, on the previous reader, hits NO_MORE_DOCS
        return (subQueryScore + valSrcScores[0]) * valSrcScores[1]; // we know there are two
    }

    virtual ExplanationPtr customExplain(int32_t doc, const ExplanationPtr& subQueryExpl, Collection<ExplanationPtr> valSrcExpls) {
        if (valSrcExpls.empty()) {
            return subQueryExpl;
        }
        ExplanationPtr exp = newLucene<Explanation>(valSrcExpls[0]->getValue() + subQueryExpl->getValue(), L"sum of:");
        exp->addDetail(subQueryExpl);
        exp->addDetail(valSrcExpls[0]);
        if (valSrcExpls.size() == 1) {
            exp->setDescription(L"CustomMulAdd, sum of:");
            return exp;
        }
        ExplanationPtr exp2 = newLucene<Explanation>(valSrcExpls[1]->getValue() * exp->getValue(), L"custom score: product of:");
        exp2->addDetail(valSrcExpls[1]);
        exp2->addDetail(exp);
        return exp2;
    }
};

class CustomMulAddQuery : public CustomScoreQuery {
public:
    CustomMulAddQuery(const QueryPtr& q, const ValueSourceQueryPtr& qValSrc1, const ValueSourceQueryPtr& qValSrc2) : CustomScoreQuery(q, newCollection<ValueSourceQueryPtr>(qValSrc1, qValSrc2)) {
    }

    virtual ~CustomMulAddQuery() {
    }

public:
    virtual String name() {
        return L"customMulAdd";
    }

protected:
    virtual CustomScoreProviderPtr getCustomScoreProvider(const IndexReaderPtr& reader) {
        return newLucene<CustomMulAddScoreProvider>(reader);
    }
};

class CustomExternalScoreProvider : public CustomScoreProvider {
public:
    CustomExternalScoreProvider(const IndexReaderPtr& reader, Collection<int32_t> values) : CustomScoreProvider(reader) {
        this->values = values;
    }

    virtual ~CustomExternalScoreProvider() {
    }

protected:
    Collection<int32_t> values;

public:
    virtual double customScore(int32_t doc, double subQueryScore, double valSrcScore) {
        EXPECT_TRUE(doc <= reader->maxDoc());
        return (double)values[doc];
    }
};

class CustomExternalQuery : public CustomScoreQuery {
public:
    CustomExternalQuery(const QueryPtr& q) : CustomScoreQuery(q) {
    }

    virtual ~CustomExternalQuery() {
    }

protected:
    virtual CustomScoreProviderPtr getCustomScoreProvider(const IndexReaderPtr& reader) {
        Collection<int32_t> values = FieldCache::DEFAULT()->getInts(reader, FunctionFixture::INT_FIELD);
        return newLucene<CustomExternalScoreProvider>(reader, values);
    }
};

class CustomScoreQueryTest : public FunctionFixture {
public:
    CustomScoreQueryTest() : FunctionFixture(true) {
    }

    virtual ~CustomScoreQueryTest() {
    }

public:
    /// since custom scoring modifies the order of docs, map results by doc ids so that we can later compare/verify them
    MapIntDouble topDocsToMap(const TopDocsPtr& td) {
        MapIntDouble h = MapIntDouble::newInstance();
        for (int32_t i = 0; i < td->totalHits; ++i) {
            h.put(td->scoreDocs[i]->doc, td->scoreDocs[i]->score);
        }
        return h;
    }

    void verifyResults(double boost, IndexSearcherPtr s, MapIntDouble h1, MapIntDouble h2customNeutral, MapIntDouble h3CustomMul,
                       MapIntDouble h4CustomAdd, MapIntDouble h5CustomMulAdd, QueryPtr q1, QueryPtr q2, QueryPtr q3, QueryPtr q4, QueryPtr q5) {
        // verify numbers of matches
        EXPECT_EQ(h1.size(), h2customNeutral.size());
        EXPECT_EQ(h1.size(), h3CustomMul.size());
        EXPECT_EQ(h1.size(), h4CustomAdd.size());
        EXPECT_EQ(h1.size(), h5CustomMulAdd.size());

        // verify scores ratios
        for (MapIntDouble::iterator it = h1.begin(); it != h1.end(); ++it) {
            int32_t doc =  it->first;
            double fieldScore = expectedFieldScore(s->getIndexReader()->document(doc)->get(ID_FIELD));
            EXPECT_TRUE(fieldScore > 0);

            double score1 = it->second;

            double score2 = h2customNeutral.get(doc);
            EXPECT_NEAR(boost * score1, score2, TEST_SCORE_TOLERANCE_DELTA);

            double score3 = h3CustomMul.get(doc);
            EXPECT_NEAR(boost * fieldScore * score1, score3, TEST_SCORE_TOLERANCE_DELTA);

            double score4 = h4CustomAdd.get(doc);
            EXPECT_NEAR(boost * (fieldScore + score1), score4, TEST_SCORE_TOLERANCE_DELTA);

            double score5 = h5CustomMulAdd.get(doc);
            EXPECT_NEAR(boost * fieldScore * (score1 + fieldScore), score5, TEST_SCORE_TOLERANCE_DELTA);
        }
    }

    /// Test that FieldScoreQuery returns docs with expected score.
    void doTestCustomScore(const String& field, FieldScoreQuery::Type tp, double boost) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(dir, true);
        FieldScoreQueryPtr qValSrc = newLucene<FieldScoreQuery>(field, tp); // a query that would score by the field
        QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, TEXT_FIELD, anlzr);
        String qtxt = L"first aid text";

        // regular (boolean) query.
        QueryPtr q1 = qp->parse(qtxt);

        // custom query, that should score the same as q1.
        CustomScoreQueryPtr q2CustomNeutral = newLucene<CustomScoreQuery>(q1);
        q2CustomNeutral->setBoost(boost);

        // custom query, that should (by default) multiply the scores of q1 by that of the field
        CustomScoreQueryPtr q3CustomMul = newLucene<CustomScoreQuery>(q1, qValSrc);
        q3CustomMul->setStrict(true);
        q3CustomMul->setBoost(boost);

        // custom query, that should add the scores of q1 to that of the field
        CustomScoreQueryPtr q4CustomAdd = newLucene<CustomAddQuery>(q1,qValSrc);
        q4CustomAdd->setStrict(true);
        q4CustomAdd->setBoost(boost);

        // custom query, that multiplies and adds the field score to that of q1
        CustomScoreQueryPtr q5CustomMulAdd = newLucene<CustomMulAddQuery>(q1, qValSrc, qValSrc);
        q5CustomMulAdd->setStrict(true);
        q5CustomMulAdd->setBoost(boost);

        // do al the searches
        TopDocsPtr td1 = s->search(q1, FilterPtr(), 1000);
        TopDocsPtr td2CustomNeutral = s->search(q2CustomNeutral, FilterPtr(), 1000);
        TopDocsPtr td3CustomMul = s->search(q3CustomMul, FilterPtr(), 1000);
        TopDocsPtr td4CustomAdd = s->search(q4CustomAdd, FilterPtr(), 1000);
        TopDocsPtr td5CustomMulAdd = s->search(q5CustomMulAdd, FilterPtr(), 1000);

        // put results in map so we can verify the scores although they have changed
        MapIntDouble h1 = topDocsToMap(td1);
        MapIntDouble h2CustomNeutral = topDocsToMap(td2CustomNeutral);
        MapIntDouble h3CustomMul = topDocsToMap(td3CustomMul);
        MapIntDouble h4CustomAdd = topDocsToMap(td4CustomAdd);
        MapIntDouble h5CustomMulAdd = topDocsToMap(td5CustomMulAdd);

        verifyResults(boost, s, h1, h2CustomNeutral, h3CustomMul, h4CustomAdd, h5CustomMulAdd,
                      q1, q2CustomNeutral, q3CustomMul, q4CustomAdd, q5CustomMulAdd);
    }
};

TEST_F(CustomScoreQueryTest, testCustomExternalQuery) {
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, TEXT_FIELD, anlzr);
    String qtxt = L"first aid text"; // from the doc texts in FunctionFixture.
    QueryPtr q1 = qp->parse(qtxt);

    QueryPtr q = newLucene<CustomExternalQuery>(q1);

    IndexSearcherPtr s = newLucene<IndexSearcher>(dir);
    TopDocsPtr hits = s->search(q, 1000);
    EXPECT_EQ(N_DOCS, hits->totalHits);
    for (int32_t i = 0; i < N_DOCS; ++i) {
        int32_t doc = hits->scoreDocs[i]->doc;
        double score = hits->scoreDocs[i]->score;
        EXPECT_NEAR((double)(1 + (4 * doc) % N_DOCS), score, 0.0001);
    }
    s->close();
}

/// Test that CustomScoreQuery of Type.BYTE returns the expected scores.
TEST_F(CustomScoreQueryTest, testCustomScoreByte) {
    // INT field values are small enough to be parsed as byte
    doTestCustomScore(INT_FIELD, FieldScoreQuery::BYTE, 1.0);
    doTestCustomScore(INT_FIELD, FieldScoreQuery::BYTE, 2.0);
}

/// Test that CustomScoreQuery of Type.INT returns the expected scores.
TEST_F(CustomScoreQueryTest, testCustomScoreInt) {
    // INT field values are small enough to be parsed as int
    doTestCustomScore(INT_FIELD, FieldScoreQuery::INT, 1.0);
    doTestCustomScore(INT_FIELD, FieldScoreQuery::INT, 2.0);
}

/// Test that CustomScoreQuery of Type.DOUBLE returns the expected scores.
TEST_F(CustomScoreQueryTest, testCustomScoreDouble) {
    // INT field can be parsed as double
    doTestCustomScore(INT_FIELD, FieldScoreQuery::DOUBLE, 1.0);
    doTestCustomScore(INT_FIELD, FieldScoreQuery::DOUBLE, 5.0);
    // same values, but in double format
    doTestCustomScore(DOUBLE_FIELD, FieldScoreQuery::DOUBLE, 1.0);
    doTestCustomScore(DOUBLE_FIELD, FieldScoreQuery::DOUBLE, 6.0);
}
