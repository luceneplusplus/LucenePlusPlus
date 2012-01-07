/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "PriorityQueue.h"
#include "Random.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(PriorityQueueTest, LuceneTestFixture)

DECLARE_LUCENE_PTR(IntegerQueue)

class IntegerQueue : public PriorityQueue<int32_t>
{
public:
    IntegerQueue(int32_t maxSize) : PriorityQueue<int32_t>(maxSize)
    {
    }

    virtual ~IntegerQueue()
    {
    }
};

DECLARE_LUCENE_PTR(IntegerPtrQueue)

class Integer : public gc_object
{
public:
    Integer(int32_t num) : number(num)
    {
    }

    virtual ~Integer()
    {
    }

    int32_t number;
};

DECLARE_LUCENE_PTR(Integer)

class IntegerPtrQueue : public PriorityQueue<IntegerPtr>
{
public:
    IntegerPtrQueue(int32_t maxSize) : PriorityQueue<IntegerPtr>(maxSize)
    {
    }

    virtual ~IntegerPtrQueue()
    {
    }

protected:
    virtual bool lessThan(const IntegerPtr& first, const IntegerPtr& second)
    {
        return (first->number < second->number);
    }
};

BOOST_AUTO_TEST_CASE(testPriorityQueue)
{
    IntegerQueuePtr testQueue = newLucene<IntegerQueue>(10000);
    int64_t sum = 0;
    RandomPtr random = newLucene<Random>();

    for (int32_t i = 0; i < 10000; ++i)
    {
        int32_t next = random->nextInt();
        sum += next;
        testQueue->add(next);
    }

    int32_t last = INT_MIN;
    int64_t sum2 = 0;

    for (int32_t i = 0; i < 10000; ++i)
    {
        int32_t next = testQueue->pop();
        BOOST_CHECK(next >= last);
        last = next;
        sum2 += last;
    }

    BOOST_CHECK_EQUAL(sum, sum2);
}

BOOST_AUTO_TEST_CASE(testPriorityQueueOverflow)
{
    IntegerQueuePtr testQueue = newLucene<IntegerQueue>(3);
    testQueue->addOverflow(2);
    testQueue->addOverflow(3);
    testQueue->addOverflow(1);
    testQueue->addOverflow(5);
    testQueue->addOverflow(7);
    testQueue->addOverflow(1);
    BOOST_CHECK_EQUAL(testQueue->size(), 3);
    BOOST_CHECK_EQUAL(3, testQueue->top());
}

BOOST_AUTO_TEST_CASE(testPriorityQueueClear)
{
    IntegerQueuePtr testQueue = newLucene<IntegerQueue>(3);
    testQueue->add(2);
    testQueue->add(3);
    testQueue->add(1);
    BOOST_CHECK_EQUAL(testQueue->size(), 3);
    testQueue->clear();
    BOOST_CHECK(testQueue->empty());
}

BOOST_AUTO_TEST_CASE(testPriorityQueueUpdate)
{
    IntegerPtrQueuePtr testQueue = newLucene<IntegerPtrQueue>(1024);
    testQueue->add(new_gc<Integer>(2));
    testQueue->add(new_gc<Integer>(3));
    testQueue->add(new_gc<Integer>(1));
    testQueue->add(new_gc<Integer>(4));
    testQueue->add(new_gc<Integer>(5));
    BOOST_CHECK_EQUAL(testQueue->size(), 5);

    IntegerPtr top = testQueue->top();
    BOOST_CHECK_EQUAL(top->number, 1);

    top->number = 6;
    testQueue->updateTop();

    BOOST_CHECK_EQUAL(testQueue->pop()->number, 2);
    BOOST_CHECK_EQUAL(testQueue->pop()->number, 3);
    BOOST_CHECK_EQUAL(testQueue->pop()->number, 4);
    BOOST_CHECK_EQUAL(testQueue->pop()->number, 5);
    BOOST_CHECK_EQUAL(testQueue->pop()->number, 6);
}

BOOST_AUTO_TEST_SUITE_END()
