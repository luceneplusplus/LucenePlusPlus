/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "TermDocs.h"
#include "Term.h"
#include "TermQuery.h"
#include "LuceneThread.h"
#include "ConcurrentMergeScheduler.h"
#include "IndexSearcher.h"
#include "TopDocs.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterReaderTest;

DECLARE_SHARED_PTR(TestableIndexWriter)
DECLARE_SHARED_PTR(AddDirectoriesThread)
DECLARE_SHARED_PTR(AddDirectoriesThreads)
DECLARE_SHARED_PTR(HeavyAtomicInt)

static DocumentPtr createDocument(int32_t n, const String& indexName, int32_t numFields) {
    StringStream sb;
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"id", StringUtils::toString(n), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"indexname", indexName, Field::STORE_YES, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    sb << L"a" << n;
    doc->add(newLucene<Field>(L"field1", sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    sb << L" b" << n;
    for (int32_t i = 1; i < numFields; ++i) {
        doc->add(newLucene<Field>(L"field" + StringUtils::toString(i + 1), sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    }
    return doc;
}

static void createIndexNoClose(bool multiSegment, const String& indexName, const IndexWriterPtr& w) {
    for (int32_t i = 0; i < 100; ++i) {
        w->addDocument(createDocument(i, indexName, 4));
    }
    if (!multiSegment) {
        w->optimize();
    }
}

static int32_t count(const TermPtr& t, const IndexReaderPtr& r) {
    int32_t count = 0;
    TermDocsPtr td = r->termDocs(t);
    while (td->next()) {
        td->doc();
        ++count;
    }
    td->close();
    return count;
}

class TestableIndexWriter : public IndexWriter {
public:
    TestableIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, int32_t mfl) : IndexWriter(d, a, mfl) {
    }

    virtual ~TestableIndexWriter() {
    }

    LUCENE_CLASS(TestableIndexWriter);

public:
    using IndexWriter::flush;
};

class HeavyAtomicInt : public LuceneObject {
public:
    HeavyAtomicInt(int32_t start) {
        value = start;
    }

    virtual ~HeavyAtomicInt() {

    }

protected:
    int32_t value;

public:
    int32_t addAndGet(int32_t inc) {
        SyncLock syncLock(this);
        value += inc;
        return value;
    }

    int32_t incrementAndGet() {
        SyncLock syncLock(this);
        return ++value;
    }

    int32_t intValue() {
        SyncLock syncLock(this);
        return value;
    }
};

class AddDirectoriesThread : public LuceneThread {
public:
    AddDirectoriesThread(const AddDirectoriesThreadsPtr& addDirectories, int32_t numIter) {
        this->_addDirectories = addDirectories;
        this->numIter = numIter;
    }

    virtual ~AddDirectoriesThread() {
    }

    LUCENE_CLASS(AddDirectoriesThread);

protected:
    AddDirectoriesThreadsWeakPtr _addDirectories;
    int32_t numIter;

public:
    virtual void run();
};

class AddDirectoriesThreads : public LuceneObject {
public:
    AddDirectoriesThreads(int32_t numDirs, const IndexWriterPtr& mainWriter) {
        this->numDirs = numDirs;
        this->mainWriter = mainWriter;
        threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREADS);
        failures = Collection<LuceneException>::newInstance();
        didClose = false;
        count = newLucene<HeavyAtomicInt>(0);
        numAddIndexesNoOptimize = newLucene<HeavyAtomicInt>(0);
        addDir = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(addDir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(2);
        for (int32_t i = 0; i < NUM_INIT_DOCS; ++i) {
            DocumentPtr doc = createDocument(i, L"addindex", 4);
            writer->addDocument(doc);
        }

        writer->close();

        readers = Collection<IndexReaderPtr>::newInstance(numDirs);
        for (int32_t i = 0; i < numDirs; ++i) {
            readers[i] = IndexReader::open(addDir, false);
        }
    }

    virtual ~AddDirectoriesThreads() {
    }

    LUCENE_CLASS(AddDirectoriesThreads);

public:
    static const int32_t NUM_THREADS;
    static const int32_t NUM_INIT_DOCS;

    DirectoryPtr addDir;
    int32_t numDirs;
    Collection<LuceneThreadPtr> threads;
    IndexWriterPtr mainWriter;
    Collection<LuceneException> failures;
    Collection<IndexReaderPtr> readers;
    bool didClose;
    HeavyAtomicIntPtr count;
    HeavyAtomicIntPtr numAddIndexesNoOptimize;

public:
    void joinThreads() {
        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->join();
        }
    }

    void close(bool doWait) {
        didClose = true;
        mainWriter->close(doWait);
    }

    void closeDir() {
        for (int32_t i = 0; i < numDirs; ++i) {
            readers[i]->close();
        }
        addDir->close();
    }

    void handle(const LuceneException& t) {
        FAIL() << t.getError();
        SyncLock syncLock(&failures);
        failures.add(t);
    }

    void launchThreads(int32_t numIter) {
        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i] = newLucene<AddDirectoriesThread>(shared_from_this(), numIter);
        }
        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->start();
        }
    }

    void doBody(int32_t j, Collection<DirectoryPtr> dirs) {
        switch (j % 4) {
        case 0:
            mainWriter->addIndexesNoOptimize(dirs);
            mainWriter->optimize();
            break;
        case 1:
            mainWriter->addIndexesNoOptimize(dirs);
            numAddIndexesNoOptimize->incrementAndGet();
            break;
        case 2:
            mainWriter->addIndexes(readers);
            break;
        case 3:
            mainWriter->commit();
            break;
        }
        count->addAndGet(dirs.size() * NUM_INIT_DOCS);
    }
};

const int32_t AddDirectoriesThreads::NUM_THREADS = 5;
const int32_t AddDirectoriesThreads::NUM_INIT_DOCS = 100;

void AddDirectoriesThread::run() {
    AddDirectoriesThreadsPtr addDirectories(_addDirectories);

    try {
        Collection<DirectoryPtr> dirs = Collection<DirectoryPtr>::newInstance(addDirectories->numDirs);
        for (int32_t k = 0; k < addDirectories->numDirs; ++k) {
            dirs[k] = newLucene<MockRAMDirectory>(addDirectories->addDir);
        }
        for (int32_t x = 0; x < numIter; ++x) {
            // only do addIndexesNoOptimize
            addDirectories->doBody(x, dirs);
        }
    } catch (LuceneException& e) {
        addDirectories->handle(e);
    }
}

TEST_F(IndexWriterReaderTest, testUpdateDocument) {
    bool optimize = true;

    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    // create the index
    createIndexNoClose(!optimize, L"index1", writer);

    // get a reader
    IndexReaderPtr r1 = writer->getReader();
    EXPECT_TRUE(r1->isCurrent());

    String id10 = r1->document(10)->getField(L"id")->stringValue();

    DocumentPtr newDoc = r1->document(10);
    newDoc->removeField(L"id");
    newDoc->add(newLucene<Field>(L"id", StringUtils::toString(8000), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    writer->updateDocument(newLucene<Term>(L"id", id10), newDoc);
    EXPECT_TRUE(!r1->isCurrent());

    IndexReaderPtr r2 = writer->getReader();
    EXPECT_TRUE(r2->isCurrent());
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id10), r2));
    EXPECT_EQ(1, count(newLucene<Term>(L"id", StringUtils::toString(8000)), r2));

    r1->close();
    writer->close();
    EXPECT_TRUE(r2->isCurrent());

    IndexReaderPtr r3 = IndexReader::open(dir1, true);
    EXPECT_TRUE(r3->isCurrent());
    EXPECT_TRUE(r2->isCurrent());
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id10), r3));
    EXPECT_EQ(1, count(newLucene<Term>(L"id", StringUtils::toString(8000)), r3));

    writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    EXPECT_TRUE(r2->isCurrent());
    EXPECT_TRUE(r3->isCurrent());

    writer->close();

    EXPECT_TRUE(!r2->isCurrent());
    EXPECT_TRUE(!r3->isCurrent());

    r2->close();
    r3->close();

    dir1->close();
}

TEST_F(IndexWriterReaderTest, testAddIndexes) {
    bool optimize = false;

    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    TestableIndexWriterPtr writer = newLucene<TestableIndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    // create the index
    createIndexNoClose(!optimize, L"index1", writer);
    writer->flush(false, true, true);

    // create a 2nd index
    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    TestableIndexWriterPtr writer2 = newLucene<TestableIndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    createIndexNoClose(!optimize, L"index2", writer2);
    writer2->close();

    IndexReaderPtr r0 = writer->getReader();
    EXPECT_TRUE(r0->isCurrent());

    writer->addIndexesNoOptimize(newCollection<DirectoryPtr>(dir2));
    EXPECT_TRUE(!r0->isCurrent());
    r0->close();

    IndexReaderPtr r1 = writer->getReader();
    EXPECT_TRUE(r1->isCurrent());

    writer->commit();
    EXPECT_TRUE(!r1->isCurrent());

    EXPECT_EQ(200, r1->maxDoc());

    int32_t index2df = r1->docFreq(newLucene<Term>(L"indexname", L"index2"));

    EXPECT_EQ(100, index2df);

    // verify the docs are from different indexes
    DocumentPtr doc5 = r1->document(5);
    EXPECT_EQ(L"index1", doc5->get(L"indexname"));
    DocumentPtr doc150 = r1->document(150);
    EXPECT_EQ(L"index2", doc150->get(L"indexname"));
    r1->close();
    writer->close();
    dir1->close();
}

TEST_F(IndexWriterReaderTest, testAddIndexes2) {
    bool optimize = false;

    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer2 = newLucene<IndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    createIndexNoClose(!optimize, L"index2", writer2);
    writer2->close();

    Collection<DirectoryPtr> dirs = newCollection<DirectoryPtr>(dir2);

    writer->addIndexesNoOptimize(dirs);
    writer->addIndexesNoOptimize(dirs);
    writer->addIndexesNoOptimize(dirs);
    writer->addIndexesNoOptimize(dirs);
    writer->addIndexesNoOptimize(dirs);

    IndexReaderPtr r1 = writer->getReader();
    EXPECT_EQ(500, r1->maxDoc());

    r1->close();
    writer->close();
    dir1->close();
}

TEST_F(IndexWriterReaderTest, testDeleteFromIndexWriter) {
    bool optimize = true;

    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    TestableIndexWriterPtr writer = newLucene<TestableIndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setReaderTermsIndexDivisor(2);

    // create the index
    createIndexNoClose(!optimize, L"index1", writer);
    writer->flush(false, true, true);

    // get a reader
    IndexReaderPtr r1 = writer->getReader();

    String id10 = r1->document(10)->getField(L"id")->stringValue();

    // deleted IW docs should not show up in the next getReader
    writer->deleteDocuments(newLucene<Term>(L"id", id10));
    IndexReaderPtr r2 = writer->getReader();
    EXPECT_EQ(1, count(newLucene<Term>(L"id", id10), r1));
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id10), r2));

    String id50 = r1->document(50)->getField(L"id")->stringValue();
    EXPECT_EQ(1, count(newLucene<Term>(L"id", id50), r1));

    writer->deleteDocuments(newLucene<Term>(L"id", id50));

    IndexReaderPtr r3 = writer->getReader();
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id10), r3));
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id50), r3));

    String id75 = r1->document(75)->getField(L"id")->stringValue();
    writer->deleteDocuments(newLucene<TermQuery>(newLucene<Term>(L"id", id75)));
    IndexReaderPtr r4 = writer->getReader();
    EXPECT_EQ(1, count(newLucene<Term>(L"id", id75), r3));
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id75), r4));

    r1->close();
    r2->close();
    r3->close();
    r4->close();
    writer->close();

    // reopen the writer to verify the delete made it to the directory
    writer = newLucene<TestableIndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    IndexReaderPtr w2r1 = writer->getReader();
    EXPECT_EQ(0, count(newLucene<Term>(L"id", id10), w2r1));
    w2r1->close();
    writer->close();
    dir1->close();
}

TEST_F(IndexWriterReaderTest, testAddIndexesAndDoDeletesThreads) {
    int32_t numIter = 5;
    int32_t numDirs = 3;

    DirectoryPtr mainDir = newLucene<MockRAMDirectory>();
    IndexWriterPtr mainWriter = newLucene<IndexWriter>(mainDir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    AddDirectoriesThreadsPtr addDirThreads = newLucene<AddDirectoriesThreads>(numIter, mainWriter);
    addDirThreads->launchThreads(numDirs);
    addDirThreads->joinThreads();

    EXPECT_EQ(addDirThreads->count->intValue(), addDirThreads->mainWriter->numDocs());

    addDirThreads->close(true);

    EXPECT_TRUE(addDirThreads->failures.empty());

    checkIndex(mainDir);

    IndexReaderPtr reader = IndexReader::open(mainDir, true);
    EXPECT_EQ(addDirThreads->count->intValue(), reader->numDocs());
    reader->close();

    addDirThreads->closeDir();
    mainDir->close();
}

/// Tests creating a segment, then check to insure the segment can be seen via IndexWriter.getReader
static void doTestIndexWriterReopenSegment(bool optimize) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    TestableIndexWriterPtr writer = newLucene<TestableIndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    IndexReaderPtr r1 = writer->getReader();
    EXPECT_EQ(0, r1->maxDoc());
    createIndexNoClose(false, L"index1", writer);
    writer->flush(!optimize, true, true);

    IndexReaderPtr iwr1 = writer->getReader();
    EXPECT_EQ(100, iwr1->maxDoc());

    IndexReaderPtr r2 = writer->getReader();
    EXPECT_EQ(100, r2->maxDoc());

    // add 100 documents
    for (int32_t x = 10000; x < 10000 + 100; ++x) {
        DocumentPtr d = createDocument(x, L"index1", 5);
        writer->addDocument(d);
    }
    writer->flush(false, true, true);
    // verify the reader was reopened internally
    IndexReaderPtr iwr2 = writer->getReader();
    EXPECT_NE(iwr2, r1);
    EXPECT_EQ(200, iwr2->maxDoc());
    // should have flushed out a segment
    IndexReaderPtr r3 = writer->getReader();
    EXPECT_NE(r2, r3);
    EXPECT_EQ(200, r3->maxDoc());

    // dec ref the readers rather than close them because closing flushes changes to the writer
    r1->close();
    iwr1->close();
    r2->close();
    r3->close();
    iwr2->close();
    writer->close();

    // test whether the changes made it to the directory
    writer = newLucene<TestableIndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    IndexReaderPtr w2r1 = writer->getReader();
    // insure the deletes were actually flushed to the directory
    EXPECT_EQ(200, w2r1->maxDoc());
    w2r1->close();
    writer->close();

    dir1->close();
}

TEST_F(IndexWriterReaderTest, testIndexWriterReopenSegmentOptimize) {
    doTestIndexWriterReopenSegment(true);
}

TEST_F(IndexWriterReaderTest, testIndexWriterReopenSegment) {
    doTestIndexWriterReopenSegment(false);
}

namespace TestMergeWarmer {

DECLARE_SHARED_PTR(MyWarmer)

class MyWarmer : public IndexReaderWarmer {
public:
    MyWarmer() {
        warmCount = 0;
    }

    virtual ~MyWarmer() {
    }

    LUCENE_CLASS(MyWarmer);

public:
    int32_t warmCount;

public:
    virtual void warm(const IndexReaderPtr& reader) {
        ++warmCount;
    }
};

}

TEST_F(IndexWriterReaderTest, testMergeWarmer) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    // create the index
    createIndexNoClose(false, L"test", writer);

    // get a reader to put writer into near real-time mode
    IndexReaderPtr r1 = writer->getReader();

    // Enroll warmer
    TestMergeWarmer::MyWarmerPtr warmer = newLucene<TestMergeWarmer::MyWarmer>();
    writer->setMergedSegmentWarmer(warmer);
    writer->setMergeFactor(2);
    writer->setMaxBufferedDocs(2);

    for (int32_t i = 0; i < 100; ++i) {
        writer->addDocument(createDocument(i, L"test", 4));
    }

    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();

    EXPECT_TRUE(warmer->warmCount > 0);
    int32_t count = warmer->warmCount;

    writer->addDocument(createDocument(17, L"test", 4));
    writer->optimize();
    EXPECT_TRUE(warmer->warmCount > count);

    writer->close();
    r1->close();
    dir1->close();
}

TEST_F(IndexWriterReaderTest, testAfterCommit) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    // create the index
    createIndexNoClose(false, L"test", writer);

    // get a reader to put writer into near real-time mode
    IndexReaderPtr r1 = writer->getReader();
    checkIndex(dir1);
    writer->commit();
    checkIndex(dir1);
    EXPECT_EQ(100, r1->numDocs());

    for (int32_t i = 0; i < 10; ++i) {
        writer->addDocument(createDocument(i, L"test", 4));
    }

    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();

    IndexReaderPtr r2 = r1->reopen();
    if (r2 != r1) {
        r1->close();
        r1 = r2;
    }

    EXPECT_EQ(110, r1->numDocs());
    writer->close();
    r1->close();
    dir1->close();
}

/// Make sure reader remains usable even if IndexWriter closes
TEST_F(IndexWriterReaderTest, testAfterClose) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    // create the index
    createIndexNoClose(false, L"test", writer);

    IndexReaderPtr r = writer->getReader();
    writer->close();

    checkIndex(dir1);

    // reader should remain usable even after IndexWriter is closed
    EXPECT_EQ(100, r->numDocs());
    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"indexname", L"test"));
    EXPECT_EQ(100, newLucene<IndexSearcher>(r)->search(q, 10)->totalHits);

    try {
        r->reopen();
    } catch (AlreadyClosedException& e) {
        EXPECT_TRUE(check_exception(LuceneException::AlreadyClosed)(e));
    }

    r->close();
    dir1->close();
}

namespace TestDuringAddIndexes {

class AddIndexesThread : public LuceneThread {
public:
    AddIndexesThread(int64_t endTime, const IndexWriterPtr& writer,  Collection<DirectoryPtr> dirs) {
        this->endTime = endTime;
        this->writer = writer;
        this->dirs = dirs;
    }

    virtual ~AddIndexesThread() {
    }

    LUCENE_CLASS(AddIndexesThread);

protected:
    int64_t endTime;
    IndexWriterPtr writer;
    Collection<DirectoryPtr> dirs;

public:
    virtual void run() {
        while ((int64_t)MiscUtils::currentTimeMillis() < endTime) {
            try {
                writer->addIndexesNoOptimize(dirs);
            } catch (LuceneException& e) {
                FAIL() << "Unexpected exception: " << e.getError();
            }
        }
    }
};

}

/// Stress test reopen during addIndexes
TEST_F(IndexWriterReaderTest, testDuringAddIndexes) {
    MockRAMDirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMergeFactor(2);

    // create the index
    createIndexNoClose(false, L"test", writer);
    writer->commit();

    Collection<DirectoryPtr> dirs = Collection<DirectoryPtr>::newInstance(10);
    for (int32_t i = 0; i < 10; ++i) {
        dirs[i] = newLucene<MockRAMDirectory>(dir1);
    }

    IndexReaderPtr r = writer->getReader();

    int32_t NUM_THREAD = 5;
    int32_t SECONDS = 3;

    int64_t endTime = MiscUtils::currentTimeMillis() + 1000 * SECONDS;

    Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREAD);
    for (int32_t i = 0; i < NUM_THREAD; ++i) {
        threads[i] = newLucene<TestDuringAddIndexes::AddIndexesThread>(endTime, writer, dirs);
        threads[i]->start();
    }

    int32_t lastCount = 0;
    while ((int64_t)MiscUtils::currentTimeMillis() < endTime) {
        IndexReaderPtr r2 = r->reopen();
        if (r2 != r) {
            r->close();
            r = r2;
        }
        QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"indexname", L"test"));
        int32_t count = newLucene<IndexSearcher>(r)->search(q, 10)->totalHits;
        EXPECT_TRUE(count >= lastCount);
        lastCount = count;
    }

    for (int32_t i = 0; i < NUM_THREAD; ++i) {
        threads[i]->join();
    }

    writer->close();
    r->close();
    EXPECT_EQ(0, dir1->getOpenDeletedFiles().size());

    checkIndex(dir1);
    dir1->close();
}

namespace TestDuringAddDelete {

class AddDeleteThread : public LuceneThread {
public:
    AddDeleteThread(int64_t endTime, const IndexWriterPtr& writer) {
        this->endTime = endTime;
        this->writer = writer;
        this->random = newLucene<Random>();
    }

    virtual ~AddDeleteThread() {
    }

    LUCENE_CLASS(AddDeleteThread);

protected:
    int64_t endTime;
    IndexWriterPtr writer;
    RandomPtr random;

public:
    virtual void run() {
        int32_t count = 0;
        while ((int64_t)MiscUtils::currentTimeMillis() < endTime) {
            try {
                for (int32_t docUpto = 0; docUpto < 10; ++docUpto) {
                    writer->addDocument(createDocument(10 * count + docUpto, L"test", 4));
                }
                ++count;
                int32_t limit = count * 10;
                for (int32_t delUpto = 0; delUpto < 5; ++delUpto) {
                    int32_t x = random->nextInt(limit);
                    writer->deleteDocuments(newLucene<Term>(L"field3", L"b" + StringUtils::toString(x)));
                }
            } catch (LuceneException& e) {
                FAIL() << "Unexpected exception: " << e.getError();
            }
        }
    }
};

}

/// Stress test reopen during add/delete
TEST_F(IndexWriterReaderTest, testDuringAddDelete) {
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir1, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMergeFactor(2);

    // create the index
    createIndexNoClose(false, L"test", writer);
    writer->commit();

    IndexReaderPtr r = writer->getReader();

    int32_t NUM_THREAD = 5;
    int32_t SECONDS = 3;

    int64_t endTime = MiscUtils::currentTimeMillis() + 1000 * SECONDS;

    Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREAD);
    for (int32_t i = 0; i < NUM_THREAD; ++i) {
        threads[i] = newLucene<TestDuringAddDelete::AddDeleteThread>(endTime, writer);
        threads[i]->start();
    }

    int32_t sum = 0;
    while ((int64_t)MiscUtils::currentTimeMillis() < endTime) {
        IndexReaderPtr r2 = r->reopen();
        if (r2 != r) {
            r->close();
            r = r2;
        }
        QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"indexname", L"test"));
        sum += newLucene<IndexSearcher>(r)->search(q, 10)->totalHits;
    }

    for (int32_t i = 0; i < NUM_THREAD; ++i) {
        threads[i]->join();
    }

    EXPECT_TRUE(sum > 0);

    writer->close();

    checkIndex(dir1);
    r->close();
    dir1->close();
}

TEST_F(IndexWriterReaderTest, testExpungeDeletes) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED));
    FieldPtr id = newLucene<Field>(L"id", L"", Field::STORE_NO, Field::INDEX_ANALYZED);
    doc->add(id);
    id->setValue(L"0");
    w->addDocument(doc);
    id->setValue(L"1");
    w->addDocument(doc);
    w->deleteDocuments(newLucene<Term>(L"id", L"0"));

    IndexReaderPtr r = w->getReader();
    w->expungeDeletes();
    w->close();
    r->close();
    r = IndexReader::open(dir, true);
    EXPECT_EQ(1, r->numDocs());
    EXPECT_TRUE(!r->hasDeletions());
    r->close();
    dir->close();
}

TEST_F(IndexWriterReaderTest, testDeletesNumDocs) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED));
    FieldPtr id = newLucene<Field>(L"id", L"", Field::STORE_NO, Field::INDEX_NOT_ANALYZED);
    doc->add(id);
    id->setValue(L"0");
    w->addDocument(doc);
    id->setValue(L"1");
    w->addDocument(doc);
    IndexReaderPtr r = w->getReader();
    EXPECT_EQ(2, r->numDocs());
    r->close();

    w->deleteDocuments(newLucene<Term>(L"id", L"0"));
    r = w->getReader();
    EXPECT_EQ(1, r->numDocs());
    r->close();

    w->deleteDocuments(newLucene<Term>(L"id", L"1"));
    r = w->getReader();
    EXPECT_EQ(0, r->numDocs());
    r->close();

    w->close();
    dir->close();
}

namespace TestSegmentWarmer {

DECLARE_SHARED_PTR(SegmentWarmer)

class SegmentWarmer : public IndexReaderWarmer {
public:
    virtual ~SegmentWarmer() {
    }

    LUCENE_CLASS(SegmentWarmer);

public:
    virtual void warm(const IndexReaderPtr& reader) {
        IndexSearcherPtr s = newLucene<IndexSearcher>(reader);
        TopDocsPtr hits = s->search(newLucene<TermQuery>(newLucene<Term>(L"foo", L"bar")), 10);
        EXPECT_EQ(20, hits->totalHits);
    }
};

}

TEST_F(IndexWriterReaderTest, testSegmentWarmer) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    w->setMaxBufferedDocs(2);
    w->getReader()->close();

    w->setMergedSegmentWarmer(newLucene<TestSegmentWarmer::SegmentWarmer>());

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"foo", L"bar", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    for (int32_t i = 0; i < 20; ++i) {
        w->addDocument(doc);
    }
    w->waitForMerges();
    w->close();
    dir->close();
}
