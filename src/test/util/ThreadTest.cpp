/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "LuceneThread.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(LuceneThreadTest, LuceneTestFixture)

DECLARE_LUCENE_PTR(StartStopThread)

class StartStopThread : public LuceneThread
{
public:
    StartStopThread(int32_t sleepTime = 500)
    {
        this->sleepTime = sleepTime;
        this->started = false;        
    }
    
    virtual ~StartStopThread()
    {
    }
    
    LUCENE_CLASS(StartStopThread);
    
public:
    int32_t sleepTime;
    bool started;
    
public:
    virtual void run()
    {
        started = true;
        LuceneThread::threadSleep(sleepTime);
    }
};

BOOST_AUTO_TEST_CASE(testThreadStart)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    BOOST_CHECK(!testThread->started);
    testThread->start();
    testThread->join();
    BOOST_CHECK(testThread->started);
}

BOOST_AUTO_TEST_CASE(testThreadAlive)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    BOOST_CHECK(!testThread->isAlive());
    testThread->start();
    BOOST_CHECK(testThread->isAlive());
    testThread->join();
    BOOST_CHECK(!testThread->isAlive());
}

BOOST_AUTO_TEST_CASE(testThreadJoinNotStarted)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    testThread->join();
    BOOST_CHECK(!testThread->started);
}

BOOST_AUTO_TEST_CASE(testThreadStartMultipleJoins)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    BOOST_CHECK(!testThread->started);
    testThread->start();
    BOOST_CHECK(testThread->join());
    BOOST_CHECK(testThread->join());
    BOOST_CHECK(testThread->started);
}

BOOST_AUTO_TEST_CASE(testThreadStartJoinWithTimeout)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    BOOST_CHECK(!testThread->started);
    testThread->start();
    BOOST_CHECK(!testThread->join(10));
    BOOST_CHECK(testThread->isAlive());
    BOOST_CHECK(testThread->started);
    LuceneThread::threadSleep(500);
    BOOST_CHECK(testThread->join());
    BOOST_CHECK(!testThread->isAlive());
}

BOOST_AUTO_TEST_CASE(testThreadSetPriority)
{
    StartStopThreadPtr testThread = newLucene<StartStopThread>();
    BOOST_CHECK_EQUAL(testThread->getPriority(), LuceneThread::NORM_PRIORITY);
    #if defined(_WIN32) || defined(_WIN64)
    testThread->setPriority(LuceneThread::MIN_PRIORITY);
    BOOST_CHECK_EQUAL(testThread->getPriority(), LuceneThread::MIN_PRIORITY);
    #endif
}

BOOST_AUTO_TEST_CASE(testThreadCreateCollection)
{
    static int32_t collectionSize = 1000;
    Collection<StartStopThreadPtr> collectionThread = Collection<StartStopThreadPtr>::newInstance(collectionSize);
    for (int32_t i = 0; i < collectionSize; ++i)
        collectionThread[i] = newLucene<StartStopThread>(50);
    for (int32_t i = 0; i < collectionSize; ++i)
        collectionThread[i]->start();
    for (int32_t i = 0; i < collectionSize; ++i)
        BOOST_CHECK(collectionThread[i]->join());
}

BOOST_AUTO_TEST_SUITE_END()
