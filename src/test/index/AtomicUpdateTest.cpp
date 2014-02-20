/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef LuceneTestFixture AtomicUpdateTest;

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& dir, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(dir, a, create, mfl) {
        random = newLucene<Random>();
    }

    virtual ~MockIndexWriter() {
    }

protected:
    RandomPtr random;

public:
    virtual bool testPoint(const String& name) {
        if (random->nextInt(4) == 2) {
            LuceneThread::threadYield();
        }
        return true;
    }
};

DECLARE_SHARED_PTR(AtomicTimedThread)
DECLARE_SHARED_PTR(AtomicIndexerThread)
DECLARE_SHARED_PTR(AtomicSearcherThread)

class AtomicTimedThread : public LuceneThread {
public:
    AtomicTimedThread() {
        this->failed = false;
    }

    virtual ~AtomicTimedThread() {
    }

    LUCENE_CLASS(AtomicTimedThread);

public:
    bool failed;

protected:
    static const int32_t RUN_TIME_SEC;

public:
    virtual void doWork() = 0;

    virtual void run() {
        int64_t stopTime = MiscUtils::currentTimeMillis() + 1000 * RUN_TIME_SEC;

        try {
            while ((int64_t)MiscUtils::currentTimeMillis() < stopTime && !failed) {
                doWork();
            }
        } catch (LuceneException& e) {
            failed = true;
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

const int32_t AtomicTimedThread::RUN_TIME_SEC = 3;

class AtomicIndexerThread : public AtomicTimedThread {
public:
    AtomicIndexerThread(const IndexWriterPtr& writer) {
        this->writer = writer;
    }

    virtual ~AtomicIndexerThread() {
    }

    LUCENE_CLASS(AtomicIndexerThread);

public:
    IndexWriterPtr writer;

public:
    virtual void doWork() {
        // Update all 100 docs
        for (int32_t i = 0; i < 100; ++i) {
            DocumentPtr d = newLucene<Document>();
            d->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            d->add(newLucene<Field>(L"contents", intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->updateDocument(newLucene<Term>(L"id", StringUtils::toString(i)), d);
        }
    }
};

class AtomicSearcherThread : public AtomicTimedThread {
public:
    AtomicSearcherThread(const DirectoryPtr& directory) {
        this->directory = directory;
    }

    virtual ~AtomicSearcherThread() {
    }

    LUCENE_CLASS(AtomicSearcherThread);

protected:
    DirectoryPtr directory;

public:
    virtual void doWork() {
        IndexReaderPtr r = IndexReader::open(directory, true);
        if (r->numDocs() != 100) {
            FAIL() << "num docs failure";
        }
        r->close();
    }
};

// Run one indexer and 2 searchers against single index as stress test.
static void runTest(const DirectoryPtr& directory) {
    Collection<AtomicTimedThreadPtr> threads(Collection<AtomicTimedThreadPtr>::newInstance(4));
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();

    IndexWriterPtr writer = newLucene<MockIndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);

    writer->setMaxBufferedDocs(7);
    writer->setMergeFactor(3);

    // Establish a base index of 100 docs
    for (int32_t i = 0; i < 100; ++i) {
        DocumentPtr d = newLucene<Document>();
        d->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        d->add(newLucene<Field>(L"contents", intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
        if ((i - 1) % 7 == 0) {
            writer->commit();
        }
        writer->addDocument(d);
    }
    writer->commit();

    IndexReaderPtr r = IndexReader::open(directory, true);
    EXPECT_EQ(100, r->numDocs());
    r->close();

    AtomicIndexerThreadPtr indexerThread1 = newLucene<AtomicIndexerThread>(writer);
    threads[0] = indexerThread1;
    indexerThread1->start();

    AtomicIndexerThreadPtr indexerThread2 = newLucene<AtomicIndexerThread>(writer);
    threads[1] = indexerThread2;
    indexerThread2->start();

    AtomicSearcherThreadPtr searcherThread1 = newLucene<AtomicSearcherThread>(directory);
    threads[2] = searcherThread1;
    searcherThread1->start();

    AtomicSearcherThreadPtr searcherThread2 = newLucene<AtomicSearcherThread>(directory);
    threads[3] = searcherThread2;
    searcherThread2->start();

    indexerThread1->join();
    indexerThread2->join();
    searcherThread1->join();
    searcherThread2->join();

    writer->close();

    EXPECT_TRUE(!indexerThread1->failed); // hit unexpected exception in indexer1
    EXPECT_TRUE(!indexerThread2->failed); // hit unexpected exception in indexer2
    EXPECT_TRUE(!searcherThread1->failed); // hit unexpected exception in search1
    EXPECT_TRUE(!searcherThread2->failed); // hit unexpected exception in search2
}

/// Run above stress test against RAMDirectory.
TEST_F(AtomicUpdateTest, testAtomicUpdatesRAMDirectory) {
    DirectoryPtr directory = newLucene<MockRAMDirectory>();
    runTest(directory);
    directory->close();
}

/// Run above stress test against FSDirectory
TEST_F(AtomicUpdateTest, testAtomicUpdatesFSDirectory) {
    String dirPath(getTempDir(L"lucene.test.atomic"));
    DirectoryPtr directory = FSDirectory::open(dirPath);
    runTest(directory);
    directory->close();
    FileUtils::removeDirectory(dirPath);
}
