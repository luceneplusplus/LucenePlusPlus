/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "ExplanationsFixture.h"
#include "DefaultSimilarity.h"
#include "BooleanQuery.h"
#include "QueryParser.h"
#include "FilteredQuery.h"
#include "FieldCacheTermsFilter.h"
#include "DisjunctionMaxQuery.h"
#include "IndexSearcher.h"
#include "SpanFirstQuery.h"
#include "SpanOrQuery.h"
#include "SpanNearQuery.h"
#include "SpanNotQuery.h"
#include "SpanTermQuery.h"
#include "ConstantScoreQuery.h"
#include "MatchAllDocsQuery.h"
#include "MultiPhraseQuery.h"

using namespace Lucene;

class Qnorm1Similarity : public DefaultSimilarity {
public:
    virtual ~Qnorm1Similarity() {
    }

public:
    virtual double queryNorm(double sumOfSquaredWeights) {
        return 1.0;
    }
};

class ItemizedFilter : public FieldCacheTermsFilter {
public:
    ItemizedFilter(const String& field, Collection<int32_t> terms) : FieldCacheTermsFilter(field, int2str(terms)) {
    }

    ItemizedFilter(Collection<int32_t> terms) : FieldCacheTermsFilter(L"KEY", int2str(terms)) {
    }

    virtual ~ItemizedFilter() {
    }

public:
    Collection<String> int2str(Collection<int32_t> terms) {
        Collection<String> out = Collection<String>::newInstance(terms.size());
        for (int32_t i = 0; i < terms.size(); ++i) {
            out[i] = StringUtils::toString(terms[i]);
        }
        return out;
    }
};

/// TestExplanations subclass that builds up super crazy complex queries on the assumption that
/// if the explanations work out right for them, they should work for anything.
class ComplexExplanationsTest : public ExplanationsFixture {
public:
    ComplexExplanationsTest() {
        searcher->setSimilarity(createQnorm1Similarity());
    }

    virtual ~ComplexExplanationsTest() {
    }

protected:
    DefaultSimilarityPtr createQnorm1Similarity() {
        return newLucene<Qnorm1Similarity>();
    }
};

TEST_F(ComplexExplanationsTest, test1) {
    BooleanQueryPtr q = newLucene<BooleanQuery>();

    q->add(qp->parse(L"\"w1 w2\"~1"), BooleanClause::MUST);
    q->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true), BooleanClause::SHOULD);
    q->add(snear(sf(L"w3", 2), st(L"w2"), st(L"w3"), 5, true), BooleanClause::SHOULD);

    QueryPtr t = newLucene<FilteredQuery>(qp->parse(L"xx"),    newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3)));
    t->setBoost(1000);
    q->add(t, BooleanClause::SHOULD);

    t = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(0, 2)));
    t->setBoost(30);
    q->add(t, BooleanClause::SHOULD);

    DisjunctionMaxQueryPtr dm = newLucene<DisjunctionMaxQuery>(0.2);
    dm->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true));
    dm->add(qp->parse(L"QQ"));
    dm->add(qp->parse(L"xx yy -zz"));
    dm->add(qp->parse(L"-xx -w1"));

    DisjunctionMaxQueryPtr dm2 = newLucene<DisjunctionMaxQuery>(0.5);
    dm2->add(qp->parse(L"w1"));
    dm2->add(qp->parse(L"w2"));
    dm2->add(qp->parse(L"w3"));
    dm->add(dm2);

    q->add(dm, BooleanClause::SHOULD);

    BooleanQueryPtr b = newLucene<BooleanQuery>();
    b->setMinimumNumberShouldMatch(2);
    b->add(snear(L"w1", L"w2", 1, true), BooleanClause::SHOULD);
    b->add(snear(L"w2", L"w3", 1, true), BooleanClause::SHOULD);
    b->add(snear(L"w1", L"w3", 3, true), BooleanClause::SHOULD);

    q->add(b, BooleanClause::SHOULD);

    qtest(q, newCollection<int32_t>(0, 1, 2));
}

TEST_F(ComplexExplanationsTest, test2) {
    BooleanQueryPtr q = newLucene<BooleanQuery>();

    q->add(qp->parse(L"\"w1 w2\"~1"), BooleanClause::MUST);
    q->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true), BooleanClause::SHOULD);
    q->add(snear(sf(L"w3", 2), st(L"w2"), st(L"w3"), 5, true), BooleanClause::SHOULD);

    QueryPtr t = newLucene<FilteredQuery>(qp->parse(L"xx"),    newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3)));
    t->setBoost(1000);
    q->add(t, BooleanClause::SHOULD);

    t = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(0, 2)));
    t->setBoost(-20);
    q->add(t, BooleanClause::SHOULD);

    DisjunctionMaxQueryPtr dm = newLucene<DisjunctionMaxQuery>(0.2);
    dm->add(snear(st(L"w2"), sor(L"w5", L"zz"), 4, true));
    dm->add(qp->parse(L"QQ"));
    dm->add(qp->parse(L"xx yy -zz"));
    dm->add(qp->parse(L"-xx -w1"));

    DisjunctionMaxQueryPtr dm2 = newLucene<DisjunctionMaxQuery>(0.5);
    dm2->add(qp->parse(L"w1"));
    dm2->add(qp->parse(L"w2"));
    dm2->add(qp->parse(L"w3"));
    dm->add(dm2);

    q->add(dm, BooleanClause::SHOULD);

    BooleanQueryPtr b = newLucene<BooleanQuery>();
    b->setMinimumNumberShouldMatch(2);
    b->add(snear(L"w1", L"w2", 1, true), BooleanClause::SHOULD);
    b->add(snear(L"w2", L"w3", 1, true), BooleanClause::SHOULD);
    b->add(snear(L"w1", L"w3", 3, true), BooleanClause::SHOULD);
    b->setBoost(0.0);

    q->add(b, BooleanClause::SHOULD);

    qtest(q, newCollection<int32_t>(0, 1, 2));
}

TEST_F(ComplexExplanationsTest, testT3) {
    bqtest(L"w1^0.0", newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testMA3) {
    QueryPtr q = newLucene<MatchAllDocsQuery>();
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testFQ5) {
    bqtest(newLucene<FilteredQuery>(qp->parse(L"xx^0"), newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3))), newCollection<int32_t>(3));
}

TEST_F(ComplexExplanationsTest, testCSQ4) {
    QueryPtr q = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(3)));
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(3));
}

TEST_F(ComplexExplanationsTest, testDMQ10) {
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"yy w5^100"));
    q->add(qp->parse(L"xx^0"));
    q->setBoost(0.0);
    bqtest(q, newCollection<int32_t>(0, 2, 3));
}

TEST_F(ComplexExplanationsTest, testMPQ7) {
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1")));
    q->add(ta(newCollection<String>(L"w2")));
    q->setSlop(1);
    q->setBoost(0.0);
    bqtest(q, newCollection<int32_t>(0, 1, 2));
}

TEST_F(ComplexExplanationsTest, testBQ12) {
    qtest(L"w1 w2^0.0", newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testBQ13) {
    qtest(L"w1 -w5^0.0", newCollection<int32_t>(1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testBQ18) {
    qtest(L"+w1^0.0 w2", newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testBQ21) {
    bqtest(L"(+w1 w2)^0.0", newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testBQ22) {
    bqtest(L"(+w1^0.0 w2)^0.0", newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testST3) {
    SpanQueryPtr q = st(L"w1");
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testST6) {
    SpanQueryPtr q = st(L"xx");
    q->setBoost(0);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(ComplexExplanationsTest, testSF3) {
    SpanQueryPtr q = sf(L"w1", 1);
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testSF7) {
    SpanQueryPtr q = sf(L"xx", 3);
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(ComplexExplanationsTest, testSNot3) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"QQ"));
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testSNot6) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"xx"));
    q->setBoost(0);
    bqtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(ComplexExplanationsTest, testSNot8) {
    SpanQueryPtr f = snear(L"w1", L"w3", 10, true);
    f->setBoost(0);
    SpanQueryPtr q = snot(f, st(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 3));
}

TEST_F(ComplexExplanationsTest, testSNot9) {
    SpanQueryPtr t = st(L"xx");
    t->setBoost(0);
    SpanQueryPtr q = snot(snear(L"w1", L"w3", 10, true), t);
    qtest(q, newCollection<int32_t>(0, 1, 3));
}
