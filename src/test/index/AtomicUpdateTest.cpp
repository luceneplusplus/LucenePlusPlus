/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "LuceneThread.h"
#include "Document.h"
#include "Field.h"
#include "Term.h"
#include "SimpleAnalyzer.h"
#include "MockRAMDirectory.h"
#include "FSDirectory.h"
#include "Random.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(AtomicUpdateTest, LuceneTestFixture)

class MockIndexWriter : public IndexWriter
{
public:
    MockIndexWriter(DirectoryPtr dir, AnalyzerPtr a, bool create, int32_t mfl) : IndexWriter(dir, a, create, mfl)
    {
        random = newLucene<Random>();
    }
    
    virtual ~MockIndexWriter()
    {
    }

protected:
    RandomPtr random;

public:
    virtual bool testPoint(const String& name)
    {
        if (random->nextInt(4) == 2)
            LuceneThread::threadYield();
        return true;
    }
};

DECLARE_SHARED_PTR(TimedThread)
DECLARE_SHARED_PTR(IndexerThread)
DECLARE_SHARED_PTR(SearcherThread)

class TimedThread : public LuceneThread
{
public:
    TimedThread()
    {
        this->failed = false;
    }
    
    virtual ~TimedThread()
    {
    }
    
    LUCENE_CLASS(TimedThread);
    
public:
    bool failed;

protected:
    static const int32_t RUN_TIME_SEC;
    
public:
    virtual void doWork() = 0;
    
    virtual void run()
    {
        int64_t stopTime = MiscUtils::currentTimeMillis() + 1000 * RUN_TIME_SEC;
        
        try
        {
            while ((int64_t)MiscUtils::currentTimeMillis() < stopTime && !failed)
                doWork();
        }
        catch (LuceneException& e)
        {
            failed = true;
            BOOST_FAIL("Unexpected exception: " << e.getError());
        }
    }
};

const int32_t TimedThread::RUN_TIME_SEC = 3;

class IndexerThread : public TimedThread
{
public:
    IndexerThread(IndexWriterPtr writer)
    {
        this->writer = writer;
    }
    
    virtual ~IndexerThread()
    {
    }
    
    LUCENE_CLASS(IndexerThread);
    
public:
    IndexWriterPtr writer;
    
public:
    virtual void doWork()
    {
        // Update all 100 docs
        for (int32_t i = 0; i < 100; ++i)
        {
            DocumentPtr d = newLucene<Document>();
            d->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            d->add(newLucene<Field>(L"contents", intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->updateDocument(newLucene<Term>(L"id", StringUtils::toString(i)), d);
        }
    }
};

class SearcherThread : public TimedThread
{
public:
    SearcherThread(DirectoryPtr directory)
    {
        this->directory = directory;
    }
    
    virtual ~SearcherThread()
    {
    }
    
    LUCENE_CLASS(SearcherThread);
    
protected:
    DirectoryPtr directory;
    
public:
    virtual void doWork()
    {
        IndexReaderPtr r = IndexReader::open(directory, true);
        if (r->numDocs() != 100)
            BOOST_FAIL("num docs failure");
        r->close();
    }
};

// Run one indexer and 2 searchers against single index as stress test.
static void runTest(DirectoryPtr directory)
{
    Collection<TimedThreadPtr> threads(Collection<TimedThreadPtr>::newInstance(4));
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
    
    IndexWriterPtr writer = newLucene<MockIndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
    
    writer->setMaxBufferedDocs(7);
    writer->setMergeFactor(3);
    
    // Establish a base index of 100 docs
    for (int32_t i = 0; i < 100; ++i)
    {
        DocumentPtr d = newLucene<Document>();
        d->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        d->add(newLucene<Field>(L"contents", intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
        if ((i - 1) % 7 == 0)
            writer->commit();
        writer->addDocument(d);
    }
    writer->commit();
    
    IndexReaderPtr r = IndexReader::open(directory, true);
    BOOST_CHECK_EQUAL(100, r->numDocs());
    r->close();

    IndexerThreadPtr indexerThread1 = newLucene<IndexerThread>(writer);
    threads[0] = indexerThread1;
    indexerThread1->start();

    IndexerThreadPtr indexerThread2 = newLucene<IndexerThread>(writer);
    threads[1] = indexerThread2;
    indexerThread2->start();

    SearcherThreadPtr searcherThread1 = newLucene<SearcherThread>(directory);
    threads[2] = searcherThread1;
    searcherThread1->start();

    SearcherThreadPtr searcherThread2 = newLucene<SearcherThread>(directory);
    threads[3] = searcherThread2;
    searcherThread2->start();
    
    indexerThread1->join();
    indexerThread2->join();
    searcherThread1->join();
    searcherThread2->join();
    
    writer->close();

    BOOST_CHECK(!indexerThread1->failed); // hit unexpected exception in indexer1
    BOOST_CHECK(!indexerThread2->failed); // hit unexpected exception in indexer2
    BOOST_CHECK(!searcherThread1->failed); // hit unexpected exception in search1
    BOOST_CHECK(!searcherThread2->failed); // hit unexpected exception in search2
}

/// Run above stress test against RAMDirectory.
BOOST_AUTO_TEST_CASE(testAtomicUpdatesRAMDirectory)
{
    DirectoryPtr directory = newLucene<MockRAMDirectory>();
    runTest(directory);
    directory->close();
}

/// Run above stress test against FSDirectory
BOOST_AUTO_TEST_CASE(testAtomicUpdatesFSDirectory)
{
    String dirPath(getTempDir(L"lucene.test.atomic"));
    DirectoryPtr directory = FSDirectory::open(dirPath);
    runTest(directory);
    directory->close();
    FileUtils::removeDirectory(dirPath);
}

BOOST_AUTO_TEST_SUITE_END()
