/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "PriorityQueue.h"
#include "Random.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(PriorityQueueTest, LuceneTestFixture)

DECLARE_SHARED_PTR(IntegerQueue)

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

DECLARE_SHARED_PTR(IntegerPtrQueue)
typedef boost::shared_ptr<int32_t> IntPtr;

class IntegerPtrQueue : public PriorityQueue<IntPtr>
{
public:
    IntegerPtrQueue(int32_t maxSize) : PriorityQueue<IntPtr>(maxSize)
    {	
    }
    
    virtual ~IntegerPtrQueue()
    {
    }

protected:
    virtual bool lessThan(const IntPtr& first, const IntPtr& second)
    {
        return (*first < *second);
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
    testQueue->add(newInstance<int32_t>(2));
    testQueue->add(newInstance<int32_t>(3));
    testQueue->add(newInstance<int32_t>(1));
    testQueue->add(newInstance<int32_t>(4));
    testQueue->add(newInstance<int32_t>(5));
    BOOST_CHECK_EQUAL(testQueue->size(), 5);
    
    IntPtr top = testQueue->top();
    BOOST_CHECK_EQUAL(*top, 1);
    
    *top = 6;
    testQueue->updateTop();
    
    BOOST_CHECK_EQUAL(*testQueue->pop(), 2);
    BOOST_CHECK_EQUAL(*testQueue->pop(), 3);
    BOOST_CHECK_EQUAL(*testQueue->pop(), 4);
    BOOST_CHECK_EQUAL(*testQueue->pop(), 5);
    BOOST_CHECK_EQUAL(*testQueue->pop(), 6);
}

BOOST_AUTO_TEST_SUITE_END()
