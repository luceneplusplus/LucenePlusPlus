/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "ExplanationsFixture.h"
#include "SpanQuery.h"
#include "SpanTermQuery.h"
#include "SpanFirstQuery.h"
#include "SpanOrQuery.h"
#include "SpanNearQuery.h"
#include "SpanNotQuery.h"

using namespace Lucene;

typedef ExplanationsFixture SpanExplanationsTest;

TEST_F(SpanExplanationsTest, testST1) {
    SpanQueryPtr q = st(L"w1");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testST2) {
    SpanQueryPtr q = st(L"w1");
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testST4) {
    SpanQueryPtr q = st(L"xx");
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testST5) {
    SpanQueryPtr q = st(L"xx");
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testSF1) {
    SpanQueryPtr q = sf(L"w1", 1);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSF2) {
    SpanQueryPtr q = sf(L"w1", 1);
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSF4) {
    SpanQueryPtr q = sf(L"xx", 2);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSF5) {
    SpanQueryPtr q = sf(L"yy", 2);
    qtest(q, Collection<int32_t>::newInstance());
}

TEST_F(SpanExplanationsTest, testSF6) {
    SpanQueryPtr q = sf(L"yy", 4);
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSO1) {
    SpanQueryPtr q = sor(L"w1", L"QQ");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSO2) {
    SpanQueryPtr q = sor(L"w1", L"w3", L"zz");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSO3) {
    SpanQueryPtr q = sor(L"w5", L"QQ", L"yy");
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

TEST_F(SpanExplanationsTest, testSO4) {
    SpanQueryPtr q = sor(L"w5", L"QQ", L"yy");
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

TEST_F(SpanExplanationsTest, testSNear1) {
    SpanQueryPtr q = snear(L"w1", L"QQ", 100, true);
    qtest(q, Collection<int32_t>::newInstance());
}

TEST_F(SpanExplanationsTest, testSNear2) {
    SpanQueryPtr q = snear(L"w1", L"xx", 100, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testSNear3) {
    SpanQueryPtr q = snear(L"w1", L"xx", 0, true);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSNear4) {
    SpanQueryPtr q = snear(L"w1", L"xx", 1, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testSNear5) {
    SpanQueryPtr q = snear(L"xx", L"w1", 0, false);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSNear6) {
    SpanQueryPtr q = snear(L"w1", L"w2", L"QQ", 100, true);
    qtest(q, Collection<int32_t>::newInstance());
}

TEST_F(SpanExplanationsTest, testSNear7) {
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 100, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testSNear8) {
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 0, true);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSNear9) {
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 1, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

TEST_F(SpanExplanationsTest, testSNear10) {
    SpanQueryPtr q = snear(L"xx", L"w1", L"w2", 0, false);
    qtest(q, newCollection<int32_t>(2));
}

TEST_F(SpanExplanationsTest, testSNear11) {
    SpanQueryPtr q = snear(L"w1", L"w2", L"w3", 1, true);
    qtest(q, newCollection<int32_t>(0, 1));
}

TEST_F(SpanExplanationsTest, testSNot1) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"QQ"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSNot2) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"QQ"));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSNot4) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSNot5) {
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"xx"));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpanExplanationsTest, testSNot7) {
    SpanQueryPtr f = snear(L"w1", L"w3", 10, true);
    f->setBoost(1000);
    SpanQueryPtr q = snot(f, st(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 3));
}

TEST_F(SpanExplanationsTest, testSNot10) {
    SpanQueryPtr t = st(L"xx");
    t->setBoost(10000);
    SpanQueryPtr q = snot(snear(L"w1", L"w3", 10, true), t);
    qtest(q, newCollection<int32_t>(0, 1, 3));
}
