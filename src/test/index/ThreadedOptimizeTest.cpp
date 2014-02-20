/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "SimpleAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "LuceneThread.h"
#include "Term.h"
#include "IndexReader.h"
#include "MockRAMDirectory.h"
#include "SerialMergeScheduler.h"
#include "ConcurrentMergeScheduler.h"
#include "FSDirectory.h"
#include "FileUtils.h"

using namespace Lucene;

class OptimizeThread : public LuceneThread {
public:
    OptimizeThread(int32_t numIter, int32_t iterFinal, int32_t iFinal, const IndexWriterPtr& writer, const IndexWriterPtr& writerFinal) {
        this->numIter = numIter;
        this->iterFinal = iterFinal;
        this->iFinal = iFinal;
        this->writer = writer;
        this->writerFinal = writerFinal;
    }

    virtual ~OptimizeThread() {
    }

    LUCENE_CLASS(OptimizeThread);

protected:
    int32_t numIter;
    int32_t iterFinal;
    int32_t iFinal;
    IndexWriterPtr writer;
    IndexWriterPtr writerFinal;

public:
    virtual void run() {
        try {
            for (int32_t j = 0; j < numIter; ++j) {
                writerFinal->optimize(false);
                for (int32_t k = 0; k < 17 * (1 + iFinal); ++k) {
                    DocumentPtr d = newLucene<Document>();
                    d->add(newLucene<Field>(L"id", StringUtils::toString(iterFinal) + L"_" + StringUtils::toString(iFinal) + L"_" + StringUtils::toString(j) + L"_" + StringUtils::toString(k), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
                    d->add(newLucene<Field>(L"contents", intToEnglish(iFinal + k), Field::STORE_NO, Field::INDEX_ANALYZED));
                    writer->addDocument(d);
                }
                for (int32_t k = 0; k < 9 * (1 + iFinal); ++k) {
                    writerFinal->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(iterFinal) + L"_" + StringUtils::toString(iFinal) + L"_" + StringUtils::toString(j) + L"_" + StringUtils::toString(k)));
                }
                writerFinal->optimize();
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

class ThreadedOptimizeTest : public LuceneTestFixture {
public:
    ThreadedOptimizeTest() {
        analyzer = newLucene<SimpleAnalyzer>();
    }

    virtual ~ThreadedOptimizeTest() {
    }

protected:
    static const int32_t NUM_THREADS;
    static const int32_t NUM_ITER;
    static const int32_t NUM_ITER2;

    AnalyzerPtr analyzer;

public:
    void runTest(const DirectoryPtr& directory, const MergeSchedulerPtr& merger) {
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(2);
        if (merger) {
            writer->setMergeScheduler(merger);
        }

        for (int32_t iter = 0; iter < NUM_ITER; ++iter) {
            int32_t iterFinal = iter;

            writer->setMergeFactor(1000);

            for (int32_t i = 0; i < 200; ++i) {
                DocumentPtr d = newLucene<Document>();
                d->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
                d->add(newLucene<Field>(L"contents", intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
                writer->addDocument(d);
            }

            writer->setMergeFactor(4);

            Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREADS);

            for (int32_t i = 0; i < NUM_THREADS; ++i) {
                int32_t iFinal = i;
                IndexWriterPtr writerFinal = writer;
                threads[i] = newLucene<OptimizeThread>(NUM_ITER2, iterFinal, iFinal, writer, writerFinal);
            }

            for (int32_t i = 0; i < NUM_THREADS; ++i) {
                threads[i]->start();
            }
            for (int32_t i = 0; i < NUM_THREADS; ++i) {
                threads[i]->join();
            }

            int32_t expectedDocCount = (int32_t)((1 + iter) * (200 + 8 * NUM_ITER2 * (int32_t)(((double)NUM_THREADS / 2.0) * (double)(1 + NUM_THREADS))));

            EXPECT_EQ(expectedDocCount, writer->maxDoc());

            writer->close();
            writer = newLucene<IndexWriter>(directory, analyzer, false, IndexWriter::MaxFieldLengthUNLIMITED);
            writer->setMaxBufferedDocs(2);

            IndexReaderPtr reader = IndexReader::open(directory, true);
            EXPECT_TRUE(reader->isOptimized());
            EXPECT_EQ(expectedDocCount, reader->numDocs());
            reader->close();
        }
        writer->close();
    }
};

const int32_t ThreadedOptimizeTest::NUM_THREADS = 3;
const int32_t ThreadedOptimizeTest::NUM_ITER = 1;
const int32_t ThreadedOptimizeTest::NUM_ITER2 = 1;

TEST_F(ThreadedOptimizeTest, testThreadedOptimize) {
    DirectoryPtr directory = newLucene<MockRAMDirectory>();
    runTest(directory, newLucene<SerialMergeScheduler>());
    runTest(directory, newLucene<ConcurrentMergeScheduler>());
    directory->close();

    String dirName(FileUtils::joinPath(getTempDir(), L"luceneTestThreadedOptimize"));
    directory = FSDirectory::open(dirName);
    runTest(directory, newLucene<SerialMergeScheduler>());
    runTest(directory, newLucene<ConcurrentMergeScheduler>());
    directory->close();
    FileUtils::removeDirectory(dirName);
}
