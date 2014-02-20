/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMFile.h"
#include "RAMOutputStream.h"
#include "RAMInputStream.h"
#include "FSDirectory.h"
#include "IndexInput.h"
#include "IndexOutput.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "Document.h"
#include "WhitespaceAnalyzer.h"
#include "Field.h"
#include "MockRAMDirectory.h"
#include "LuceneThread.h"
#include "FileUtils.h"

using namespace Lucene;

class RAMDirectoryTest : public LuceneTestFixture {
public:
    RAMDirectoryTest() {
        indexDir = FileUtils::joinPath(getTempDir(), L"RAMDirIndex");
        DirectoryPtr dir(FSDirectory::open(indexDir));
        IndexWriterPtr writer(newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED));

        // add enough document so that the index will be larger than RAMDirectory::READ_BUFFER_SIZE
        docsToAdd = 500;

        // add some documents
        for (int32_t i = 0; i < docsToAdd; ++i) {
            DocumentPtr doc(newLucene<Document>());
            doc->add(newLucene<Field>(L"content", intToEnglish(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        EXPECT_EQ(docsToAdd, writer->maxDoc());

        writer->close();
        dir->close();
    }

    virtual ~RAMDirectoryTest() {
        FileUtils::removeDirectory(indexDir);
    }

protected:
    String indexDir;
    int32_t docsToAdd;
};

class TestRAMDirectoryThread : public LuceneThread {
public:
    TestRAMDirectoryThread(const IndexWriterPtr& writer, int32_t num) {
        this->writer = writer;
        this->num = num;
    }

    LUCENE_CLASS(TestRAMDirectoryThread);

public:
    static const int32_t numThreads;
    static const int32_t docsPerThread;

protected:
    IndexWriterPtr writer;
    int32_t num;

public:
    virtual void run() {
        for (int32_t j = 1; j < docsPerThread; ++j) {
            DocumentPtr doc(newLucene<Document>());
            doc->add(newLucene<Field>(L"sizeContent", intToEnglish((num * docsPerThread) + j), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            try {
                writer->addDocument(doc);
            } catch (LuceneException& e) {
                boost::throw_exception(RuntimeException(e.getError()));
            }
        }
    }
};

const int32_t TestRAMDirectoryThread::numThreads = 10;
const int32_t TestRAMDirectoryThread::docsPerThread = 40;

typedef boost::shared_ptr<TestRAMDirectoryThread> TestRAMDirectoryThreadPtr;

/// Fake a huge ram file by using the same byte buffer for all buffers under INT_MAX.
class DenseRAMFile : public RAMFile {
public:
    DenseRAMFile() {
        capacity = 0;
        singleBuffers = MapIntByteArray::newInstance();
    }

    LUCENE_CLASS(DenseRAMFile);

public:
    static const int64_t MAX_VALUE;

protected:
    int64_t capacity;
    MapIntByteArray singleBuffers;

protected:
    virtual ByteArray newBuffer(int32_t size) {
        capacity += size;
        if (capacity <= MAX_VALUE) {
            // below INT_MAX we reuse buffers
            ByteArray buf(singleBuffers.get(size));
            if (!buf) {
                buf = ByteArray::newInstance(size);
                singleBuffers.put(size, buf);
            }
            return buf;
        }
        return RAMFile::newBuffer(size);
    }
};

const int64_t DenseRAMFile::MAX_VALUE = 2 * (int64_t)INT_MAX;

typedef boost::shared_ptr<DenseRAMFile> DenseRAMFilePtr;

TEST_F(RAMDirectoryTest, testRAMDirectory) {
    DirectoryPtr dir(FSDirectory::open(indexDir));
    MockRAMDirectoryPtr ramDir(newLucene<MockRAMDirectory>(dir));

    // close the underlaying directory
    dir->close();

    // Check size
    EXPECT_EQ(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

    // open reader to test document count
    IndexReaderPtr reader(IndexReader::open(ramDir, true));
    EXPECT_EQ(docsToAdd, reader->numDocs());

    // open search to check if all doc's are there
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);

    // search for all documents
    for (int32_t i = 0; i < docsToAdd; ++i) {
        DocumentPtr doc = searcher->doc(i);
        EXPECT_TRUE(doc->getField(L"content"));
    }

    // cleanup
    reader->close();
    searcher->close();
}

TEST_F(RAMDirectoryTest, testRAMDirectorySize) {
    DirectoryPtr dir(FSDirectory::open(indexDir));
    MockRAMDirectoryPtr ramDir(newLucene<MockRAMDirectory>(dir));
    dir->close();

    IndexWriterPtr writer(newLucene<IndexWriter>(ramDir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED));
    writer->optimize();

    EXPECT_EQ(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

    Collection<TestRAMDirectoryThreadPtr> threads(Collection<TestRAMDirectoryThreadPtr>::newInstance(TestRAMDirectoryThread::numThreads));
    for (int32_t i = 0; i < TestRAMDirectoryThread::numThreads; ++i) {
        threads[i] = newLucene<TestRAMDirectoryThread>(writer, i);
    }

    for (int32_t i = 0; i < TestRAMDirectoryThread::numThreads; ++i) {
        threads[i]->start();
    }
    for (int32_t i = 0; i < TestRAMDirectoryThread::numThreads; ++i) {
        threads[i]->join();
    }

    writer->optimize();
    EXPECT_EQ(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

    writer->close();
}

TEST_F(RAMDirectoryTest, testIllegalEOF) {
    RAMDirectoryPtr dir(newLucene<RAMDirectory>());
    IndexOutputPtr o(dir->createOutput(L"out"));
    ByteArray b(ByteArray::newInstance(1024));
    o->writeBytes(b.get(), 0, 1024);
    o->close();
    IndexInputPtr i(dir->openInput(L"out"));
    i->seek(1024);
    i->close();
    dir->close();
}

/// Test huge RAMFile with more than INT_MAX bytes.
TEST_F(RAMDirectoryTest, testHugeFile) {
    DenseRAMFilePtr f(newLucene<DenseRAMFile>());

    // output part
    RAMOutputStreamPtr out(newLucene<RAMOutputStream>(f));
    ByteArray b1(ByteArray::newInstance(RAMOutputStream::BUFFER_SIZE));
    ByteArray b2(ByteArray::newInstance(RAMOutputStream::BUFFER_SIZE / 3));

    for (int32_t i = 0; i < b1.size(); ++i) {
        b1[i] = (uint8_t)(i & 0x0007f);
    }
    for (int32_t i = 0; i < b2.size(); ++i) {
        b2[i] = (uint8_t)(i & 0x0003f);
    }
    int64_t n = 0;
    EXPECT_EQ(n, out->length()); // output length must match
    while (n <= DenseRAMFile::MAX_VALUE - b1.size()) {
        out->writeBytes(b1.get(), 0, b1.size());
        out->flush();
        n += b1.size();
        EXPECT_EQ(n, out->length()); // output length must match
    }
    int32_t m = b2.size();
    int64_t l = 12;
    for (int32_t j = 0; j < l; ++j) {
        for (int32_t i = 0; i < b2.size(); ++i) {
            b2[i]++;
        }

        out->writeBytes(b2.get(), 0, m);
        out->flush();
        n += m;
        EXPECT_EQ(n, out->length()); // output length must match
    }
    out->close();
    // input part
    RAMInputStreamPtr in(newLucene<RAMInputStream>(f));
    EXPECT_EQ(n, in->length()); // input length must match
    for (int32_t j = 0; j < l; ++j) {
        int64_t loc = n - (l - j) * m;
        in->seek(loc / 3);
        in->seek(loc);
        for (int32_t i = 0; i < m; ++i) {
            uint8_t bt = in->readByte();
            uint8_t expected = (uint8_t)(1 + j + (i & 0x0003f));
            EXPECT_EQ(expected, bt); // must read same value that was written
        }
    }
}
