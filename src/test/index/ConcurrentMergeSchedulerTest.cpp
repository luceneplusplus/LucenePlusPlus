/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "SimpleAnalyzer.h"
#include "WhitespaceAnalyzer.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "ConcurrentMergeScheduler.h"
#include "_ConcurrentMergeScheduler.h"
#include "Document.h"
#include "Field.h"
#include "LogDocMergePolicy.h"
#include "Term.h"
#include "SegmentInfos.h"
#include "IndexFileDeleter.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "TestPoint.h"

using namespace Lucene;

typedef LuceneTestFixture ConcurrentMergeSchedulerTest;

static bool mergeCalled = false;
static bool mergeThreadCreated = false;
static bool excCalled = false;

static void checkNoUnreferencedFiles(const DirectoryPtr& dir) {
    HashSet<String> _startFiles = dir->listAll();
    SegmentInfosPtr infos = newLucene<SegmentInfos>();
    infos->read(dir);
    IndexFileDeleterPtr deleter = newLucene<IndexFileDeleter>(dir, newLucene<KeepOnlyLastCommitDeletionPolicy>(), infos, InfoStreamPtr(), DocumentsWriterPtr(), HashSet<String>());
    HashSet<String> _endFiles = dir->listAll();

    Collection<String> startFiles = Collection<String>::newInstance(_startFiles.begin(), _startFiles.end());
    Collection<String> endFiles = Collection<String>::newInstance(_endFiles.begin(), _endFiles.end());

    std::sort(startFiles.begin(), startFiles.end());
    std::sort(endFiles.begin(), endFiles.end());

    EXPECT_TRUE(startFiles.equals(endFiles));
}

namespace TestFlushException {

DECLARE_SHARED_PTR(FailOnlyOnFlush)

class FailOnlyOnFlush : public MockDirectoryFailure {
public:
    FailOnlyOnFlush() {
        hitExc = false;
        mainThread = LuceneThread::currentId();
        TestPoint::clear();
    }

    virtual ~FailOnlyOnFlush() {
    }

public:
    bool hitExc;
    int64_t mainThread;

public:
    virtual void setDoFail() {
        MockDirectoryFailure::setDoFail();
        hitExc = false;
    }

    virtual void clearDoFail() {
        MockDirectoryFailure::clearDoFail();
        this->doFail = false;
    }

    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (this->doFail && mainThread == LuceneThread::currentId() && TestPoint::getTestPoint(L"doFlush")) {
            hitExc = true;
            boost::throw_exception(IOException(L"now failing during flush"));
        }
    }
};

DECLARE_SHARED_PTR(TestableIndexWriter)

class TestableIndexWriter : public IndexWriter {
public:
    TestableIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
    }

    virtual ~TestableIndexWriter() {
    }

    LUCENE_CLASS(TestableIndexWriter);

public:
    using IndexWriter::flush;
};

}

/// Make sure running background merges still work fine even when we are hitting exceptions during flushing.
TEST_F(ConcurrentMergeSchedulerTest, testFlushExceptions) {
    MockRAMDirectoryPtr directory = newLucene<MockRAMDirectory>();
    TestFlushException::FailOnlyOnFlushPtr failure = newLucene<TestFlushException::FailOnlyOnFlush>();
    directory->failOn(failure);

    TestFlushException::TestableIndexWriterPtr writer = newLucene<TestFlushException::TestableIndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();
    writer->setMergeScheduler(cms);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    doc->add(idField);
    int32_t extraCount = 0;

    for (int32_t i = 0; i < 10; ++i) {
        for (int32_t j = 0; j < 20; ++j) {
            idField->setValue(StringUtils::toString(i * 20 + j));
            writer->addDocument(doc);
        }

        // must cycle here because sometimes the merge flushes the doc we just added and so there's nothing to
        // flush, and we don't hit the exception
        while (true) {
            try {
                writer->addDocument(doc);
                failure->setDoFail();
                writer->flush(true, false, true);
                EXPECT_TRUE(!failure->hitExc);
                ++extraCount;
            } catch (LuceneException&) {
                failure->clearDoFail();
                break;
            }
        }
    }

    writer->close();
    IndexReaderPtr reader = IndexReader::open(directory, true);
    EXPECT_EQ(200 + extraCount, reader->numDocs());
    reader->close();
    directory->close();
}

/// Test that deletes committed after a merge started and before it finishes, are correctly merged back
TEST_F(ConcurrentMergeSchedulerTest, testDeleteMerging) {
    RAMDirectoryPtr directory = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();
    writer->setMergeScheduler(cms);

    LogDocMergePolicyPtr mp = newLucene<LogDocMergePolicy>(writer);
    writer->setMergePolicy(mp);

    // Force degenerate merging so we can get a mix of merging of segments with and without deletes at the start
    mp->setMinMergeDocs(1000);

    DocumentPtr doc = newLucene<Document>();
    FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    doc->add(idField);
    for (int32_t i = 0; i < 10; ++i) {
        for (int32_t j = 0; j < 100; ++j) {
            idField->setValue(StringUtils::toString(i * 100 + j));
            writer->addDocument(doc);
        }

        int32_t delID = i;
        while (delID < 100 * (1 + i)) {
            writer->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(delID)));
            delID += 10;
        }

        writer->commit();
    }

    writer->close();
    IndexReaderPtr reader = IndexReader::open(directory, true);
    // Verify that we did not lose any deletes
    EXPECT_EQ(450, reader->numDocs());
    reader->close();
    directory->close();
}

TEST_F(ConcurrentMergeSchedulerTest, testNoExtraFiles) {
    RAMDirectoryPtr directory = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    for (int32_t i = 0; i < 7; ++i) {
        ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();
        writer->setMergeScheduler(cms);
        writer->setMaxBufferedDocs(2);

        for (int32_t j = 0; j < 21; ++j) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"content", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }

        writer->close();
        checkNoUnreferencedFiles(directory);

        // Reopen
        writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
    }

    writer->close();
    directory->close();
}

TEST_F(ConcurrentMergeSchedulerTest, testNoWaitClose) {
    RAMDirectoryPtr directory = newLucene<MockRAMDirectory>();

    DocumentPtr doc = newLucene<Document>();
    FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    doc->add(idField);

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    for (int32_t i = 0; i < 10; ++i) {
        ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();
        writer->setMergeScheduler(cms);
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(100);

        for (int32_t j = 0; j < 201; ++j) {
            idField->setValue(StringUtils::toString(i * 201 + j));
            writer->addDocument(doc);
        }

        int32_t delID = i * 201;
        for (int32_t j = 0; j < 20; ++j) {
            writer->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(delID)));
            delID += 5;
        }

        // Force a bunch of merge threads to kick off so we stress out aborting them on close
        writer->setMergeFactor(3);
        writer->addDocument(doc);
        writer->commit();

        writer->close(false);

        IndexReaderPtr reader = IndexReader::open(directory, true);
        EXPECT_EQ((1 + i) * 182, reader->numDocs());

        reader->close();

        // Reopen
        writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
    }

    writer->close();
    directory->close();

    // allow time for merge threads to finish
    LuceneThread::threadSleep(1000);
}

namespace TestSubclassConcurrentMergeScheduler {

DECLARE_SHARED_PTR(MyMergeScheduler)

class FailOnlyOnMerge : public MockDirectoryFailure {
public:
    FailOnlyOnMerge() {
        TestPoint::clear();
    }

    virtual ~FailOnlyOnMerge() {
    }

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (TestPoint::getTestPoint(L"doMerge")) {
            boost::throw_exception(IOException(L"now failing during merge"));
        }
    }
};

class MyMergeThread : public MergeThread {
public:
    MyMergeThread(const ConcurrentMergeSchedulerPtr& merger, const IndexWriterPtr& writer, const OneMergePtr& startMerge) : MergeThread(merger, writer, startMerge) {
        mergeThreadCreated = true;
    }

    virtual ~MyMergeThread() {
    }
};

class MyMergeScheduler : public ConcurrentMergeScheduler {
public:
    virtual ~MyMergeScheduler() {
    }

    LUCENE_CLASS(MyMergeScheduler);

protected:
    virtual MergeThreadPtr getMergeThread(const IndexWriterPtr& writer, const OneMergePtr& merge) {
        MergeThreadPtr thread = newLucene<MyMergeThread>(shared_from_this(), writer, merge);
        thread->setThreadPriority(getMergeThreadPriority());
        return thread;
    }

    virtual void handleMergeException(const LuceneException& exc) {
        excCalled = true;
    }

    virtual void doMerge(const OneMergePtr& merge) {
        mergeCalled = true;
        ConcurrentMergeScheduler::doMerge(merge);
    }
};

}

TEST_F(ConcurrentMergeSchedulerTest, testSubclassConcurrentMergeScheduler) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    dir->failOn(newLucene<TestSubclassConcurrentMergeScheduler::FailOnlyOnMerge>());

    DocumentPtr doc = newLucene<Document>();
    FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    doc->add(idField);

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    TestSubclassConcurrentMergeScheduler::MyMergeSchedulerPtr ms = newLucene<TestSubclassConcurrentMergeScheduler::MyMergeScheduler>();
    writer->setMergeScheduler(ms);
    writer->setMaxBufferedDocs(2);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
    for (int32_t i = 0; i < 20; ++i) {
        writer->addDocument(doc);
    }

    ms->sync();
    writer->close();

    EXPECT_TRUE(mergeThreadCreated);
    EXPECT_TRUE(mergeCalled);
    EXPECT_TRUE(excCalled);
    dir->close();
    EXPECT_TRUE(ConcurrentMergeScheduler::anyUnhandledExceptions());
}
