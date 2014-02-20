/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "LuceneThread.h"
#include "Document.h"
#include "Field.h"
#include "CloseableThreadLocal.h"
#include "Term.h"
#include "MockRAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "ConcurrentMergeScheduler.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

class IndexWriterExceptionsTest : public LuceneTestFixture {
public:
    IndexWriterExceptionsTest() {
        random = newLucene<Random>();
        tvSettings = newCollection<Field::TermVector>(
                         Field::TERM_VECTOR_NO, Field::TERM_VECTOR_YES, Field::TERM_VECTOR_WITH_OFFSETS,
                         Field::TERM_VECTOR_WITH_POSITIONS, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS
                     );
    }

    virtual ~IndexWriterExceptionsTest() {
    }

protected:
    RandomPtr random;
    Collection<Field::TermVector> tvSettings;

public:
    Field::TermVector randomTVSetting() {
        return tvSettings[random->nextInt(tvSettings.size())];
    }
};

static CloseableThreadLocal<LuceneThread> doFail;

DECLARE_SHARED_PTR(ExceptionsIndexerThread)

class ExceptionsIndexerThread : public LuceneThread {
public:
    ExceptionsIndexerThread(const IndexWriterPtr& writer, IndexWriterExceptionsTest* fixture) {
        this->writer = writer;
        this->fixture = fixture;
        this->r = newLucene<Random>(47);
    }

    virtual ~ExceptionsIndexerThread() {
    }

    LUCENE_CLASS(ExceptionsIndexerThread);

public:
    IndexWriterPtr writer;
    IndexWriterExceptionsTest* fixture;
    LuceneException failure;
    RandomPtr r;

public:
    virtual void run() {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"content1", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_ANALYZED, fixture->randomTVSetting()));
        doc->add(newLucene<Field>(L"content6", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_ANALYZED, fixture->randomTVSetting()));
        doc->add(newLucene<Field>(L"content2", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, fixture->randomTVSetting()));
        doc->add(newLucene<Field>(L"content3", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_NO));

        doc->add(newLucene<Field>(L"content4", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_ANALYZED, fixture->randomTVSetting()));
        doc->add(newLucene<Field>(L"content5", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, fixture->randomTVSetting()));

        doc->add(newLucene<Field>(L"content7", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, fixture->randomTVSetting()));

        FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED, fixture->randomTVSetting());
        doc->add(idField);

        int64_t stopTime = MiscUtils::currentTimeMillis() + 3000;

        while ((int64_t)MiscUtils::currentTimeMillis() < stopTime) {
            doFail.set(shared_from_this());
            String id = StringUtils::toString(r->nextInt(50));
            idField->setValue(id);
            TermPtr idTerm = newLucene<Term>(L"id", id);
            try {
                writer->updateDocument(idTerm, doc);
            } catch (RuntimeException&) {
                try {
                    checkIndex(writer->getDirectory());
                } catch (IOException& ioe) {
                    failure = ioe;
                    break;
                }
            } catch (LuceneException& e) {
                failure = e;
                break;
            }

            doFail.set(LuceneThreadPtr());

            // After a possible exception (above) I should be able to add a new document
            // without hitting an exception
            try {
                writer->updateDocument(idTerm, doc);
            } catch (LuceneException& e) {
                failure = e;
                break;
            }
        }
    }
};

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& dir, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(dir, a, create, mfl) {
        this->r = newLucene<Random>(17);
    }

    virtual ~MockIndexWriter() {
    }

protected:
    RandomPtr r;

public:
    virtual bool testPoint(const String& name) {
        if (doFail.get() && name != L"startDoFlush" && r->nextInt(20) == 17) {
            boost::throw_exception(RuntimeException(L"intentionally failing at " + name));
        }
        return true;
    }
};

TEST_F(IndexWriterExceptionsTest, testRandomExceptions) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();

    writer->setRAMBufferSizeMB(0.1);

    ExceptionsIndexerThreadPtr thread = newLucene<ExceptionsIndexerThread>(writer, this);
    thread->run();

    if (!thread->failure.isNull()) {
        FAIL() << "thread hit unexpected failure";
    }

    writer->commit();

    try {
        writer->close();
    } catch (LuceneException&) {
        writer->rollback();
    }

    // Confirm that when doc hits exception partway through tokenization, it's deleted
    IndexReaderPtr r2 = IndexReader::open(dir, true);
    int32_t count = r2->docFreq(newLucene<Term>(L"content4", L"aaa"));
    int32_t count2 = r2->docFreq(newLucene<Term>(L"content4", L"ddd"));
    EXPECT_EQ(count, count2);
    r2->close();

    checkIndex(dir);
}

TEST_F(IndexWriterExceptionsTest, testRandomExceptionsThreads) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();

    writer->setRAMBufferSizeMB(0.2);

    int32_t NUM_THREADS = 4;

    Collection<ExceptionsIndexerThreadPtr> threads = Collection<ExceptionsIndexerThreadPtr>::newInstance(NUM_THREADS);
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i] = newLucene<ExceptionsIndexerThread>(writer, this);
        threads[i]->start();
    }

    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i]->join();
    }

    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        if (!threads[i]->failure.isNull()) {
            FAIL() << "thread hit unexpected failure: " << threads[i]->failure.getError();
        }
    }

    writer->commit();

    try {
        writer->close();
    } catch (LuceneException&) {
        writer->rollback();
    }

    // Confirm that when doc hits exception partway through tokenization, it's deleted
    IndexReaderPtr r2 = IndexReader::open(dir, true);
    int32_t count = r2->docFreq(newLucene<Term>(L"content4", L"aaa"));
    int32_t count2 = r2->docFreq(newLucene<Term>(L"content4", L"ddd"));
    EXPECT_EQ(count, count2);
    r2->close();

    checkIndex(dir);
}
