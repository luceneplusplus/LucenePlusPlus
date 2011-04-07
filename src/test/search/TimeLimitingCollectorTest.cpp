/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "LuceneThread.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "IndexSearcher.h"
#include "QueryParser.h"
#include "Document.h"
#include "Field.h"
#include "Collector.h"
#include "BitSet.h"
#include "TimeLimitingCollector.h"

using namespace Lucene;

DECLARE_SHARED_PTR(MyHitCollector)

/// counting collector that can slow down at collect().
class MyHitCollector : public Collector
{
public:
    MyHitCollector()
    {
        bits = newLucene<BitSet>();
        slowdown = 0;
        lastDocCollected = -1;
        docBase = 0;
    }
    
    virtual ~MyHitCollector()
    {
    }

protected:
    BitSetPtr bits;
    int32_t slowdown;
    int32_t lastDocCollected;
    int32_t docBase;

public:
    /// amount of time to wait on each collect to simulate a long iteration
    void setSlowDown(int32_t milliseconds)
    {
        slowdown = milliseconds;
    }
    
    int32_t hitCount()
    {
        return bits->cardinality();
    }
    
    int32_t getLastDocCollected()
    {
        return lastDocCollected;
    }
    
    virtual void setScorer(ScorerPtr scorer)
    {
        // scorer is not needed
    }
    
    virtual void collect(int32_t doc)
    {
        int32_t docId = doc + docBase;
        if (slowdown > 0)
            LuceneThread::threadSleep(slowdown);
        if (docId < 0)
            BOOST_FAIL("Invalid doc");
        bits->set(docId);
        lastDocCollected = docId;
    }
    
    virtual void setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
        this->docBase = docBase;
    }
    
    virtual bool acceptsDocsOutOfOrder()
    {
        return false;
    }
};

class TimeLimitingCollectorFixture;

class TimeLimitingThread : public LuceneThread
{
public:
    TimeLimitingThread(bool withTimeout, TimeLimitingCollectorFixture* fixture);
    
    virtual ~TimeLimitingThread()
    {
    }

    LUCENE_CLASS(TimeLimitingThread);

protected:
    bool withTimeout;
    TimeLimitingCollectorFixture* fixture;

public:
    virtual void run();
};

/// Tests the {@link TimeLimitingCollector}.  
/// This test checks (1) search correctness (regardless of timeout), (2) expected timeout behaviour, and (3) a sanity test with multiple searching threads.
class TimeLimitingCollectorFixture : public LuceneTestFixture
{
public:
    TimeLimitingCollectorFixture()
    {
        Collection<String> docText = newCollection<String>(
            L"docThatNeverMatchesSoWeCanRequireLastDocCollectedToBeGreaterThanZero",
            L"one blah three",
            L"one foo three multiOne",
            L"one foobar three multiThree",
            L"blueberry pancakes",
            L"blueberry pie",
            L"blueberry strudel",
            L"blueberry pizza"
        );
        
        DirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
        
        for (int32_t i = 0; i < N_DOCS; ++i)
            add(docText[i % docText.size()], writer);
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);

        String qtxt = L"one";
        // start from 1, so that the 0th doc never matches
        for (int32_t i = 1; i < docText.size(); ++i)
            qtxt += L" " + docText[i]; // large query so that search will be longer
        QueryParserPtr queryParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, FIELD_NAME, newLucene<WhitespaceAnalyzer>());
        query = queryParser->parse(qtxt);

        // warm the searcher
        searcher->search(query, FilterPtr(), 1000);
    }
    
    virtual ~TimeLimitingCollectorFixture()
    {
        searcher->close();
        TimeLimitingCollector::setResolution(TimeLimitingCollector::DEFAULT_RESOLUTION);
        TimeLimitingCollector::stopTimer();
    }

protected:
    static const int32_t SLOW_DOWN;
    static const int64_t TIME_ALLOWED; // so searches can find about 17 docs.

    // max time allowed is relaxed for multi-threading tests. 
    // the multi-thread case fails when setting this to 1 (no slack) and launching many threads (>2000).  
    // but this is not a real failure, just noise.
    static const int32_t MULTI_THREAD_SLACK;      

    static const int32_t N_DOCS;
    static const int32_t N_THREADS;

    static const String FIELD_NAME;
    
    SearcherPtr searcher;
    QueryPtr query;

public:
    void add(const String& value, IndexWriterPtr writer)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD_NAME, value, Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    
    void doTestSearch()
    {
        int32_t totalResults = 0;
        int32_t totalTLCResults = 0;
        
        MyHitCollectorPtr myHc = newLucene<MyHitCollector>();
        search(myHc);
        totalResults = myHc->hitCount();

        myHc = newLucene<MyHitCollector>();
        int64_t oneHour = 3600000;
        CollectorPtr tlCollector = createTimedCollector(myHc, oneHour, false);
        search(tlCollector);
        totalTLCResults = myHc->hitCount();
        
        if (totalResults != totalTLCResults)
            BOOST_CHECK_EQUAL(totalResults, totalTLCResults);
    }
    
    void doTestTimeout(bool multiThreaded, bool greedy)
    {
        MyHitCollectorPtr myHc = newLucene<MyHitCollector>();
        myHc->setSlowDown(SLOW_DOWN);
        CollectorPtr tlCollector = createTimedCollector(myHc, TIME_ALLOWED, greedy);

        TimeExceededException timoutException;
        
        // search
        try
        {
            search(tlCollector);
        }
        catch (TimeExceededException& e)
        {
            timoutException = e;
        }
        catch (LuceneException& e)
        {
            BOOST_FAIL("Unexpected exception: " << e.getError());
        }
        
        // must get exception
        if (timoutException.isNull())
            BOOST_CHECK(!timoutException.isNull());
        
        String message = timoutException.getError();
        String::size_type last = message.find_last_of(L":");
        if (last == String::npos)
            BOOST_CHECK_NE(last, String::npos);
        
        // greediness affect last doc collected
        int32_t exceptionDoc = StringUtils::toInt(message.substr(last + 1));
        int32_t lastCollected = myHc->getLastDocCollected(); 
        if (exceptionDoc <= 0)
            BOOST_CHECK(exceptionDoc > 0);
        if (greedy)
        {
            if (exceptionDoc != lastCollected)
                BOOST_CHECK_EQUAL(exceptionDoc, lastCollected);
            if (myHc->hitCount() <= 0)
                BOOST_CHECK(myHc->hitCount() > 0);
        }
        else if (exceptionDoc <= lastCollected)
            BOOST_CHECK(exceptionDoc > lastCollected);

        String::size_type allowed = message.find_first_of(L":");
        if (allowed == String::npos)
            BOOST_CHECK_NE(allowed, String::npos);
        int32_t timeAllowed = StringUtils::toInt(message.substr(allowed + 1));
        
        String::size_type elapsed = message.find_first_of(L":", allowed + 1);
        if (elapsed == String::npos)
            BOOST_CHECK_NE(elapsed, String::npos);
        int32_t timeElapsed = StringUtils::toInt(message.substr(elapsed + 1));
        
        // verify that elapsed time at exception is within valid limits
        if (timeAllowed != TIME_ALLOWED)
            BOOST_CHECK_EQUAL(timeAllowed, TIME_ALLOWED);
        // a) Not too early
        if (timeElapsed <= TIME_ALLOWED - TimeLimitingCollector::getResolution())
            BOOST_CHECK(timeElapsed > TIME_ALLOWED - TimeLimitingCollector::getResolution());
        // b) Not too late.
        //    This part is problematic in a busy test system, so we just print a warning.
        //    We already verified that a timeout occurred, we just can't be picky about how long it took.
        if (timeElapsed > maxTime(multiThreaded))
        {
            BOOST_TEST_MESSAGE("Informative: timeout exceeded (no action required: most probably just " <<
                               "because the test machine is slower than usual): " <<
                               "lastDoc = " << exceptionDoc <<
                               ", && allowed = " << timeAllowed <<
                               ", && elapsed = " << timeElapsed << " >= " << StringUtils::toUTF8(maxTimeStr(multiThreaded)));
        }
    }
    
    void doTestMultiThreads(bool withTimeout)
    {
        Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(N_THREADS);
        for (int32_t i = 0; i < threads.size(); ++i)
        {
            threads[i] = newLucene<TimeLimitingThread>(withTimeout, this);
            threads[i]->start();
        }
        for (int32_t i = 0; i < threads.size(); ++i)
            threads[i]->join();
    }
    
    int64_t maxTime(bool multiThreaded)
    {
        int64_t res = 2 * TimeLimitingCollector::getResolution() + TIME_ALLOWED + SLOW_DOWN; // some slack for less noise in this test
        if (multiThreaded)
            res *= (int64_t)MULTI_THREAD_SLACK; // larger slack
        return res;
    }
    
    String maxTimeStr(bool multiThreaded)
    {
        StringStream buf;
        buf << L"( 2 * resolution + TIME_ALLOWED + SLOW_DOWN = 2 * " << TimeLimitingCollector::getResolution() << L" + " << TIME_ALLOWED << L" + " << SLOW_DOWN << L")";
        if (multiThreaded)
            buf << L" * " << MULTI_THREAD_SLACK;
        return StringUtils::toString(maxTime(multiThreaded)) + L" = " + buf.str();
    }
    
    CollectorPtr createTimedCollector(MyHitCollectorPtr hc, int64_t timeAllowed, bool greedy)
    {
        TimeLimitingCollectorPtr res = newLucene<TimeLimitingCollector>(hc, timeAllowed);
        res->setGreedy(greedy); // set to true to make sure at least one doc is collected.
        return res;
    }
    
    void search(CollectorPtr collector)
    {
        searcher->search(query, collector);
    }
};

TimeLimitingThread::TimeLimitingThread(bool withTimeout, TimeLimitingCollectorFixture* fixture)
{
    this->withTimeout = withTimeout;
    this->fixture = fixture;
}
    
void TimeLimitingThread::run()
{
    try
    {
        if (withTimeout)
            fixture->doTestTimeout(true, true);
        else
            fixture->doTestSearch();
    }
    catch (LuceneException& e)
    {
        BOOST_FAIL("Unexpected exception: " << e.getError());
    }
}

const int32_t TimeLimitingCollectorFixture::SLOW_DOWN = 47;
const int64_t TimeLimitingCollectorFixture::TIME_ALLOWED = 17 * TimeLimitingCollectorFixture::SLOW_DOWN; // so searches can find about 17 docs.

// max time allowed is relaxed for multi-threading tests. 
// the multi-thread case fails when setting this to 1 (no slack) and launching many threads (>2000).  
// but this is not a real failure, just noise.
const int32_t TimeLimitingCollectorFixture::MULTI_THREAD_SLACK = 7;      

const int32_t TimeLimitingCollectorFixture::N_DOCS = 3000;
const int32_t TimeLimitingCollectorFixture::N_THREADS = 50;
    
const String TimeLimitingCollectorFixture::FIELD_NAME = L"body";

BOOST_FIXTURE_TEST_SUITE(TimeLimitingCollectorTest, TimeLimitingCollectorFixture)

/// test search correctness with no timeout
BOOST_AUTO_TEST_CASE(testSearch)
{
    doTestSearch();
}

/// Test that timeout is obtained, and soon enough
BOOST_AUTO_TEST_CASE(testTimeoutGreedy)
{
    doTestTimeout(false, true);
}

/// Test that timeout is obtained, and soon enough
BOOST_AUTO_TEST_CASE(testTimeoutNotGreedy)
{
    doTestTimeout(false, false);
}

/// Test timeout behavior when resolution is modified.
BOOST_AUTO_TEST_CASE(testModifyResolution)
{
    // increase and test
    int64_t resolution = 20 * TimeLimitingCollector::DEFAULT_RESOLUTION; // 400
    TimeLimitingCollector::setResolution(resolution);
    BOOST_CHECK_EQUAL(resolution, TimeLimitingCollector::getResolution());
    doTestTimeout(false, true);
    // decrease much and test
    resolution = 5;
    TimeLimitingCollector::setResolution(resolution);
    BOOST_CHECK_EQUAL(resolution, TimeLimitingCollector::getResolution());
    doTestTimeout(false, true);
    // return to default and test
    resolution = TimeLimitingCollector::DEFAULT_RESOLUTION;
    TimeLimitingCollector::setResolution(resolution);
    BOOST_CHECK_EQUAL(resolution, TimeLimitingCollector::getResolution());
    doTestTimeout(false, true);
}

/// Test correctness with multiple searching threads.
BOOST_AUTO_TEST_CASE(testSearchMultiThreaded)
{
    doTestMultiThreads(false);
}

/// Test correctness with multiple searching threads.
BOOST_AUTO_TEST_CASE(testTimeoutMultiThreaded)
{
    doTestMultiThreads(true);
}

BOOST_AUTO_TEST_SUITE_END()
