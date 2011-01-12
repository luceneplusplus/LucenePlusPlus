/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(SpanExplanationsTest, ExplanationsFixture)

BOOST_AUTO_TEST_CASE(testST1)
{
    SpanQueryPtr q = st(L"w1");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testST2)
{
    SpanQueryPtr q = st(L"w1");
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testST4)
{
    SpanQueryPtr q = st(L"xx");
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testST5)
{
    SpanQueryPtr q = st(L"xx");
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testSF1)
{
    SpanQueryPtr q = sf(L"w1", 1);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSF2)
{
    SpanQueryPtr q = sf(L"w1", 1);
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSF4)
{
    SpanQueryPtr q = sf(L"xx", 2);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSF5)
{
    SpanQueryPtr q = sf(L"yy", 2);
    qtest(q, Collection<int32_t>::newInstance());
}

BOOST_AUTO_TEST_CASE(testSF6)
{
    SpanQueryPtr q = sf(L"yy", 4);
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSO1)
{
    SpanQueryPtr q = sor(L"w1", L"QQ");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSO2)
{
    SpanQueryPtr q = sor(L"w1", L"w3", L"zz");
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSO3)
{
    SpanQueryPtr q = sor(L"w5", L"QQ", L"yy");
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSO4)
{
    SpanQueryPtr q = sor(L"w5", L"QQ", L"yy");
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSNear1)
{
    SpanQueryPtr q = snear(L"w1", L"QQ", 100, true);
    qtest(q, Collection<int32_t>::newInstance());
}

BOOST_AUTO_TEST_CASE(testSNear2)
{
    SpanQueryPtr q = snear(L"w1", L"xx", 100, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testSNear3)
{
    SpanQueryPtr q = snear(L"w1", L"xx", 0, true);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSNear4)
{
    SpanQueryPtr q = snear(L"w1", L"xx", 1, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testSNear5)
{
    SpanQueryPtr q = snear(L"xx", L"w1", 0, false);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSNear6)
{
    SpanQueryPtr q = snear(L"w1", L"w2", L"QQ", 100, true);
    qtest(q, Collection<int32_t>::newInstance());
}

BOOST_AUTO_TEST_CASE(testSNear7)
{
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 100, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testSNear8)
{
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 0, true);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSNear9)
{
    SpanQueryPtr q = snear(L"w1", L"xx", L"w2", 1, true);
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testSNear10)
{
    SpanQueryPtr q = snear(L"xx", L"w1", L"w2", 0, false);
    qtest(q, newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(testSNear11)
{
    SpanQueryPtr q = snear(L"w1", L"w2", L"w3", 1, true);
    qtest(q, newCollection<int32_t>(0, 1));
}

BOOST_AUTO_TEST_CASE(testSNot1)
{
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"QQ"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSNot2)
{
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"QQ"));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSNot4)
{
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSNot5)
{
    SpanQueryPtr q = snot(sf(L"w1", 10), st(L"xx"));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSNot7)
{
    SpanQueryPtr f = snear(L"w1", L"w3", 10, true);
    f->setBoost(1000);
    SpanQueryPtr q = snot(f, st(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 3));
}

BOOST_AUTO_TEST_CASE(testSNot10)
{
    SpanQueryPtr t = st(L"xx");
    t->setBoost(10000);
    SpanQueryPtr q = snot(snear(L"w1", L"w3", 10, true), t);
    qtest(q, newCollection<int32_t>(0, 1, 3));
}

BOOST_AUTO_TEST_SUITE_END()
