/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "ConcurrentMergeScheduler.h"
#include "SerialMergeScheduler.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "IndexFileDeleter.h"
#include "StandardAnalyzer.h"
#include "DocumentsWriter.h"
#include "TermPositions.h"
#include "LogDocMergePolicy.h"
#include "SegmentInfos.h"
#include "SegmentInfo.h"
#include "FSDirectory.h"
#include "IndexInput.h"
#include "IndexOutput.h"
#include "IndexFileNames.h"
#include "SingleInstanceLockFactory.h"
#include "TokenFilter.h"
#include "StandardTokenizer.h"
#include "TestPoint.h"
#include "WhitespaceTokenizer.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "PhraseQuery.h"
#include "SpanTermQuery.h"
#include "TermPositionVector.h"
#include "TermVectorOffsetInfo.h"
#include "SimpleAnalyzer.h"
#include "CachingTokenFilter.h"
#include "StringReader.h"
#include "TeeSinkTokenFilter.h"
#include "StopAnalyzer.h"
#include "Random.h"
#include "UTF8Stream.h"
#include "InfoStream.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterTest;

static void addDoc(const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void addDocWithIndex(const IndexWriterPtr& writer, int32_t index) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa " + StringUtils::toString(index), Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", StringUtils::toString(index), Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

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

DECLARE_SHARED_PTR(FailOnlyOnFlush)
DECLARE_SHARED_PTR(FailOnlyOnAbortOrFlush)
DECLARE_SHARED_PTR(FailOnlyInCloseDocStore)
DECLARE_SHARED_PTR(FailOnlyInWriteSegment)
DECLARE_SHARED_PTR(FailOnlyInSync)
DECLARE_SHARED_PTR(FailOnlyInCommit)

class FailOnlyOnFlush : public MockDirectoryFailure {
public:
    FailOnlyOnFlush() {
        count = 0;
        TestPoint::clear();
    }

    virtual ~FailOnlyOnFlush() {
    }

public:
    int32_t count;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (this->doFail) {
            if (TestPoint::getTestPoint(L"FreqProxTermsWriter", L"appendPostings") &&
                    TestPoint::getTestPoint(L"doFlush") && count++ >= 30) {
                doFail = false;
                boost::throw_exception(IOException(L"now failing during flush"));
            }
        }
    }
};

/// Throws IOException during FieldsWriter.flushDocument and during DocumentsWriter.abort
class FailOnlyOnAbortOrFlush : public MockDirectoryFailure {
public:
    FailOnlyOnAbortOrFlush(bool onlyOnce) {
        onlyOnce = false;
    }

    virtual ~FailOnlyOnAbortOrFlush() {
    }

protected:
    bool onlyOnce;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (doFail) {
            if (TestPoint::getTestPoint(L"abort") || TestPoint::getTestPoint(L"flushDocument")) {
                if (onlyOnce) {
                    doFail = false;
                }
                boost::throw_exception(IOException(L"now failing on purpose"));
            }
        }
    }
};

/// Throws IOException during DocumentsWriter.closeDocStore
class FailOnlyInCloseDocStore : public MockDirectoryFailure {
public:
    FailOnlyInCloseDocStore(bool onlyOnce) {
        onlyOnce = false;
    }

    virtual ~FailOnlyInCloseDocStore() {
    }

protected:
    bool onlyOnce;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (doFail) {
            if (TestPoint::getTestPoint(L"closeDocStore")) {
                if (onlyOnce) {
                    doFail = false;
                }
                boost::throw_exception(IOException(L"now failing on purpose"));
            }
        }
    }
};

/// Throws IOException during DocumentsWriter.writeSegment
class FailOnlyInWriteSegment : public MockDirectoryFailure {
public:
    FailOnlyInWriteSegment(bool onlyOnce) {
        onlyOnce = false;
    }

    virtual ~FailOnlyInWriteSegment() {
    }

protected:
    bool onlyOnce;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (doFail) {
            if (TestPoint::getTestPoint(L"DocFieldProcessor", L"flush")) {
                if (onlyOnce) {
                    doFail = false;
                }
                boost::throw_exception(IOException(L"now failing on purpose"));
            }
        }
    }
};

/// Throws IOException during MockRAMDirectory.sync
class FailOnlyInSync : public MockDirectoryFailure {
public:
    FailOnlyInSync() {
        didFail = false;
    }

    virtual ~FailOnlyInSync() {
    }

public:
    bool didFail;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (doFail) {
            if (TestPoint::getTestPoint(L"MockRAMDirectory", L"sync")) {
                didFail = true;
                boost::throw_exception(IOException(L"now failing on purpose during sync"));
            }
        }
    }
};

class FailOnlyInCommit : public MockDirectoryFailure {
public:
    FailOnlyInCommit() {
        fail1 = false;
        fail2 = false;
    }

    virtual ~FailOnlyInCommit() {
    }

public:
    bool fail1;
    bool fail2;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        bool isCommit = TestPoint::getTestPoint(L"SegmentInfos", L"prepareCommit");
        bool isDelete = TestPoint::getTestPoint(L"MockRAMDirectory", L"deleteFile");

        if (isCommit) {
            if (!isDelete) {
                fail1 = true;
                boost::throw_exception(RuntimeException(L"now fail first"));
            } else {
                fail2 = true;
                boost::throw_exception(IOException(L"now fail during delete"));
            }
        }
    }
};

class CrashingFilter : public TokenFilter {
public:
    CrashingFilter(const String& fieldName, const TokenStreamPtr& input) : TokenFilter(input) {
        this->count = 0;
        this->fieldName = fieldName;
    }

    virtual ~CrashingFilter() {
    }

    LUCENE_CLASS(CrashingFilter);

public:
    String fieldName;
    int32_t count;

public:
    virtual bool incrementToken() {
        if (fieldName == L"crash" && count++ >= 4) {
            boost::throw_exception(IOException(L"now failing on purpose"));
        }
        return input->incrementToken();
    }

    virtual void reset() {
        TokenFilter::reset();
        count = 0;
    }
};

DECLARE_SHARED_PTR(IndexerThread)

class IndexerThread : public LuceneThread {
public:
    IndexerThread(const IndexWriterPtr& writer, bool noErrors) {
        this->writer = writer;
        this->noErrors = noErrors;
        this->addCount = 0;
    }

    virtual ~IndexerThread() {
    }

    LUCENE_CLASS(IndexerThread);

public:
    IndexWriterPtr writer;
    bool noErrors;
    int32_t addCount;

public:
    virtual void run() {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));

        int32_t idUpto = 0;
        int32_t fullCount = 0;
        int64_t stopTime = MiscUtils::currentTimeMillis() + 500;

        while ((int64_t)MiscUtils::currentTimeMillis() < stopTime) {
            try {
                writer->updateDocument(newLucene<Term>(L"id", StringUtils::toString(idUpto++)), doc);
                ++addCount;
            } catch (IOException& e) {
                if (boost::starts_with(e.getError(), L"fake disk full at") || e.getError() == L"now failing on purpose") {
                    LuceneThread::threadSleep(1);
                    if (fullCount++ >= 5) {
                        break;
                    }
                } else {
                    if (noErrors) {
                        FAIL() << "Unexpected exception";
                    }
                    break;
                }
            } catch (...) {
                if (noErrors) {
                    FAIL() << "Unexpected exception";
                }
                break;
            }
        }
    }
};

DECLARE_SHARED_PTR(RunAddIndexesThreads)
DECLARE_SHARED_PTR(RunAddThread)

class RunAddIndexesThreads : public LuceneObject {
public:
    RunAddIndexesThreads(int32_t numCopy);
    virtual ~RunAddIndexesThreads();

    LUCENE_CLASS(RunAddIndexesThreads);

public:
    DirectoryPtr dir;
    DirectoryPtr dir2;

    static const int32_t NUM_INIT_DOCS;
    static const int32_t NUM_THREADS;

    IndexWriterPtr writer2;
    bool didClose;
    Collection<IndexReaderPtr> readers;
    int32_t NUM_COPY;
    Collection<LuceneThreadPtr> threads;
    ConcurrentMergeSchedulerPtr cms;

public:
    void launchThreads(int32_t numIter);
    void joinThreads();
    void close(bool doWait);
    void closeDir();

    virtual void doBody(int32_t j, Collection<DirectoryPtr> dirs) = 0;
    virtual void handle(LuceneException& e) = 0;
};

const int32_t RunAddIndexesThreads::NUM_INIT_DOCS = 17;
const int32_t RunAddIndexesThreads::NUM_THREADS = 5;

class RunAddThread : public LuceneThread {
public:
    RunAddThread(const RunAddIndexesThreadsPtr& runAdd, int32_t numIter, int32_t numCopy, const DirectoryPtr& dir) {
        this->_runAdd = runAdd;
        this->numIter = numIter;
        this->numCopy = numCopy;
        this->dir = dir;
    }

    virtual ~RunAddThread() {
    }

protected:
    RunAddIndexesThreadsWeakPtr _runAdd;
    int32_t numIter;
    int32_t numCopy;
    DirectoryPtr dir;

public:
    virtual void run() {
        try {
            Collection<DirectoryPtr> dirs = Collection<DirectoryPtr>::newInstance(numCopy);
            for (int32_t k = 0; k < numCopy; ++k) {
                dirs[k] = newLucene<MockRAMDirectory>(dir);
            }

            int32_t j = 0;

            while (true) {
                if (numIter > 0 && j == numIter) {
                    break;
                }
                RunAddIndexesThreadsPtr(_runAdd)->doBody(j++, dirs);
            }
        } catch (LuceneException& e) {
            RunAddIndexesThreadsPtr(_runAdd)->handle(e);
        }
    }
};

RunAddIndexesThreads::RunAddIndexesThreads(int32_t numCopy) {
    threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREADS);
    didClose = false;
    NUM_COPY = numCopy;
    dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < NUM_INIT_DOCS; ++i) {
        addDoc(writer);
    }

    writer->close();

    dir2 = newLucene<MockRAMDirectory>();
    writer2 = newLucene<IndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    cms = boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer2->getMergeScheduler());

    readers = Collection<IndexReaderPtr>::newInstance(NUM_COPY);
    for (int32_t i = 0; i < NUM_COPY; ++i) {
        readers[i] = IndexReader::open(dir, true);
    }
}

RunAddIndexesThreads::~RunAddIndexesThreads() {
}

void RunAddIndexesThreads::launchThreads(int32_t numIter) {
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i] = newLucene<RunAddThread>(shared_from_this(), numIter, NUM_COPY, dir);
    }
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i]->start();
    }
}

void RunAddIndexesThreads::joinThreads() {
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i]->join();
    }
}

void RunAddIndexesThreads::close(bool doWait) {
    didClose = true;
    writer2->close(doWait);
}

void RunAddIndexesThreads::closeDir() {
    for (int32_t i = 0; i < NUM_COPY; ++i) {
        readers[i]->close();
    }
    dir2->close();
}

TEST_F(IndexWriterTest, testDocCount) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriter::setDefaultWriteLockTimeout(2000);
    EXPECT_EQ(2000, IndexWriter::getDefaultWriteLockTimeout());

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    IndexWriter::setDefaultWriteLockTimeout(1000);

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    EXPECT_EQ(100, writer->maxDoc());
    writer->close();

    // delete 40 documents
    IndexReaderPtr reader = IndexReader::open(dir, false);
    for (int32_t i = 0; i < 40; ++i) {
        reader->deleteDocument(i);
    }
    reader->close();

    // test doc count before segments are merged/index is optimized
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    EXPECT_EQ(100, writer->maxDoc());
    writer->close();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(100, reader->maxDoc());
    EXPECT_EQ(60, reader->numDocs());
    reader->close();

    // optimize the index and check that the new doc count is correct
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    EXPECT_EQ(100, writer->maxDoc());
    EXPECT_EQ(60, writer->numDocs());
    writer->optimize();
    EXPECT_EQ(60, writer->maxDoc());
    EXPECT_EQ(60, writer->numDocs());
    writer->close();

    // check that the index reader gives the same numbers
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(60, reader->maxDoc());
    EXPECT_EQ(60, reader->numDocs());
    reader->close();

    // make sure opening a new index for create over this existing one works correctly
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    EXPECT_EQ(0, writer->maxDoc());
    EXPECT_EQ(0, writer->numDocs());
    writer->close();
}

/// Test: make sure when we run out of disk space or hit random IOExceptions in any of the addIndexesNoOptimize(*) calls
/// that 1) index is not corrupt (searcher can open/search it) and 2) transactional semantics are followed:
/// either all or none of the incoming documents were in fact added.
TEST_F(IndexWriterTest, testAddIndexOnDiskFull) {
    int32_t START_COUNT = 57;
    int32_t NUM_DIR = 50;
    int32_t END_COUNT = START_COUNT + NUM_DIR * 25;

    // Build up a bunch of dirs that have indexes which we will then merge together by calling addIndexesNoOptimize(*)
    Collection<DirectoryPtr> dirs = Collection<DirectoryPtr>::newInstance(NUM_DIR);
    int64_t inputDiskUsage = 0;
    for (int32_t i = 0; i < NUM_DIR; ++i) {
        dirs[i] = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dirs[i], newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t j = 0; j < 25; ++j) {
            addDocWithIndex(writer, 25 * i + j);
        }
        writer->close();
        HashSet<String> files = dirs[i]->listAll();
        for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
            inputDiskUsage += dirs[i]->fileLength(*file);
        }
    }

    // Now, build a starting index that has START_COUNT docs.  We will then try to addIndexesNoOptimize into a copy of this
    RAMDirectoryPtr startDir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(startDir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t j = 0; j < START_COUNT; ++j) {
        addDocWithIndex(writer, j);
    }
    writer->close();

    // Make sure starting index seems to be working properly
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    IndexReaderPtr reader = IndexReader::open(startDir, true);
    EXPECT_EQ(57, reader->docFreq(searchTerm));

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(57, hits.size());
    searcher->close();
    reader->close();

    // Iterate with larger and larger amounts of free disk space.  With little free disk space,
    // addIndexesNoOptimize will certainly run out of space and fail.  Verify that when this
    // happens, index is not corrupt and index in fact has added no documents.  Then, we increase
    // disk space by 2000 bytes each iteration.  At some point there is enough free disk space
    // and addIndexesNoOptimize should succeed and index should show all documents were added.

    int64_t diskUsage = startDir->sizeInBytes();

    int64_t startDiskUsage = 0;
    HashSet<String> files = startDir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
        startDiskUsage += startDir->fileLength(*file);
    }

    for (int32_t iter = 0; iter < 3; ++iter) {
        // std::cout << "TEST: iter=" << iter;

        // Start with 100 bytes more than we are currently using:
        int64_t diskFree = diskUsage + 100;
        int32_t method = iter;

        bool success = false;
        bool done = false;

        String methodName;
        if (method == 0) {
            methodName = L"addIndexes(Directory[]) + optimize()";
        } else if (method == 1) {
            methodName = L"addIndexes(IndexReader[])";
        } else {
            methodName = L"addIndexesNoOptimize(Directory[])";
        }

        while (!done) {
            // Make a new dir that will enforce disk usage
            MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>(startDir);
            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);

            MergeSchedulerPtr ms = writer->getMergeScheduler();
            for (int32_t x = 0; x < 2; ++x) {
                if (MiscUtils::typeOf<ConcurrentMergeScheduler>(ms)) {
                    // This test intentionally produces exceptions in the threads that CMS launches; we don't
                    // want to pollute test output with these.
                    if (x == 0) {
                        boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms)->setSuppressExceptions();
                    } else {
                        boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms)->clearSuppressExceptions();
                    }
                }

                // Two loops: first time, limit disk space and throw random IOExceptions; second time, no disk space limit
                double rate = 0.05;
                double diskRatio = (double)diskFree / (double)diskUsage;
                int64_t thisDiskFree = 0;

                String testName;

                if (x == 0) {
                    thisDiskFree = diskFree;
                    if (diskRatio >= 2.0) {
                        rate /= 2;
                    }
                    if (diskRatio >= 4.0) {
                        rate /= 2;
                    }
                    if (diskRatio >= 6.0) {
                        rate = 0.0;
                    }
                    testName = L"disk full test " + methodName + L" with disk full at " + StringUtils::toString(diskFree) + L" bytes";
                } else {
                    thisDiskFree = 0;
                    rate = 0.0;
                    testName = L"disk full test " + methodName + L" with unlimited disk space";
                }

                // std::cout << "\ncycle: " << testName;

                dir->setMaxSizeInBytes(thisDiskFree);
                dir->setRandomIOExceptionRate(rate, diskFree);

                try {
                    if (method == 0) {
                        writer->addIndexesNoOptimize(dirs);
                        writer->optimize();
                    } else if (method == 1) {
                        Collection<IndexReaderPtr> readers = Collection<IndexReaderPtr>::newInstance(dirs.size());
                        for (int32_t i = 0; i < dirs.size(); ++i) {
                            readers[i] = IndexReader::open(dirs[i], true);
                        }
                        LuceneException finally;
                        try {
                            writer->addIndexes(readers);
                        } catch (LuceneException& e) {
                            finally = e;
                        }
                        for (int32_t i = 0; i < dirs.size(); ++i) {
                            readers[i]->close();
                        }
                        finally.throwException();
                    } else {
                        writer->addIndexesNoOptimize(dirs);
                    }

                    success = true;
                    // std::cout << "  success!";

                    if (x == 0) {
                        done = true;
                    }
                } catch (IOException& e) {
                    success = false;
                    // std::cout << "  hit IOException: " << e.getError();

                    if (x == 1) {
                        FAIL() << methodName << " hit IOException after disk space was freed up";
                    }
                }

                // Make sure all threads from ConcurrentMergeScheduler are done
                syncConcurrentMerges(writer);

                // std::cout << "  now test readers";

                // Finally, verify index is not corrupt, and, if we succeeded, we see all docs added, and if we
                // failed, we see either all docs or no docs added (transactional semantics)
                try {
                    reader = IndexReader::open(dir, true);
                } catch (IOException& e) {
                    FAIL() << testName << ": exception when creating IndexReader: " << e.getError();
                }
                int32_t result = reader->docFreq(searchTerm);
                if (success) {
                    if (result != START_COUNT) {
                        FAIL() << testName << ": method did not throw exception but docFreq('aaa') is " << result << " instead of expected " << START_COUNT;
                    }
                } else {
                    // On hitting exception we still may have added all docs
                    if (result != START_COUNT && result != END_COUNT) {
                        FAIL() << testName << ": method did throw exception but docFreq('aaa') is " << result << " instead of expected " << START_COUNT << " or " << END_COUNT;
                    }
                }

                searcher = newLucene<IndexSearcher>(reader);
                try {
                    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), END_COUNT)->scoreDocs;
                } catch (IOException& e) {
                    FAIL() << testName << ": exception when searching: " << e.getError();
                }
                int32_t result2 = hits.size();
                if (success) {
                    if (result2 != result) {
                        FAIL() << testName << ": method did not throw exception but hits.length for search on term 'aaa' is " << result2 << " instead of expected " << result;
                    }
                } else {
                    // On hitting exception we still may have added all docs
                    if (result2 != result) {
                        FAIL() << testName << ": method did throw exception but hits.length for search on term 'aaa' is " << result2 << " instead of expected " << result;
                    }
                }

                searcher->close();
                reader->close();
                // std::cout << "  count is " << result;

                if (done || result == END_COUNT) {
                    break;
                }
            }

            // std::cout << "  start disk = " << startDiskUsage << "; input disk = " << inputDiskUsage << "; max used = " << dir->getMaxUsedSizeInBytes();

            if (done) {
                // Make sure that temp free Directory space required is at most 3X total input size of indices
                EXPECT_TRUE((dir->getMaxUsedSizeInBytes() - startDiskUsage) < 3 * (startDiskUsage + inputDiskUsage));
            }

            // Make sure we don't hit disk full during close below
            dir->setMaxSizeInBytes(0);
            dir->setRandomIOExceptionRate(0.0, 0);

            writer->close();

            // Wait for all BG threads to finish else dir->close() will throw IOException because there are still open files
            syncConcurrentMerges(ms);

            dir->close();

            // Try again with 2000 more bytes of free space
            diskFree += 2000;
        }
    }

    startDir->close();
}

/// Make sure IndexWriter cleans up on hitting a disk full exception in addDocument.
TEST_F(IndexWriterTest, testAddDocumentOnDiskFull) {
    for (int32_t pass = 0; pass < 2; ++pass) {
        // std::cout << "TEST: pass=" << pass;
        bool doAbort = (pass == 1);
        int64_t diskFree = 200;
        while (true) {
            // std::cout << "TEST: cycle: diskFree=" << diskFree;
            MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
            dir->setMaxSizeInBytes(diskFree);
            IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

            MergeSchedulerPtr ms = writer->getMergeScheduler();
            if (MiscUtils::typeOf<ConcurrentMergeScheduler>(ms)) {
                boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms)->setSuppressExceptions();
            }

            bool hitError = false;
            try {
                for (int32_t i = 0; i < 200; ++i) {
                    addDoc(writer);
                }
            } catch (IOException&) {
                // std::cout << "TEST: exception on addDoc";
                hitError = true;
            }

            if (hitError) {
                if (doAbort) {
                    writer->rollback();
                } else {
                    try {
                        writer->close();
                    } catch (IOException&) {
                        // std::cout << "TEST: exception on close";
                        dir->setMaxSizeInBytes(0);
                        writer->close();
                    }
                }

                syncConcurrentMerges(ms);

                checkNoUnreferencedFiles(dir);

                // Make sure reader can open the index
                IndexReader::open(dir, true)->close();

                dir->close();

                // Now try again with more space
                diskFree += 500;
            } else {
                syncConcurrentMerges(writer);
                dir->close();
                break;
            }
        }
    }
}

/// Make sure we skip wicked long terms.
TEST_F(IndexWriterTest, testWickedLongTerm) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

    String bigTerm(DocumentsWriter::CHAR_BLOCK_SIZE - 1, L'x');
    DocumentPtr doc = newLucene<Document>();

    // Max length term is 16383, so this contents produces a too-long term
    String contents = L"abc xyz x" + bigTerm + L" another term";
    doc->add(newLucene<Field>(L"content", contents, Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    // Make sure we can add another normal document
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"abc bbb ccc", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);

    // Make sure all terms < max size were indexed
    EXPECT_EQ(2, reader->docFreq(newLucene<Term>(L"content", L"abc")));
    EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"content", L"bbb")));
    EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"content", L"term")));
    EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"content", L"another")));

    // Make sure position is still incremented when massive term is skipped
    TermPositionsPtr tps = reader->termPositions(newLucene<Term>(L"content", L"another"));
    EXPECT_TRUE(tps->next());
    EXPECT_EQ(1, tps->freq());
    EXPECT_EQ(3, tps->nextPosition());

    // Make sure the doc that has the massive term is in the index
    EXPECT_EQ(2, reader->numDocs());

    reader->close();

    // Make sure we can add a document with exactly the maximum length term, and search on that term
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", bigTerm, Field::STORE_NO, Field::INDEX_ANALYZED));
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    sa->setMaxTokenLength(100000);
    writer  = newLucene<IndexWriter>(dir, sa, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    writer->close();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"content", bigTerm)));
    reader->close();

    dir->close();
}

TEST_F(IndexWriterTest, testOptimizeMaxNumSegments) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));

    for (int32_t numDocs = 38; numDocs < 500; numDocs += 38) {
        IndexWriterPtr writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        LogDocMergePolicyPtr ldmp = newLucene<LogDocMergePolicy>(writer);
        ldmp->setMinMergeDocs(1);
        writer->setMergePolicy(ldmp);
        writer->setMergeFactor(5);
        writer->setMaxBufferedDocs(2);
        for (int32_t j = 0; j <numDocs; ++j) {
            writer->addDocument(doc);
        }
        writer->close();

        SegmentInfosPtr sis = newLucene<SegmentInfos>();
        sis->read(dir);
        int32_t segCount = sis->size();

        writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        writer->setMergePolicy(ldmp);
        writer->setMergeFactor(5);
        writer->optimize((int32_t)3);
        writer->close();

        sis = newLucene<SegmentInfos>();
        sis->read(dir);
        int32_t optSegCount = sis->size();

        if (segCount < 3) {
            EXPECT_EQ(segCount, optSegCount);
        } else {
            EXPECT_EQ(3, optSegCount);
        }
    }
}

TEST_F(IndexWriterTest, testOptimizeMaxNumSegments2) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));

    IndexWriterPtr writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    LogDocMergePolicyPtr ldmp = newLucene<LogDocMergePolicy>(writer);
    ldmp->setMinMergeDocs(1);
    writer->setMergePolicy(ldmp);
    writer->setMergeFactor(4);
    writer->setMaxBufferedDocs(2);

    for (int32_t iter = 0; iter < 10; ++iter) {
        for (int32_t i = 0; i < 19; ++i) {
            writer->addDocument(doc);
        }

        writer->commit();
        writer->waitForMerges();
        writer->commit();

        SegmentInfosPtr sis = newLucene<SegmentInfos>();
        sis->read(dir);

        int32_t segCount = sis->size();

        writer->optimize((int32_t)7);
        writer->commit();

        sis = newLucene<SegmentInfos>();
        boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();
        sis->read(dir);
        int32_t optSegCount = sis->size();

        if (segCount < 7) {
            EXPECT_EQ(segCount, optSegCount);
        } else {
            EXPECT_EQ(7, optSegCount);
        }
    }
}

/// Make sure optimize doesn't use any more than 1X starting index size as its temporary free space required.
TEST_F(IndexWriterTest, testOptimizeTempSpaceUsage) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t j = 0; j < 500; ++j) {
        addDocWithIndex(writer, j);
    }

    // force one extra segment w/ different doc store so we see the doc stores get merged
    writer->commit();
    addDocWithIndex(writer, 500);

    writer->close();

    int64_t startDiskUsage = 0;
    HashSet<String> files = dir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
        startDiskUsage += dir->fileLength(*file);
    }

    dir->resetMaxUsedSizeInBytes();
    writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();
    int64_t maxDiskUsage = dir->getMaxUsedSizeInBytes();

    EXPECT_TRUE(maxDiskUsage <= 4 * startDiskUsage);

    dir->close();
}

/// Make sure we can open an index for create even when a reader holds it open (this fails pre lock-less commits on windows)
TEST_F(IndexWriterTest, testCreateWithReader) {
    String indexDir(FileUtils::joinPath(getTempDir(), L"lucenetestindexwriter"));

    LuceneException finally;
    try {
        DirectoryPtr dir = FSDirectory::open(indexDir);

        // add one document and close writer
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        addDoc(writer);
        writer->close();

        // now open reader
        IndexReaderPtr reader = IndexReader::open(dir, true);
        EXPECT_EQ(reader->numDocs(), 1);

        // now open index for create
        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        EXPECT_EQ(writer->maxDoc(), 0);
        addDoc(writer);
        writer->close();

        EXPECT_EQ(reader->numDocs(), 1);
        IndexReaderPtr reader2 = IndexReader::open(dir, true);
        EXPECT_EQ(reader2->numDocs(), 1);
        reader->close();
        reader2->close();
    } catch (LuceneException& e) {
        finally = e;
    }
    FileUtils::removeDirectory(indexDir);

    if (!finally.isNull()) {
        FAIL() << finally.getError();
    }
}

/// Simulate a writer that crashed while writing segments file: make sure we can still open the index (ie,
/// gracefully fallback to the previous segments file), and that we can add to the index
TEST_F(IndexWriterTest, testSimulatedCrashedWriter) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    writer->close();

    int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
    EXPECT_TRUE(gen > 1);

    // Make the next segments file, with last byte missing, to simulate a writer that crashed while
    // writing segments file
    String fileNameIn = SegmentInfos::getCurrentSegmentFileName(dir);
    String fileNameOut = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", 1 + gen);
    IndexInputPtr in = dir->openInput(fileNameIn);
    IndexOutputPtr out = dir->createOutput(fileNameOut);
    int64_t length = in->length();
    for (int32_t i = 0; i < length - 1; ++i) {
        out->writeByte(in->readByte());
    }
    in->close();
    out->close();

    IndexReaderPtr reader;
    EXPECT_NO_THROW(reader = IndexReader::open(dir, true));
    reader->close();

    EXPECT_NO_THROW(writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED));

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    writer->close();
}

/// Simulate a corrupt index by removing last byte of latest segments file and make sure we get an
/// IOException trying to open the index
TEST_F(IndexWriterTest, testSimulatedCorruptIndex1) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    writer->close();

    int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
    EXPECT_TRUE(gen > 1);

    String fileNameIn = SegmentInfos::getCurrentSegmentFileName(dir);
    String fileNameOut = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", 1 + gen);
    IndexInputPtr in = dir->openInput(fileNameIn);
    IndexOutputPtr out = dir->createOutput(fileNameOut);
    int64_t length = in->length();
    for (int32_t i = 0; i < length - 1; ++i) {
        out->writeByte(in->readByte());
    }
    in->close();
    out->close();
    dir->deleteFile(fileNameIn);

    IndexReaderPtr reader;
    try {
        reader = IndexReader::open(dir, true);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }

    if (reader) {
        reader->close();
    }
}

TEST_F(IndexWriterTest, testChangesAfterClose) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    addDoc(writer);
    writer->close();

    try {
        addDoc(writer);
    } catch (AlreadyClosedException& e) {
        EXPECT_TRUE(check_exception(LuceneException::AlreadyClosed)(e));
    }
}

/// Simulate a corrupt index by removing one of the cfs files and make sure we get an IOException trying to open the index
TEST_F(IndexWriterTest, testSimulatedCorruptIndex2) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    writer->close();

    int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
    EXPECT_TRUE(gen > 1);

    HashSet<String> files = dir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
        if (boost::ends_with(*file, L".cfs")) {
            dir->deleteFile(*file);
            break;
        }
    }

    IndexReaderPtr reader;
    try {
        reader = IndexReader::open(dir, true);
    } catch (FileNotFoundException& e) {
        EXPECT_TRUE(check_exception(LuceneException::FileNotFound)(e));
    }

    if (reader) {
        reader->close();
    }
}

/// Simple test for "commit on close": open writer then add a bunch of docs, making sure reader does
/// not see these docs until writer is closed.
TEST_F(IndexWriterTest, testCommitOnClose) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 14; ++i) {
        addDoc(writer);
    }
    writer->close();

    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(14, hits.size());
    searcher->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 3; ++i) {
        for (int32_t j = 0; j < 11; ++j) {
            addDoc(writer);
        }
        searcher = newLucene<IndexSearcher>(dir, false);
        hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(14, hits.size());
        searcher->close();
        EXPECT_TRUE(reader->isCurrent());
    }

    // Now, close the writer
    writer->close();
    EXPECT_TRUE(!reader->isCurrent());

    searcher = newLucene<IndexSearcher>(dir, false);
    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(47, hits.size());
    searcher->close();
}

TEST_F(IndexWriterTest, testCommitOnCloseAbort) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    for (int32_t i = 0; i < 14; ++i) {
        addDoc(writer);
    }
    writer->close();

    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(14, hits.size());
    searcher->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    for (int32_t i = 0; i < 17; ++i) {
        addDoc(writer);
    }

    // Delete all docs
    writer->deleteDocuments(searchTerm);

    searcher = newLucene<IndexSearcher>(dir, false);
    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(14, hits.size());
    searcher->close();

    // Now, close the writer
    writer->rollback();

    checkNoUnreferencedFiles(dir);

    searcher = newLucene<IndexSearcher>(dir, false);
    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(14, hits.size());
    searcher->close();

    // Now make sure we can re-open the index, add docs, and all is good
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);

    // On abort, writer in fact may write to the same segments_N file
    dir->setPreventDoubleWrite(false);

    for (int32_t i = 0; i < 12; ++i) {
        for (int32_t j = 0; j < 17; ++j) {
            addDoc(writer);
        }
        searcher = newLucene<IndexSearcher>(dir, false);
        hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(14, hits.size());
        searcher->close();
    }

    writer->close();
    searcher = newLucene<IndexSearcher>(dir, false);
    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(218, hits.size());
    searcher->close();

    dir->close();
}

/// Verify that a writer with "commit on close" indeed cleans up the temp segments created after opening
/// that are not referenced by the starting segments file.  We check this by using MockRAMDirectory to
/// measure max temp disk space used.
TEST_F(IndexWriterTest, testCommitOnCloseDiskUsage) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    for (int32_t j = 0; j < 30; ++j) {
        addDocWithIndex(writer, j);
    }
    writer->close();
    dir->resetMaxUsedSizeInBytes();

    int64_t startDiskUsage = dir->getMaxUsedSizeInBytes();
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
    for (int32_t j = 0; j < 1470; ++j) {
        addDocWithIndex(writer, j);
    }
    int64_t midDiskUsage = dir->getMaxUsedSizeInBytes();
    dir->resetMaxUsedSizeInBytes();
    writer->optimize();
    writer->close();

    IndexReader::open(dir, true)->close();

    int64_t endDiskUsage = dir->getMaxUsedSizeInBytes();

    // Ending index is 50X as large as starting index; due to 3X disk usage normally we allow 150X max
    // transient usage.  If something is wrong with deleter and it doesn't delete intermediate segments
    // then it will exceed this 150X
    EXPECT_TRUE(midDiskUsage < 150 * startDiskUsage);
    EXPECT_TRUE(endDiskUsage < 150 * startDiskUsage);
}

/// Verify that calling optimize when writer is open for "commit on close" works correctly both for
/// rollback() and close().
TEST_F(IndexWriterTest, testCommitOnCloseOptimize) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    for (int32_t j = 0; j < 17; ++j) {
        addDocWithIndex(writer, j);
    }
    writer->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();

    // Open a reader before closing (commiting) the writer
    IndexReaderPtr reader = IndexReader::open(dir, true);

    // Reader should see index as unoptimized at this point
    EXPECT_TRUE(!reader->isOptimized());
    reader->close();

    // Abort the writer
    writer->rollback();
    checkNoUnreferencedFiles(dir);

    // Open a reader after aborting writer
    reader = IndexReader::open(dir, true);

    // Reader should still see index as unoptimized
    EXPECT_TRUE(!reader->isOptimized());
    reader->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();
    checkNoUnreferencedFiles(dir);

    // Open a reader after aborting writer
    reader = IndexReader::open(dir, true);

    // Reader should still see index as unoptimized:
    EXPECT_TRUE(reader->isOptimized());
    reader->close();
}

TEST_F(IndexWriterTest, testIndexNoDocuments) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->commit();
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->maxDoc());
    EXPECT_EQ(0, reader->numDocs());
    reader->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->commit();
    writer->close();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->maxDoc());
    EXPECT_EQ(0, reader->numDocs());
    reader->close();
}

TEST_F(IndexWriterTest, testManyFields) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    for (int32_t j = 0; j < 100; ++j) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"a" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"b" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"c" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"d" + StringUtils::toString(j), L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"e" + StringUtils::toString(j), L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"f" + StringUtils::toString(j), L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(100, reader->maxDoc());
    EXPECT_EQ(100, reader->numDocs());

    for (int32_t j = 0; j < 100; ++j) {
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"a" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j))));
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"b" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j))));
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"c" + StringUtils::toString(j), L"aaa" + StringUtils::toString(j))));
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"d" + StringUtils::toString(j), L"aaa")));
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"e" + StringUtils::toString(j), L"aaa")));
        EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"f" + StringUtils::toString(j), L"aaa")));
    }
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testSmallRAMBuffer) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setRAMBufferSizeMB(0.000001);
    int32_t lastNumFile = dir->listAll().size();
    for (int32_t j = 0; j < 9; ++j) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
        int32_t numFile = dir->listAll().size();
        // Verify that with a tiny RAM buffer we see new segment after every doc
        EXPECT_TRUE(numFile > lastNumFile);
        lastNumFile = numFile;
    }
    writer->close();
    dir->close();
}

/// Make sure it's OK to change RAM buffer size and maxBufferedDocs in a write session
TEST_F(IndexWriterTest, testChangingRAMBuffer) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);

    int32_t lastFlushCount = -1;
    for (int32_t j = 1; j < 52; ++j) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
        syncConcurrentMerges(writer);
        int32_t flushCount = writer->getFlushCount();
        if (j == 1) {
            lastFlushCount = flushCount;
        } else if (j < 10) {
            // No new files should be created
            EXPECT_EQ(flushCount, lastFlushCount);
        } else if (j == 10) {
            EXPECT_TRUE(flushCount > lastFlushCount);
            lastFlushCount = flushCount;
            writer->setRAMBufferSizeMB(0.000001);
            writer->setMaxBufferedDocs(IndexWriter::DISABLE_AUTO_FLUSH);
        } else if (j < 20) {
            EXPECT_TRUE(flushCount > lastFlushCount);
            lastFlushCount = flushCount;
        } else if (j == 20) {
            writer->setRAMBufferSizeMB(16);
            writer->setMaxBufferedDocs(IndexWriter::DISABLE_AUTO_FLUSH);
            lastFlushCount = flushCount;
        } else if (j < 30) {
            EXPECT_EQ(flushCount, lastFlushCount);
        } else if (j == 30) {
            writer->setRAMBufferSizeMB(0.000001);
            writer->setMaxBufferedDocs(IndexWriter::DISABLE_AUTO_FLUSH);
        } else if (j < 40) {
            EXPECT_TRUE(flushCount> lastFlushCount);
            lastFlushCount = flushCount;
        } else if (j == 40) {
            writer->setMaxBufferedDocs(10);
            writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
            lastFlushCount = flushCount;
        } else if (j < 50) {
            EXPECT_EQ(flushCount, lastFlushCount);
            writer->setMaxBufferedDocs(10);
            writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
        } else if (j == 50) {
            EXPECT_TRUE(flushCount > lastFlushCount);
        }
    }
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testChangingRAMBuffer2) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMaxBufferedDeleteTerms(10);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);

    for (int32_t j = 1; j < 52; ++j) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"aaa" + StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }

    int32_t lastFlushCount = -1;
    for (int32_t j = 1; j < 52; ++j) {
        writer->deleteDocuments(newLucene<Term>(L"field", L"aaa" + StringUtils::toString(j)));
        syncConcurrentMerges(writer);
        int32_t flushCount = writer->getFlushCount();
        if (j == 1) {
            lastFlushCount = flushCount;
        } else if (j < 10) {
            // No new files should be created
            EXPECT_EQ(flushCount, lastFlushCount);
        } else if (j == 10) {
            EXPECT_TRUE(flushCount > lastFlushCount);
            lastFlushCount = flushCount;
            writer->setRAMBufferSizeMB(0.000001);
            writer->setMaxBufferedDeleteTerms(1);
        } else if (j < 20) {
            EXPECT_TRUE(flushCount > lastFlushCount);
            lastFlushCount = flushCount;
        } else if (j == 20) {
            writer->setRAMBufferSizeMB(16);
            writer->setMaxBufferedDeleteTerms(IndexWriter::DISABLE_AUTO_FLUSH);
            lastFlushCount = flushCount;
        } else if (j < 30) {
            EXPECT_EQ(flushCount, lastFlushCount);
        } else if (j == 30) {
            writer->setRAMBufferSizeMB(0.000001);
            writer->setMaxBufferedDeleteTerms(IndexWriter::DISABLE_AUTO_FLUSH);
            writer->setMaxBufferedDeleteTerms(1);
        } else if (j < 40) {
            EXPECT_TRUE(flushCount> lastFlushCount);
            lastFlushCount = flushCount;
        } else if (j == 40) {
            writer->setMaxBufferedDeleteTerms(10);
            writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
            lastFlushCount = flushCount;
        } else if (j < 50) {
            EXPECT_EQ(flushCount, lastFlushCount);
            writer->setMaxBufferedDeleteTerms(10);
            writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
        } else if (j == 50) {
            EXPECT_TRUE(flushCount > lastFlushCount);
        }
    }
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testDiverseDocs) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setRAMBufferSizeMB(0.5);
    RandomPtr rand = newLucene<Random>();

    for (int32_t i = 0; i < 3; ++i) {
        // First, docs where every term is unique (heavy on Posting instances)
        for (int32_t j = 0; j < 100; ++j) {
            DocumentPtr doc = newLucene<Document>();
            for (int32_t k = 0; k < 100; ++k) {
                doc->add(newLucene<Field>(L"field", StringUtils::toString(rand->nextInt()), Field::STORE_YES, Field::INDEX_ANALYZED));
            }
            writer->addDocument(doc);
        }

        // Next, many single term docs where only one term occurs (heavy on byte blocks)
        for (int32_t j = 0; j < 100; ++j) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"field", L"aaa aaa aaa aaa aaa aaa aaa aaa aaa aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }

        // Next, many single term docs where only one term occurs but the terms are very long (heavy on char[] arrays)
        for (int32_t j = 0; j < 100; ++j) {
            StringStream buffer;
            for (int32_t k = 0; k < 1000; ++k) {
                buffer << j << L".";
            }
            String longTerm = buffer.str();

            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"field", longTerm, Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
    }
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(newLucene<Term>(L"field", L"aaa")), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(300, hits.size());
    searcher->close();

    dir->close();
}

TEST_F(IndexWriterTest, testEnablingNorms) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);

    // Enable norms for only 1 doc, pre flush
    for (int32_t j = 0; j < 10; ++j) {
        DocumentPtr doc = newLucene<Document>();
        FieldPtr f = newLucene<Field>(L"field", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED);
        if (j != 8) {
            f->setOmitNorms(true);
        }
        doc->add(f);
        writer->addDocument(doc);
    }
    writer->close();

    TermPtr searchTerm = newLucene<Term>(L"field", L"aaa");

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(10, hits.size());
    searcher->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);

    // Enable norms for only 1 doc, post flush
    for (int32_t j = 0; j < 27; ++j) {
        DocumentPtr doc = newLucene<Document>();
        FieldPtr f = newLucene<Field>(L"field", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED);
        if (j != 26) {
            f->setOmitNorms(true);
        }
        doc->add(f);
        writer->addDocument(doc);
    }

    writer->close();

    searcher = newLucene<IndexSearcher>(dir, false);
    hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(27, hits.size());
    searcher->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    reader->close();

    dir->close();
}

TEST_F(IndexWriterTest, testHighFreqTerm) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, 100000000);
    writer->setRAMBufferSizeMB(0.01);
    // Massive doc that has 128 K a's
    StringStream buffer;
    for (int32_t i = 0; i < 4096; ++i) {
        buffer << L" a a a a a a a a";
        buffer << L" a a a a a a a a";
        buffer << L" a a a a a a a a";
        buffer << L" a a a a a a a a";
    }
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", buffer.str(), Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(1, reader->maxDoc());
    EXPECT_EQ(1, reader->numDocs());
    TermPtr t = newLucene<Term>(L"field", L"a");
    EXPECT_EQ(1, reader->docFreq(t));
    TermDocsPtr td = reader->termDocs(t);
    td->next();
    EXPECT_EQ(128 * 1024, td->freq());
    reader->close();
    dir->close();
}

namespace TestNullLockFactory {

class MyRAMDirectory : public RAMDirectory {
public:
    MyRAMDirectory() {
        lockFactory.reset();
        myLockFactory = newLucene<SingleInstanceLockFactory>();
    }

    virtual ~MyRAMDirectory() {
    }

    LUCENE_CLASS(MyRAMDirectory);

protected:
    LockFactoryPtr myLockFactory;

public:
    virtual LockPtr makeLock(const String& name) {
        return myLockFactory->makeLock(name);
    }
};

}

/// Make sure that a Directory implementation that does not use LockFactory at all (ie overrides makeLock and
/// implements its own private locking) works OK.  This was raised on java-dev as loss of backwards compatibility.
TEST_F(IndexWriterTest, testNullLockFactory) {
    DirectoryPtr dir = newLucene<TestNullLockFactory::MyRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }

    writer->close();
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
    Collection<ScoreDocPtr> hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(100, hits.size());
    writer->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    dir->close();
}

namespace TestFlushWithNoMerging {

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

TEST_F(IndexWriterTest, testFlushWithNoMerging) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    TestFlushWithNoMerging::TestableIndexWriterPtr writer = newLucene<TestFlushWithNoMerging::TestableIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    for (int32_t i = 0; i < 19; ++i) {
        writer->addDocument(doc);
    }
    writer->flush(false, true, true);
    writer->close();
    SegmentInfosPtr sis = newLucene<SegmentInfos>();
    sis->read(dir);
    // Since we flushed without allowing merging we should now have 10 segments
    EXPECT_EQ(sis->size(), 10);
}

/// Make sure we can flush segment with norms, then add empty doc (no norms) and flush
TEST_F(IndexWriterTest, testEmptyDocAfterFlushingRealDoc) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
    writer->commit();
    writer->addDocument(newLucene<Document>());
    writer->close();
    checkIndex(dir);
    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(2, reader->numDocs());
}

/// Test calling optimize(false) whereby optimize is kicked off but we don't wait for it to finish (but
/// writer.close()) does wait
TEST_F(IndexWriterTest, testBackgroundOptimize) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    for (int32_t pass = 0; pass < 2; ++pass) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMergeScheduler(newLucene<ConcurrentMergeScheduler>());
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(101);
        for (int32_t i = 0; i < 200; ++i) {
            writer->addDocument(doc);
        }
        writer->optimize(false);

        if (pass == 0) {
            writer->close();
            IndexReaderPtr reader = IndexReader::open(dir, true);
            EXPECT_TRUE(reader->isOptimized());
            reader->close();
        } else {
            // Get another segment to flush so we can verify it is NOT included in the optimization
            writer->addDocument(doc);
            writer->addDocument(doc);
            writer->close();

            IndexReaderPtr reader = IndexReader::open(dir, true);
            EXPECT_TRUE(!reader->isOptimized());
            reader->close();

            SegmentInfosPtr infos = newLucene<SegmentInfos>();
            infos->read(dir);
            EXPECT_EQ(2, infos->size());
        }
    }
    dir->close();

    // allow time for merge threads to finish
    LuceneThread::threadSleep(1000);
}

/// Test that no NullPointerException will be raised, when adding one document with a single, empty
/// field and term vectors enabled.
TEST_F(IndexWriterTest, testBadSegment) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    EXPECT_NO_THROW(doc->add(newLucene<Field>(L"tvtest", L"", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES)));
    writer->addDocument(doc);
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testNoTermVectorAfterTermVector) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvtest", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    writer->addDocument(doc);
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvtest", L"x y z", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
    writer->addDocument(doc);
    // Make first segment
    writer->commit();

    doc->add(newLucene<Field>(L"tvtest", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    writer->addDocument(doc);

    // Make 2nd segment
    writer->commit();

    writer->optimize();
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testNoTermVectorAfterTermVectorMerge) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvtest", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    writer->addDocument(doc);
    writer->commit();

    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvtest", L"x y z", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
    writer->addDocument(doc);
    // Make first segment
    writer->commit();

    writer->optimize();

    doc->add(newLucene<Field>(L"tvtest", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    writer->addDocument(doc);

    // Make 2nd segment
    writer->commit();
    writer->optimize();

    writer->close();
    dir->close();
}

namespace TestMaxThreadPriority {

// Just intercepts all merges & verifies that we are never merging a segment with >= 20 (maxMergeDocs) docs
class MyMergeScheduler : public MergeScheduler {
public:
    virtual ~MyMergeScheduler() {
    }

    LUCENE_CLASS(MyMergeScheduler);

public:
    virtual void merge(const IndexWriterPtr& writer) {
        while (true) {
            OneMergePtr merge = writer->getNextMerge();
            if (!merge) {
                break;
            }
            for (int32_t i = 0; i < merge->segments->size(); ++i) {
                EXPECT_TRUE(merge->segments->info(i)->docCount < 20);
            }
            writer->merge(merge);
        }
    }

    virtual void close() {
    }
};

}

TEST_F(IndexWriterTest, testMaxThreadPriority) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMergeScheduler(newLucene<TestMaxThreadPriority::MyMergeScheduler>());
    writer->setMaxMergeDocs(20);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvtest", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    for (int32_t i = 0; i < 177; ++i) {
        writer->addDocument(doc);
    }
    writer->close();
    dir->close();
}

namespace TestExceptionFromTokenStream {

class ExceptionTokenFilter : public TokenFilter {
public:
    ExceptionTokenFilter(const TokenStreamPtr& input) : TokenFilter(input) {
        count = 0;
    }

    virtual ~ExceptionTokenFilter() {
    }

    LUCENE_CLASS(ExceptionTokenFilter);

protected:
    int32_t count;

public:
    virtual bool incrementToken() {
        if (count++ == 5) {
            boost::throw_exception(IOException(L"now failing on purpose"));
        }
        return input->incrementToken();
    }
};

class ExceptionAnalyzer : public Analyzer {
public:
    virtual ~ExceptionAnalyzer() {
    }

    LUCENE_CLASS(ExceptionAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<ExceptionTokenFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, reader));
    }
};

}

TEST_F(IndexWriterTest, testExceptionFromTokenStream) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<TestExceptionFromTokenStream::ExceptionAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    String contents = L"aa bb cc dd ee ff gg hh ii jj kk";
    doc->add(newLucene<Field>(L"content", contents, Field::STORE_NO, Field::INDEX_ANALYZED));

    try {
        writer->addDocument(doc);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }

    // Make sure we can add another normal document
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aa bb cc dd", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    // Make sure we can add another normal document
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aa bb cc dd", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPtr t = newLucene<Term>(L"content", L"aa");
    EXPECT_EQ(reader->docFreq(t), 3);

    // Make sure the doc that hit the exception was marked as deleted
    TermDocsPtr tdocs = reader->termDocs(t);
    int32_t count = 0;
    while (tdocs->next()) {
        ++count;
    }

    EXPECT_EQ(2, count);

    EXPECT_EQ(reader->docFreq(newLucene<Term>(L"content", L"gg")), 0);
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testDocumentsWriterAbort) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    FailOnlyOnFlushPtr failure = newLucene<FailOnlyOnFlush>();
    failure->setDoFail();
    dir->failOn(failure);

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    String contents = L"aa bb cc dd ee ff gg hh ii jj kk";
    doc->add(newLucene<Field>(L"content", contents, Field::STORE_NO, Field::INDEX_ANALYZED));
    bool hitError = false;
    for (int32_t i = 0; i < 200; ++i) {
        try {
            writer->addDocument(doc);
        } catch (IOException&) {
            // only one flush should fail
            EXPECT_TRUE(!hitError);
            hitError = true;
        }
    }
    EXPECT_TRUE(hitError);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(198, reader->docFreq(newLucene<Term>(L"content", L"aa")));
    reader->close();
}

namespace TestDocumentsWriterExceptions {

class CrashAnalyzer : public Analyzer {
public:
    virtual ~CrashAnalyzer() {
    }

    LUCENE_CLASS(CrashAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<CrashingFilter>(fieldName, newLucene<WhitespaceTokenizer>(reader));
    }
};

}

TEST_F(IndexWriterTest, testDocumentsWriterExceptions) {
    AnalyzerPtr analyzer = newLucene<TestDocumentsWriterExceptions::CrashAnalyzer>();

    for (int32_t i = 0; i < 2; ++i) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->addDocument(doc);
        writer->addDocument(doc);
        doc->add(newLucene<Field>(L"crash", L"this should crash after 4 terms", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        doc->add(newLucene<Field>(L"other", L"this will not get indexed", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));

        try {
            writer->addDocument(doc);
        } catch (IOException& e) {
            EXPECT_TRUE(check_exception(LuceneException::IO)(e));
        }

        if (i == 0) {
            doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
            writer->addDocument(doc);
            writer->addDocument(doc);
        }
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, true);
        int32_t expected = 3 + (1 - i) * 2;
        EXPECT_EQ(expected, reader->docFreq(newLucene<Term>(L"contents", L"here")));
        EXPECT_EQ(expected, reader->maxDoc());
        int32_t numDel = 0;
        for (int32_t j = 0; j < reader->maxDoc(); ++j) {
            if (reader->isDeleted(j)) {
                ++numDel;
            } else {
                reader->document(j);
                reader->getTermFreqVectors(j);
            }
        }
        reader->close();

        EXPECT_EQ(1, numDel);

        writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(10);
        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        for (int32_t j = 0; j < 17; ++j) {
            writer->addDocument(doc);
        }
        writer->optimize();
        writer->close();

        reader = IndexReader::open(dir, true);
        expected = 19 + (1 - i) * 2;
        EXPECT_EQ(expected, reader->docFreq(newLucene<Term>(L"contents", L"here")));
        EXPECT_EQ(expected, reader->maxDoc());
        numDel = 0;
        for (int32_t j = 0; j < reader->maxDoc(); ++j) {
            if (reader->isDeleted(j)) {
                ++numDel;
            } else {
                reader->document(j);
                reader->getTermFreqVectors(j);
            }
        }
        reader->close();
        EXPECT_EQ(0, numDel);

        dir->close();
    }
}

namespace TestDocumentsWriterExceptionThreads {

class CrashAnalyzer : public Analyzer {
public:
    virtual ~CrashAnalyzer() {
    }

    LUCENE_CLASS(CrashAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<CrashingFilter>(fieldName, newLucene<WhitespaceTokenizer>(reader));
    }
};

class ExceptionThread : public LuceneThread {
public:
    ExceptionThread(const IndexWriterPtr& writer, int32_t numIter, int32_t finalI) {
        this->writer = writer;
        this->numIter = numIter;
        this->finalI = finalI;
    }

    virtual ~ExceptionThread() {
    }

    LUCENE_CLASS(ExceptionThread);

protected:
    IndexWriterPtr writer;
    int32_t numIter;
    int32_t finalI;

public:
    virtual void run() {
        try {
            for (int32_t iter = 0; iter < numIter; ++iter) {
                DocumentPtr doc = newLucene<Document>();
                doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
                writer->addDocument(doc);
                writer->addDocument(doc);
                doc->add(newLucene<Field>(L"crash", L"this should crash after 4 terms", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
                doc->add(newLucene<Field>(L"other", L"this will not get indexed", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));

                try {
                    writer->addDocument(doc);
                    FAIL() << "did not hit expected exception";
                } catch (IOException&) {
                }

                if (finalI == 0) {
                    doc = newLucene<Document>();
                    doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
                    writer->addDocument(doc);
                    writer->addDocument(doc);
                }
            }
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

}

TEST_F(IndexWriterTest, testDocumentsWriterExceptionThreads) {
    AnalyzerPtr analyzer = newLucene<TestDocumentsWriterExceptionThreads::CrashAnalyzer>();

    int32_t NUM_THREAD = 3;
    int32_t NUM_ITER = 100;

    for (int32_t i = 0; i < 2; ++i) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

        {
            IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
            int32_t finalI = i;

            Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREAD);
            for (int32_t t = 0; t < NUM_THREAD; ++t) {
                threads[t] = newLucene<TestDocumentsWriterExceptionThreads::ExceptionThread>(writer, NUM_ITER, finalI);
                threads[t]->start();
            }

            for (int32_t t = 0; t < NUM_THREAD; ++t) {
                threads[t]->join();
            }

            writer->close();
        }

        IndexReaderPtr reader = IndexReader::open(dir, true);
        int32_t expected = (3 + (1 - i) * 2) * NUM_THREAD * NUM_ITER;
        EXPECT_EQ(expected, reader->docFreq(newLucene<Term>(L"contents", L"here")));
        EXPECT_EQ(expected, reader->maxDoc());
        int32_t numDel = 0;
        for (int32_t j = 0; j < reader->maxDoc(); ++j) {
            if (reader->isDeleted(j)) {
                ++numDel;
            } else {
                reader->document(j);
                reader->getTermFreqVectors(j);
            }
        }
        reader->close();

        EXPECT_EQ(NUM_THREAD * NUM_ITER, numDel);

        IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(10);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"contents", L"here are some contents", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        for (int32_t j = 0; j < 17; ++j) {
            writer->addDocument(doc);
        }
        writer->optimize();
        writer->close();

        reader = IndexReader::open(dir, true);
        expected += 17 - NUM_THREAD * NUM_ITER;
        EXPECT_EQ(expected, reader->docFreq(newLucene<Term>(L"contents", L"here")));
        EXPECT_EQ(expected, reader->maxDoc());
        numDel = 0;
        for (int32_t j = 0; j < reader->maxDoc(); ++j) {
            if (reader->isDeleted(j)) {
                ++numDel;
            } else {
                reader->document(j);
                reader->getTermFreqVectors(j);
            }
        }
        reader->close();
        EXPECT_EQ(0, numDel);

        dir->close();
    }
}

TEST_F(IndexWriterTest, testVariableSchema) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    int32_t delID = 0;

    for (int32_t i = 0; i < 20; ++i) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(2);
        writer->setUseCompoundFile(false);
        DocumentPtr doc = newLucene<Document>();
        String contents = L"aa bb cc dd ee ff gg hh ii jj kk";

        if (i == 7) {
            // Add empty docs here
            doc->add(newLucene<Field>(L"content3", L"", Field::STORE_NO, Field::INDEX_ANALYZED));
        } else {
            Field::Store storeVal = Field::STORE_NO;
            if (i % 2 == 0) {
                doc->add(newLucene<Field>(L"content4", contents, Field::STORE_YES, Field::INDEX_ANALYZED));
                storeVal = Field::STORE_YES;
            }

            doc->add(newLucene<Field>(L"content1", contents, storeVal, Field::INDEX_ANALYZED));
            doc->add(newLucene<Field>(L"content3", L"", Field::STORE_YES, Field::INDEX_ANALYZED));
            doc->add(newLucene<Field>(L"content5", L"", storeVal, Field::INDEX_ANALYZED));
        }

        for (int32_t j = 0; j < 4; ++j) {
            writer->addDocument(doc);
        }

        writer->close();
        IndexReaderPtr reader = IndexReader::open(dir, false);
        reader->deleteDocument(delID++);
        reader->close();

        if (i % 4 == 0) {
            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
            writer->setUseCompoundFile(false);
            writer->optimize();
            writer->close();
        }
    }
}

namespace TestNoWaitClose {

class NoWaitThread : public LuceneThread {
public:
    NoWaitThread(const IndexWriterPtr& finalWriter, const DocumentPtr& doc) {
        this->finalWriter = finalWriter;
        this->doc = doc;
    }

    virtual ~NoWaitThread() {
    }

    LUCENE_CLASS(NoWaitThread);

protected:
    IndexWriterPtr finalWriter;
    DocumentPtr doc;

public:
    virtual void run() {
        bool done = false;
        while (!done) {
            for (int32_t i = 0; i < 100; ++i) {
                try {
                    finalWriter->addDocument(doc);
                } catch (AlreadyClosedException&) {
                    done = true;
                    break;
                } catch (NullPointerException&) {
                    done = true;
                    break;
                } catch (...) {
                    FAIL() << "Unexpected exception";
                    done = true;
                    break;
                }
            }
            LuceneThread::threadYield();
        }
    }
};

}

TEST_F(IndexWriterTest, testNoWaitClose) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    DocumentPtr doc = newLucene<Document>();
    FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    doc->add(idField);

    for (int32_t pass = 0; pass < 2; ++pass) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

        for (int32_t iter = 0; iter < 10; ++iter) {
            MergeSchedulerPtr ms;
            if (pass == 1) {
                ms = newLucene<ConcurrentMergeScheduler>();
            } else {
                ms = newLucene<SerialMergeScheduler>();
            }

            writer->setMergeScheduler(ms);
            writer->setMaxBufferedDocs(2);
            writer->setMergeFactor(100);

            for (int32_t j = 0; j < 199; ++j) {
                idField->setValue(StringUtils::toString(iter * 201 + j));
                writer->addDocument(doc);
            }

            int32_t delID = iter * 199;
            for (int32_t j = 0; j < 20; ++j) {
                writer->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(delID)));
                delID += 5;
            }

            // Force a bunch of merge threads to kick off so we stress out aborting them on close
            writer->setMergeFactor(2);

            IndexWriterPtr finalWriter = writer;
            LuceneThreadPtr t1 = newLucene<TestNoWaitClose::NoWaitThread>(finalWriter, doc);

            t1->start();

            writer->close(false);
            t1->join();

            // Make sure reader can read
            IndexReaderPtr reader = IndexReader::open(dir, true);
            reader->close();

            // Reopen
            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
        }
        writer->close();
    }
    dir->close();
}

/// Make sure we can close() even while threads are trying to add documents.
TEST_F(IndexWriterTest, testCloseWithThreads) {
    int32_t NUM_THREADS = 3;

    for (int32_t iter = 0; iter < 20; ++iter) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();

        // We expect AlreadyClosedException
        cms->setSuppressExceptions();

        writer->setMergeScheduler(cms);
        writer->setMaxBufferedDocs(10);
        writer->setMergeFactor(4);

        Collection<IndexerThreadPtr> threads = Collection<IndexerThreadPtr>::newInstance(NUM_THREADS);

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i] = newLucene<IndexerThread>(writer, false);
        }

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->start();
        }

        bool done = false;
        while (!done) {
            LuceneThread::threadSleep(100);
            for (int32_t i = 0; i < NUM_THREADS; ++i) {
                // only stop when at least one thread has added a doc
                if (threads[i]->addCount > 0) {
                    done = true;
                    break;
                }
            }
        }

        writer->close(false);

        // Make sure threads that are adding docs are not hung
        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->join();
            if (threads[i]->isAlive()) {
                FAIL() << "thread seems to be hung";
            }
        }

        // Quick test to make sure index is not corrupt
        IndexReaderPtr reader = IndexReader::open(dir, true);
        TermDocsPtr tdocs = reader->termDocs(newLucene<Term>(L"field", L"aaa"));
        int32_t count = 0;
        while (tdocs->next()) {
            ++count;
        }
        EXPECT_TRUE(count > 0);
        reader->close();

        dir->close();
    }
}

/// Make sure immediate disk full on creating an IndexWriter (hit during DW.ThreadState.init()) is OK
TEST_F(IndexWriterTest, testImmediateDiskFull) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    dir->setMaxSizeInBytes(dir->getRecomputedActualSizeInBytes());
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    try {
        writer->addDocument(doc);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }
    try {
        writer->addDocument(doc);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }
    try {
        writer->close(false);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }
}

/// Make sure immediate disk full on creating an IndexWriter (hit during DW.ThreadState.init()),
/// with multiple threads, is OK
TEST_F(IndexWriterTest, testImmediateDiskFullWithThreads) {
    int32_t NUM_THREADS = 3;

    for (int32_t iter = 0; iter < 10; ++iter) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
        ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();

        // We expect AlreadyClosedException
        cms->setSuppressExceptions();

        writer->setMergeScheduler(cms);
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(4);
        dir->setMaxSizeInBytes(4 * 1024 + 20 * iter);

        Collection<IndexerThreadPtr> threads = Collection<IndexerThreadPtr>::newInstance(NUM_THREADS);

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i] = newLucene<IndexerThread>(writer, true);
        }

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->start();
        }

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->join();
        }

        try {
            writer->close(false);
        } catch (IOException&) {
        }

        // allow time for merge threads to finish
        LuceneThread::threadSleep(1000);

        dir->close();
    }
}

static void _testSingleThreadFailure(const MockDirectoryFailurePtr& failure) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"aaa bbb ccc ddd eee fff ggg hhh iii jjj", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));

    for (int32_t i = 0; i < 6; ++i) {
        writer->addDocument(doc);
    }

    dir->failOn(failure);
    failure->setDoFail();
    try {
        writer->addDocument(doc);
        writer->addDocument(doc);
        writer->commit();
        FAIL() << "did not hit exception";
    } catch (IOException&) {
    }
    failure->clearDoFail();
    writer->addDocument(doc);
    writer->close(false);
}

static void _testMultipleThreadsFailure(const MockDirectoryFailurePtr& failure) {
    int32_t NUM_THREADS = 3;

    for (int32_t iter = 0; iter < 5; ++iter) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
        ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();

        // We expect disk full exceptions in the merge threads
        cms->setSuppressExceptions();

        writer->setMergeScheduler(cms);
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(4);

        Collection<IndexerThreadPtr> threads = Collection<IndexerThreadPtr>::newInstance(NUM_THREADS);

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i] = newLucene<IndexerThread>(writer, true);
        }

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->start();
        }

        LuceneThread::threadSleep(10);

        dir->failOn(failure);
        failure->setDoFail();

        for (int32_t i = 0; i < NUM_THREADS; ++i) {
            threads[i]->join();
        }

        bool success = false;

        try {
            writer->close(false);
            success = true;
        } catch (IOException&) {
            failure->clearDoFail();
            writer->close(false);
        }

        if (success) {
            IndexReaderPtr reader = IndexReader::open(dir, true);
            for (int32_t j = 0; j < reader->maxDoc(); ++j) {
                if (!reader->isDeleted(j)) {
                    reader->document(j);
                    reader->getTermFreqVectors(j);
                }
            }
            reader->close();
        }

        // allow time for merge threads to finish
        LuceneThread::threadSleep(1000);

        dir->close();
    }
}

/// Make sure initial IOException, and then 2nd IOException during rollback(), is OK
TEST_F(IndexWriterTest, testIOExceptionDuringAbort) {
    _testSingleThreadFailure(newLucene<FailOnlyOnAbortOrFlush>(false));
}

/// Make sure initial IOException, and then 2nd IOException during rollback(), is OK
TEST_F(IndexWriterTest, testIOExceptionDuringAbortOnlyOnce) {
    _testSingleThreadFailure(newLucene<FailOnlyOnAbortOrFlush>(true));
}

/// Make sure initial IOException, and then 2nd IOException during rollback(), with
/// multiple threads, is OK
TEST_F(IndexWriterTest, testIOExceptionDuringAbortWithThreads) {
    _testMultipleThreadsFailure(newLucene<FailOnlyOnAbortOrFlush>(false));
}

/// Make sure initial IOException, and then 2nd IOException during rollback(), with
/// multiple threads, is OK
TEST_F(IndexWriterTest, testIOExceptionDuringAbortWithThreadsOnlyOnce) {
    _testMultipleThreadsFailure(newLucene<FailOnlyOnAbortOrFlush>(true));
}

/// Test IOException in closeDocStore
TEST_F(IndexWriterTest, testIOExceptionDuringCloseDocStore) {
    _testSingleThreadFailure(newLucene<FailOnlyInCloseDocStore>(false));
}

/// Test IOException in closeDocStore
TEST_F(IndexWriterTest, testIOExceptionDuringCloseDocStoreOnlyOnce) {
    _testSingleThreadFailure(newLucene<FailOnlyInCloseDocStore>(true));
}

/// Test IOException in closeDocStore, with threads
TEST_F(IndexWriterTest, testIOExceptionDuringCloseDocStoreWithThreads) {
    _testMultipleThreadsFailure(newLucene<FailOnlyInCloseDocStore>(false));
}

/// Test IOException in closeDocStore, with threads
TEST_F(IndexWriterTest, testIOExceptionDuringCloseDocStoreWithThreadsOnlyOnce) {
    _testMultipleThreadsFailure(newLucene<FailOnlyInCloseDocStore>(true));
}

/// Test IOException in writeSegment
TEST_F(IndexWriterTest, testIOExceptionDuringWriteSegment) {
    _testSingleThreadFailure(newLucene<FailOnlyInWriteSegment>(false));
}

/// Test IOException in writeSegment
TEST_F(IndexWriterTest, testIOExceptionDuringWriteSegmentOnlyOnce) {
    _testSingleThreadFailure(newLucene<FailOnlyInWriteSegment>(true));
}

/// Test IOException in writeSegment, with threads
TEST_F(IndexWriterTest, testIOExceptionDuringWriteSegmentWithThreads) {
    _testMultipleThreadsFailure(newLucene<FailOnlyInWriteSegment>(false));
}

/// Test IOException in writeSegment, with threads
TEST_F(IndexWriterTest, testIOExceptionDuringWriteSegmentWithThreadsOnlyOnce) {
    _testMultipleThreadsFailure(newLucene<FailOnlyInWriteSegment>(true));
}

/// Test unlimited field length
TEST_F(IndexWriterTest, testUnlimitedMaxFieldLength) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    StringStream buffer;
    for (int32_t i = 0; i < 10000; ++i) {
        buffer << L" a";
    }
    buffer << L" x";
    doc->add(newLucene<Field>(L"field", buffer.str(), Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPtr t = newLucene<Term>(L"field", L"x");
    EXPECT_EQ(1, reader->docFreq(t));
    reader->close();
    dir->close();
}

/// Simulate checksum error in segments_N
TEST_F(IndexWriterTest, testSegmentsChecksumError) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // add 100 documents
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    writer->close();

    int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
    EXPECT_TRUE(gen > 1);

    String segmentsFileName = SegmentInfos::getCurrentSegmentFileName(dir);
    IndexInputPtr in = dir->openInput(segmentsFileName);
    IndexOutputPtr out = dir->createOutput(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", 1 + gen));
    out->copyBytes(in, in->length() - 1);
    uint8_t b = in->readByte();
    out->writeByte((uint8_t)(1 + b));
    out->close();
    in->close();

    IndexReaderPtr reader;
    EXPECT_NO_THROW(reader = IndexReader::open(dir, true));
    reader->close();
}

/// Test writer.commit() when ac=false
TEST_F(IndexWriterTest, testForceCommit) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(5);

    for (int32_t i = 0; i < 23; ++i) {
        addDoc(writer);
    }
    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());
    writer->commit();
    IndexReaderPtr reader2 = reader->reopen();
    EXPECT_EQ(0, reader->numDocs());
    EXPECT_EQ(23, reader2->numDocs());
    reader->close();

    for (int32_t i = 0; i < 17; ++i) {
        addDoc(writer);
    }
    EXPECT_EQ(23, reader2->numDocs());
    reader2->close();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(23, reader->numDocs());
    reader->close();
    writer->commit();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(40, reader->numDocs());
    reader->close();
    writer->close();
    dir->close();
}

/// Test exception during sync
TEST_F(IndexWriterTest, testExceptionDuringSync) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    FailOnlyInSyncPtr failure = newLucene<FailOnlyInSync>();
    dir->failOn(failure);

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    failure->setDoFail();

    ConcurrentMergeSchedulerPtr cms = newLucene<ConcurrentMergeScheduler>();
    cms->setSuppressExceptions();
    writer->setMergeScheduler(cms);

    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(5);

    for (int32_t i = 0; i < 23; ++i) {
        addDoc(writer);
        if ((i - 1) % 2 == 0) {
            try {
                writer->commit();
            } catch (IOException& e) {
                EXPECT_TRUE(check_exception(LuceneException::IO)(e));
            }
        }
    }

    cms->sync();
    EXPECT_TRUE(failure->didFail);
    failure->clearDoFail();
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(23, reader->numDocs());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testTermVectorCorruption) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    for (int32_t iter = 0; iter < 2; ++iter) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(2);
        writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
        writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
        writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

        DocumentPtr document = newLucene<Document>();

        FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
        document->add(storedField);
        writer->addDocument(document);
        writer->addDocument(document);

        document = newLucene<Document>();
        document->add(storedField);
        FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);

        document->add(termVectorField);
        writer->addDocument(document);
        writer->optimize();
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, true);
        for (int32_t i = 0; i < reader->numDocs(); ++i) {
            reader->document(i);
            reader->getTermFreqVectors(i);
        }
        reader->close();

        writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(2);
        writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
        writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
        writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

        writer->addIndexesNoOptimize(newCollection<DirectoryPtr>(newLucene<MockRAMDirectory>(dir)));
        writer->optimize();
        writer->close();
    }
    dir->close();
}

TEST_F(IndexWriterTest, testTermVectorCorruption2) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    for (int32_t iter = 0; iter < 2; ++iter) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(2);
        writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
        writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
        writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

        DocumentPtr document = newLucene<Document>();

        FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
        document->add(storedField);
        writer->addDocument(document);
        writer->addDocument(document);

        document = newLucene<Document>();
        document->add(storedField);
        FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);

        document->add(termVectorField);
        writer->addDocument(document);
        writer->optimize();
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, true);
        EXPECT_TRUE(!reader->getTermFreqVectors(0));
        EXPECT_TRUE(!reader->getTermFreqVectors(1));
        EXPECT_TRUE(reader->getTermFreqVectors(2));
        reader->close();
    }
    dir->close();
}

TEST_F(IndexWriterTest, testTermVectorCorruption3) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
    writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

    DocumentPtr document = newLucene<Document>();

    FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
    document->add(storedField);

    FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);

    document->add(termVectorField);
    for (int32_t i = 0; i < 10; ++i) {
        writer->addDocument(document);
    }
    writer->close();

    writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);
    writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));
    for (int32_t i = 0; i < 6; ++i) {
        writer->addDocument(document);
    }

    writer->optimize();
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    for (int32_t i = 0; i < 10; ++i) {
        reader->getTermFreqVectors(i);
        reader->document(i);
    }
    reader->close();
    dir->close();
}

/// Test user-specified field length
TEST_F(IndexWriterTest, testUserSpecifiedMaxFieldLength) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), 100000);

    DocumentPtr doc = newLucene<Document>();
    StringStream buffer;
    for (int32_t i = 0; i < 10000; ++i) {
        buffer << L" a";
    }
    buffer << L" x";
    doc->add(newLucene<Field>(L"field", buffer.str(), Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPtr t = newLucene<Term>(L"field", L"x");
    EXPECT_EQ(1, reader->docFreq(t));
    reader->close();
    dir->close();
}

/// Test expungeDeletes, when 2 singular merges are required
TEST_F(IndexWriterTest, testExpungeDeletes) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);

    DocumentPtr document = newLucene<Document>();

    FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
    document->add(storedField);

    FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    document->add(termVectorField);

    for (int32_t i = 0; i < 10; ++i) {
        writer->addDocument(document);
    }
    writer->close();

    IndexReaderPtr ir = IndexReader::open(dir, false);
    EXPECT_EQ(10, ir->maxDoc());
    EXPECT_EQ(10, ir->numDocs());
    ir->deleteDocument(0);
    ir->deleteDocument(7);
    EXPECT_EQ(8, ir->numDocs());
    ir->close();

    writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);

    EXPECT_EQ(8, writer->numDocs());
    EXPECT_EQ(10, writer->maxDoc());
    writer->expungeDeletes();
    EXPECT_EQ(8, writer->numDocs());
    writer->close();
    ir = IndexReader::open(dir, true);
    EXPECT_EQ(8, ir->maxDoc());
    EXPECT_EQ(8, ir->numDocs());
    ir->close();
    dir->close();
}

/// Test expungeDeletes, when many adjacent merges are required
TEST_F(IndexWriterTest, testExpungeDeletes2) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(50);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);

    DocumentPtr document = newLucene<Document>();

    FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
    document->add(storedField);

    FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    document->add(termVectorField);

    for (int32_t i = 0; i < 98; ++i) {
        writer->addDocument(document);
    }
    writer->close();

    IndexReaderPtr ir = IndexReader::open(dir, false);
    EXPECT_EQ(98, ir->maxDoc());
    EXPECT_EQ(98, ir->numDocs());
    for (int32_t i = 0; i < 98; i += 2) {
        ir->deleteDocument(i);
    }
    EXPECT_EQ(49, ir->numDocs());
    ir->close();

    writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);

    writer->setMergeFactor(3);
    EXPECT_EQ(49, writer->numDocs());
    writer->expungeDeletes();
    writer->close();
    ir = IndexReader::open(dir, true);
    EXPECT_EQ(49, ir->maxDoc());
    EXPECT_EQ(49, ir->numDocs());
    ir->close();
    dir->close();
}

/// Test expungeDeletes without waiting, when many adjacent merges are required
TEST_F(IndexWriterTest, testExpungeDeletes3) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(50);
    writer->setRAMBufferSizeMB(IndexWriter::DISABLE_AUTO_FLUSH);

    DocumentPtr document = newLucene<Document>();

    FieldPtr storedField = newLucene<Field>(L"stored", L"stored", Field::STORE_YES, Field::INDEX_NO);
    document->add(storedField);

    FieldPtr termVectorField = newLucene<Field>(L"termVector", L"termVector", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    document->add(termVectorField);

    for (int32_t i = 0; i < 98; ++i) {
        writer->addDocument(document);
    }
    writer->close();

    IndexReaderPtr ir = IndexReader::open(dir, false);
    EXPECT_EQ(98, ir->maxDoc());
    EXPECT_EQ(98, ir->numDocs());
    for (int32_t i = 0; i < 98; i += 2) {
        ir->deleteDocument(i);
    }
    EXPECT_EQ(49, ir->numDocs());
    ir->close();

    writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);

    writer->setMergeFactor(3);
    writer->expungeDeletes(false);
    writer->close();
    ir = IndexReader::open(dir, true);
    EXPECT_EQ(49, ir->maxDoc());
    EXPECT_EQ(49, ir->numDocs());
    ir->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEmptyFieldName) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"", L"a b c", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();
}

namespace TestExceptionDocumentsWriterInit {

DECLARE_SHARED_PTR(MockIndexWriter)

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        doFail = false;
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

public:
    bool doFail;

public:
    virtual bool testPoint(const String& name) {
        if (doFail && name == L"DocumentsWriter.ThreadState.init start") {
            boost::throw_exception(RuntimeException(L"intentionally failing"));
        }
        return true;
    }
};

}

TEST_F(IndexWriterTest, testExceptionDocumentsWriterInit) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    TestExceptionDocumentsWriterInit::MockIndexWriterPtr writer = newLucene<TestExceptionDocumentsWriterInit::MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a field", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->doFail = true;
    try {
        writer->addDocument(doc);
    } catch (RuntimeException& e) {
        EXPECT_TRUE(check_exception(LuceneException::Runtime)(e));
    }
    writer->close();
    checkIndex(dir);
    dir->close();
}

namespace TestExceptionJustBeforeFlush {

DECLARE_SHARED_PTR(MockIndexWriter)

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        doFail = false;
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

public:
    bool doFail;

public:
    virtual bool testPoint(const String& name) {
        if (doFail && name == L"DocumentsWriter.ThreadState.init start") {
            boost::throw_exception(RuntimeException(L"intentionally failing"));
        }
        return true;
    }
};

class CrashAnalyzer : public Analyzer {
public:
    virtual ~CrashAnalyzer() {
    }

    LUCENE_CLASS(CrashAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<CrashingFilter>(fieldName, newLucene<WhitespaceTokenizer>(reader));
    }
};

}

TEST_F(IndexWriterTest, testExceptionJustBeforeFlush) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    TestExceptionJustBeforeFlush::MockIndexWriterPtr writer = newLucene<TestExceptionJustBeforeFlush::MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDocs(2);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a field", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    AnalyzerPtr analyzer = newLucene<TestExceptionJustBeforeFlush::CrashAnalyzer>();
    DocumentPtr crashDoc = newLucene<Document>();
    crashDoc->add(newLucene<Field>(L"crash", L"do it on token 4", Field::STORE_YES, Field::INDEX_ANALYZED));

    try {
        writer->addDocument(crashDoc, analyzer);
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }
    writer->addDocument(doc);
    writer->close();
    dir->close();
}

namespace TestExceptionOnMergeInit {

DECLARE_SHARED_PTR(MockIndexWriter)

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        doFail = false;
        failed = false;
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

public:
    bool doFail;
    bool failed;

public:
    virtual bool testPoint(const String& name) {
        if (doFail && name == L"startMergeInit") {
            failed = true;
            boost::throw_exception(RuntimeException(L"intentionally failing"));
        }
        return true;
    }
};

}

TEST_F(IndexWriterTest, testExceptionOnMergeInit) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    TestExceptionOnMergeInit::MockIndexWriterPtr writer = newLucene<TestExceptionOnMergeInit::MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(2);
    writer->doFail = true;
    writer->setMergeScheduler(newLucene<ConcurrentMergeScheduler>());
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a field", Field::STORE_YES, Field::INDEX_ANALYZED));
    for (int32_t i = 0; i < 10; ++i) {
        try {
            writer->addDocument(doc);
        } catch (RuntimeException&) {
            break;
        }
    }
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();
    EXPECT_TRUE(writer->failed);
    writer->close();
    dir->close();
}

namespace TestDoBeforeAfterFlush {

DECLARE_SHARED_PTR(MockIndexWriter)

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        afterWasCalled = false;
        beforeWasCalled = false;
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

public:
    bool afterWasCalled;
    bool beforeWasCalled;

protected:
    virtual void doAfterFlush() {
        afterWasCalled = true;
    }

    virtual void doBeforeFlush() {
        beforeWasCalled = true;
    }
};

}

TEST_F(IndexWriterTest, testDoBeforeAfterFlush) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    TestDoBeforeAfterFlush::MockIndexWriterPtr writer = newLucene<TestDoBeforeAfterFlush::MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a field", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->commit();
    EXPECT_TRUE(writer->beforeWasCalled);
    EXPECT_TRUE(writer->afterWasCalled);
    writer->beforeWasCalled = false;
    writer->afterWasCalled = false;
    writer->deleteDocuments(newLucene<Term>(L"field", L"field"));
    writer->commit();
    EXPECT_TRUE(writer->beforeWasCalled);
    EXPECT_TRUE(writer->afterWasCalled);
    writer->close();

    IndexReaderPtr ir = IndexReader::open(dir, true);
    EXPECT_EQ(1, ir->maxDoc());
    EXPECT_EQ(0, ir->numDocs());
    ir->close();

    dir->close();
}

TEST_F(IndexWriterTest, testExceptionsDuringCommit) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    FailOnlyInCommitPtr failure = newLucene<FailOnlyInCommit>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a field", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    dir->failOn(failure);
    try {
        writer->close();
    } catch (RuntimeException& e) {
        EXPECT_TRUE(check_exception(LuceneException::Runtime)(e));
    }
    EXPECT_TRUE(failure->fail1 && failure->fail2);
    writer->rollback();
    dir->close();
}

namespace TestNegativePositions {

class NegativeTokenStream : public TokenStream {
public:
    NegativeTokenStream() {
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        tokens = newCollection<String>(L"a", L"b", L"c" );
        tokenIter = tokens.begin();
        first = true;
    }

    virtual ~NegativeTokenStream() {
    }

public:
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    Collection<String> tokens;
    Collection<String>::iterator tokenIter;
    bool first;

public:
    virtual bool incrementToken() {
        if (tokenIter == tokens.end()) {
            return false;
        }
        clearAttributes();
        termAtt->setTermBuffer(*tokenIter++);
        posIncrAtt->setPositionIncrement(first ? 0 : 1);
        first = false;
        return true;
    }
};

}

TEST_F(IndexWriterTest, testNegativePositions) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    TokenStreamPtr tokens = newLucene<TestNegativePositions::NegativeTokenStream>();
    doc->add(newLucene<Field>(L"field", tokens));
    writer->addDocument(doc);
    writer->commit();

    IndexSearcherPtr s = newLucene<IndexSearcher>(dir, false);
    PhraseQueryPtr pq = newLucene<PhraseQuery>();
    pq->add(newLucene<Term>(L"field", L"a"));
    pq->add(newLucene<Term>(L"field", L"b"));
    pq->add(newLucene<Term>(L"field", L"c"));
    Collection<ScoreDocPtr> hits = s->search(pq, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    QueryPtr q = newLucene<SpanTermQuery>(newLucene<Term>(L"field", L"a"));
    hits = s->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    TermPositionsPtr tps = s->getIndexReader()->termPositions(newLucene<Term>(L"field", L"a"));
    EXPECT_TRUE(tps->next());
    EXPECT_EQ(1, tps->freq());
    EXPECT_EQ(0, tps->nextPosition());
    writer->close();

    EXPECT_TRUE(checkIndex(dir));
    s->close();
    dir->close();
}

TEST_F(IndexWriterTest, testPrepareCommit) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(5);

    for (int32_t i = 0; i < 23; ++i) {
        addDoc(writer);
    }

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());

    writer->prepareCommit();

    IndexReaderPtr reader2 = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader2->numDocs());

    writer->commit();

    IndexReaderPtr reader3 = reader->reopen();
    EXPECT_EQ(0, reader->numDocs());
    EXPECT_EQ(0, reader2->numDocs());
    EXPECT_EQ(23, reader3->numDocs());
    reader->close();
    reader2->close();

    for (int32_t i = 0; i < 17; ++i) {
        addDoc(writer);
    }

    EXPECT_EQ(23, reader3->numDocs());
    reader3->close();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(23, reader->numDocs());
    reader->close();

    writer->prepareCommit();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(23, reader->numDocs());
    reader->close();

    writer->commit();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(40, reader->numDocs());
    reader->close();
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testPrepareCommitRollback) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    dir->setPreventDoubleWrite(false);

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(5);

    for (int32_t i = 0; i < 23; ++i) {
        addDoc(writer);
    }

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());

    writer->prepareCommit();

    IndexReaderPtr reader2 = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader2->numDocs());

    writer->rollback();

    IndexReaderPtr reader3 = reader->reopen();
    EXPECT_EQ(0, reader->numDocs());
    EXPECT_EQ(0, reader2->numDocs());
    EXPECT_EQ(0, reader3->numDocs());
    reader->close();
    reader2->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 17; ++i) {
        addDoc(writer);
    }

    EXPECT_EQ(0, reader3->numDocs());
    reader3->close();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());
    reader->close();

    writer->prepareCommit();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());
    reader->close();

    writer->commit();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(17, reader->numDocs());
    reader->close();
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testPrepareCommitNoChanges) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->prepareCommit();
    writer->commit();
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());
    reader->close();
    dir->close();
}

namespace TestAddIndexesWithThreads {

DECLARE_SHARED_PTR(CommitAndAddIndexes)

class CommitAndAddIndexes : public RunAddIndexesThreads {
public:
    CommitAndAddIndexes(int32_t numCopy) : RunAddIndexesThreads(numCopy) {
    }

    virtual ~CommitAndAddIndexes() {
    }

public:
    virtual void handle(LuceneException& e) {
        FAIL() << "Unexpected exception: " << e.getError();
    }

    virtual void doBody(int32_t j, Collection<DirectoryPtr> dirs) {
        switch (j % 4) {
        case 0:
            writer2->addIndexesNoOptimize(dirs);
            writer2->optimize();
            break;
        case 1:
            writer2->addIndexesNoOptimize(dirs);
            break;
        case 2:
            writer2->addIndexes(readers);
            break;
        case 3:
            writer2->commit();
            break;
        }
    }
};

}

/// Test simultaneous addIndexes & commits from multiple threads
TEST_F(IndexWriterTest, testAddIndexesWithThreads) {
    static const int32_t NUM_ITER = 12;
    static const int32_t NUM_COPY = 3;
    TestAddIndexesWithThreads::CommitAndAddIndexesPtr c = newLucene<TestAddIndexesWithThreads::CommitAndAddIndexes>(NUM_COPY);
    c->launchThreads(NUM_ITER);

    for (int32_t i = 0; i < 100; ++i) {
        addDoc(c->writer2);
    }

    c->joinThreads();

    EXPECT_EQ(100 + NUM_COPY * (3 * NUM_ITER / 4) * c->NUM_THREADS * c->NUM_INIT_DOCS, c->writer2->numDocs());

    c->close(true);

    checkIndex(c->dir2);

    IndexReaderPtr reader = IndexReader::open(c->dir2, true);
    EXPECT_EQ(100 + NUM_COPY * (3 * NUM_ITER / 4) * c->NUM_THREADS * c->NUM_INIT_DOCS, reader->numDocs());
    reader->close();

    c->closeDir();
}

namespace TestAddIndexesWithClose {

DECLARE_SHARED_PTR(CommitAndAddIndexes)

class CommitAndAddIndexes : public RunAddIndexesThreads {
public:
    CommitAndAddIndexes(int32_t numCopy) : RunAddIndexesThreads(numCopy) {
    }

    virtual ~CommitAndAddIndexes() {
    }

public:
    virtual void handle(LuceneException& e) {
        if (e.getType() != LuceneException::AlreadyClosed && e.getType() != LuceneException::NullPointer) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }

    virtual void doBody(int32_t j, Collection<DirectoryPtr> dirs) {
        switch (j % 4) {
        case 0:
            writer2->addIndexesNoOptimize(dirs);
            writer2->optimize();
            break;
        case 1:
            writer2->addIndexesNoOptimize(dirs);
            break;
        case 2:
            writer2->addIndexes(readers);
            break;
        case 3:
            writer2->commit();
            break;
        }
    }
};

}

/// Test simultaneous addIndexes & close
TEST_F(IndexWriterTest, testAddIndexesWithClose) {
    static const int32_t NUM_COPY = 3;
    TestAddIndexesWithClose::CommitAndAddIndexesPtr c = newLucene<TestAddIndexesWithClose::CommitAndAddIndexes>(NUM_COPY);
    c->launchThreads(-1);

    // Close without first stopping/joining the threads
    c->close(true);

    c->joinThreads();

    checkIndex(c->dir2);

    c->closeDir();
}

namespace TestAddIndexesWithCloseNoWait {

DECLARE_SHARED_PTR(CommitAndAddIndexes)

class CommitAndAddIndexes : public RunAddIndexesThreads {
public:
    CommitAndAddIndexes(int32_t numCopy) : RunAddIndexesThreads(numCopy) {
    }

    virtual ~CommitAndAddIndexes() {
    }

public:
    virtual void handle(LuceneException& e) {
        bool report = true;
        if (e.getType() == LuceneException::AlreadyClosed || e.getType() == LuceneException::MergeAborted || e.getType() == LuceneException::NullPointer) {
            report = !didClose;
        } else if (e.getType() == LuceneException::IO) {
            report = !didClose;
        }
        if (report) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }

    virtual void doBody(int32_t j, Collection<DirectoryPtr> dirs) {
        switch (j % 5) {
        case 0:
            writer2->addIndexesNoOptimize(dirs);
            writer2->optimize();
            break;
        case 1:
            writer2->addIndexesNoOptimize(dirs);
            break;
        case 2:
            writer2->addIndexes(readers);
            break;
        case 3:
            writer2->optimize();
        case 4:
            writer2->commit();
            break;
        }
    }
};

}

/// Test simultaneous addIndexes and close
TEST_F(IndexWriterTest, testAddIndexesWithCloseNoWait) {
    static const int32_t NUM_COPY = 50;
    TestAddIndexesWithCloseNoWait::CommitAndAddIndexesPtr c = newLucene<TestAddIndexesWithCloseNoWait::CommitAndAddIndexes>(NUM_COPY);
    c->launchThreads(-1);

    LuceneThread::threadSleep(500);

    // Close without first stopping/joining the threads
    c->close(false);

    c->joinThreads();

    checkIndex(c->dir2);

    c->closeDir();
}

namespace TestAddIndexesWithRollback {

DECLARE_SHARED_PTR(CommitAndAddIndexes)

class CommitAndAddIndexes : public RunAddIndexesThreads {
public:
    CommitAndAddIndexes(int32_t numCopy) : RunAddIndexesThreads(numCopy) {
    }

    virtual ~CommitAndAddIndexes() {
    }

public:
    virtual void handle(LuceneException& e) {
        bool report = true;
        if (e.getType() == LuceneException::AlreadyClosed || e.getType() == LuceneException::MergeAborted || e.getType() == LuceneException::NullPointer) {
            report = !didClose;
        } else if (e.getType() == LuceneException::IO) {
            report = !didClose;
        }
        if (report) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }

    virtual void doBody(int32_t j, Collection<DirectoryPtr> dirs) {
        switch (j % 5) {
        case 0:
            writer2->addIndexesNoOptimize(dirs);
            writer2->optimize();
            break;
        case 1:
            writer2->addIndexesNoOptimize(dirs);
            break;
        case 2:
            writer2->addIndexes(readers);
            break;
        case 3:
            writer2->optimize();
        case 4:
            writer2->commit();
            break;
        }
    }
};

}

/// Test simultaneous addIndexes and close
TEST_F(IndexWriterTest, testAddIndexesWithRollback) {
    static const int32_t NUM_COPY = 50;
    TestAddIndexesWithRollback::CommitAndAddIndexesPtr c = newLucene<TestAddIndexesWithRollback::CommitAndAddIndexes>(NUM_COPY);
    c->launchThreads(-1);

    LuceneThread::threadSleep(500);

    // Close without first stopping/joining the threads
    c->didClose = true;
    c->writer2->rollback();

    c->joinThreads();

    checkIndex(c->dir2);

    c->closeDir();
}

namespace TestRollbackExceptionHang {

DECLARE_SHARED_PTR(MockIndexWriter)

class MockIndexWriter : public IndexWriter {
public:
    MockIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        doFail = false;
    }

    virtual ~MockIndexWriter() {
    }

    LUCENE_CLASS(MockIndexWriter);

public:
    bool doFail;

public:
    virtual bool testPoint(const String& name) {
        if (doFail && name == L"rollback before checkpoint") {
            boost::throw_exception(RuntimeException(L"intentionally failing"));
        }
        return true;
    }
};

}

TEST_F(IndexWriterTest, testRollbackExceptionHang) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    TestRollbackExceptionHang::MockIndexWriterPtr w = newLucene<TestRollbackExceptionHang::MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    addDoc(w);
    w->doFail = true;
    try {
        w->rollback();
    } catch (RuntimeException& e) {
        EXPECT_TRUE(check_exception(LuceneException::Runtime)(e));
    }

    w->doFail = false;
    w->rollback();
}

TEST_F(IndexWriterTest, testBinaryFieldOffsetLength) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ByteArray b = ByteArray::newInstance(50);
    for (int32_t i = 0; i < 50; ++i) {
        b[i] = (uint8_t)(i + 77);
    }

    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"binary", b, 10, 17, Field::STORE_YES);
    ByteArray bx = f->getBinaryValue();
    EXPECT_TRUE(bx);
    EXPECT_EQ(50, bx.size());
    EXPECT_EQ(10, f->getBinaryOffset());
    EXPECT_EQ(17, f->getBinaryLength());
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    doc = reader->document(0);
    f = doc->getField(L"binary");
    b = f->getBinaryValue();
    EXPECT_TRUE(b);
    EXPECT_EQ(17, b.size());
    EXPECT_EQ(87, b[0]);
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testCommitUserData) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t j = 0; j < 17; ++j) {
        addDoc(writer);
    }
    writer->close();

    EXPECT_EQ(0, IndexReader::getCommitUserData(dir).size());

    IndexReaderPtr reader = IndexReader::open(dir, true);
    // commit(Map) never called for this index
    EXPECT_EQ(0, reader->getCommitUserData().size());
    reader->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t j = 0; j < 17; ++j) {
        addDoc(writer);
    }
    MapStringString data = MapStringString::newInstance();
    data.put(L"label", L"test1");
    writer->commit(data);
    writer->close();

    EXPECT_EQ(L"test1", IndexReader::getCommitUserData(dir).get(L"label"));

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(L"test1", reader->getCommitUserData().get(L"label"));
    reader->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();

    EXPECT_EQ(L"test1", IndexReader::getCommitUserData(dir).get(L"label"));

    dir->close();
}

TEST_F(IndexWriterTest, testOptimizeExceptions) {
    MockRAMDirectoryPtr startDir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(startDir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(100);
    for (int32_t i = 0; i < 27; ++i) {
        addDoc(writer);
    }
    writer->close();

    for (int32_t i = 0; i < 200; ++i) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>(startDir);
        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
        boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();
        dir->setRandomIOExceptionRate(0.5, 100);

        try {
            writer->optimize();
        } catch (IOException& e) {
            EXPECT_TRUE(check_exception(LuceneException::IO)(e));
        }

        // Make sure we don't hit random exception during close below
        dir->setRandomIOExceptionRate(0.0, 0);

        writer->close();
        dir->close();
    }
}

namespace TestOutOfMemoryErrorCausesCloseToFail {

class MemoryIndexWriter : public IndexWriter {
public:
    MemoryIndexWriter(const DirectoryPtr& d, const AnalyzerPtr& a, bool create, int32_t mfl) : IndexWriter(d, a, create, mfl) {
        thrown = false;
    }

    virtual ~MemoryIndexWriter() {
    }

    LUCENE_CLASS(MemoryIndexWriter);

protected:
    bool thrown;

public:
    virtual void message(const String& message) {
        if (boost::starts_with(message, L"now flush at close") && !thrown) {
            thrown = true;
            boost::throw_exception(std::bad_alloc());
        }
    }
};

}

TEST_F(IndexWriterTest, testOutOfMemoryErrorCausesCloseToFail) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<TestOutOfMemoryErrorCausesCloseToFail::MemoryIndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setInfoStream(newLucene<InfoStreamNull>());

    try {
        writer->close();
    } catch (OutOfMemoryError& e) {
        EXPECT_TRUE(check_exception(LuceneException::OutOfMemory)(e));
    }

    writer->close();
}

TEST_F(IndexWriterTest, testDoubleOffsetCounting) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f);
    FieldPtr f2 = newLucene<Field>(L"field", L"", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f2);
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);

    // Token "" occurred once
    EXPECT_EQ(1, termOffsets.size());
    EXPECT_EQ(8, termOffsets[0]->getStartOffset());
    EXPECT_EQ(8, termOffsets[0]->getEndOffset());

    // Token "abcd" occurred three times
    termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(1);
    EXPECT_EQ(3, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(4, termOffsets[1]->getStartOffset());
    EXPECT_EQ(8, termOffsets[1]->getEndOffset());
    EXPECT_EQ(8, termOffsets[2]->getStartOffset());
    EXPECT_EQ(12, termOffsets[2]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testDoubleOffsetCounting2) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);
    EXPECT_EQ(2, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(5, termOffsets[1]->getStartOffset());
    EXPECT_EQ(9, termOffsets[1]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionCharAnalyzer) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd   ", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);

    EXPECT_EQ(2, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(8, termOffsets[1]->getStartOffset());
    EXPECT_EQ(12, termOffsets[1]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionWithCachingTokenFilter) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    TokenStreamPtr stream = newLucene<CachingTokenFilter>(analyzer->tokenStream(L"field", newLucene<StringReader>(L"abcd   ")));
    FieldPtr f = newLucene<Field>(L"field", stream, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);

    EXPECT_EQ(2, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(8, termOffsets[1]->getStartOffset());
    EXPECT_EQ(12, termOffsets[1]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionWithTeeSinkTokenFilter) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    TeeSinkTokenFilterPtr tee = newLucene<TeeSinkTokenFilter>(analyzer->tokenStream(L"field", newLucene<StringReader>(L"abcd   ")));
    TokenStreamPtr sink = tee->newSinkTokenStream();
    FieldPtr f1 = newLucene<Field>(L"field", tee, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    FieldPtr f2 = newLucene<Field>(L"field", sink, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f1);
    doc->add(f2);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);

    EXPECT_EQ(2, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(8, termOffsets[1]->getStartOffset());
    EXPECT_EQ(12, termOffsets[1]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionStopFilter) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd the", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    Collection<TermVectorOffsetInfoPtr> termOffsets = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"))->getOffsets(0);

    EXPECT_EQ(2, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    EXPECT_EQ(9, termOffsets[1]->getStartOffset());
    EXPECT_EQ(13, termOffsets[1]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionStandard) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd the  ", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    FieldPtr f2 = newLucene<Field>(L"field", L"crunch man", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f2);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPositionVectorPtr tpv = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"));
    Collection<TermVectorOffsetInfoPtr> termOffsets = tpv->getOffsets(0);

    EXPECT_EQ(1, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    termOffsets = tpv->getOffsets(1);
    EXPECT_EQ(11, termOffsets[0]->getStartOffset());
    EXPECT_EQ(17, termOffsets[0]->getEndOffset());
    termOffsets = tpv->getOffsets(2);
    EXPECT_EQ(18, termOffsets[0]->getStartOffset());
    EXPECT_EQ(21, termOffsets[0]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionStandardEmptyField) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    FieldPtr f2 = newLucene<Field>(L"field", L"crunch man", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(f2);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPositionVectorPtr tpv = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"));
    Collection<TermVectorOffsetInfoPtr> termOffsets = tpv->getOffsets(0);

    EXPECT_EQ(1, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(6, termOffsets[0]->getEndOffset());
    termOffsets = tpv->getOffsets(1);
    EXPECT_EQ(7, termOffsets[0]->getStartOffset());
    EXPECT_EQ(10, termOffsets[0]->getEndOffset());
    reader->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEndOffsetPositionStandardEmptyField2) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"field", L"abcd", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f);
    doc->add(newLucene<Field>(L"field", L"", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    FieldPtr f2 = newLucene<Field>(L"field", L"crunch", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS);
    doc->add(f2);
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermPositionVectorPtr tpv = boost::dynamic_pointer_cast<TermPositionVector>(reader->getTermFreqVector(0, L"field"));
    Collection<TermVectorOffsetInfoPtr> termOffsets = tpv->getOffsets(0);

    EXPECT_EQ(1, termOffsets.size());
    EXPECT_EQ(0, termOffsets[0]->getStartOffset());
    EXPECT_EQ(4, termOffsets[0]->getEndOffset());
    termOffsets = tpv->getOffsets(1);
    EXPECT_EQ(5, termOffsets[0]->getStartOffset());
    EXPECT_EQ(11, termOffsets[0]->getEndOffset());
    reader->close();
    dir->close();
}

/// Make sure opening an IndexWriter with create=true does not remove non-index files
TEST_F(IndexWriterTest, testOtherFiles) {
    String indexDir(FileUtils::joinPath(getTempDir(), L"otherfiles"));
    DirectoryPtr dir = FSDirectory::open(indexDir);

    LuceneException finally;
    try {
        // Create my own random file
        IndexOutputPtr out = dir->createOutput(L"myrandomfile");
        out->writeByte((uint8_t)42);
        out->close();

        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->close();

        EXPECT_TRUE(dir->fileExists(L"myrandomfile"));

        // Make sure this does not copy myrandomfile
        DirectoryPtr dir2 = newLucene<RAMDirectory>(dir);
        EXPECT_TRUE(!dir2->fileExists(L"myrandomfile"));
    } catch (LuceneException& e) {
        finally = e;
    }
    dir->close();
    FileUtils::removeDirectory(indexDir);
    finally.throwException();
}

TEST_F(IndexWriterTest, testDeadlock) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa bbb ccc ddd eee fff ggg hhh iii", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
    writer->addDocument(doc);
    writer->addDocument(doc);
    writer->commit();
    // index has 2 segments

    MockRAMDirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer2 = newLucene<IndexWriter>(dir2, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    writer2->addDocument(doc);
    writer2->close();

    IndexReaderPtr r1 = IndexReader::open(dir2, true);
    IndexReaderPtr r2 = boost::dynamic_pointer_cast<IndexReader>(r1->clone());

    writer->addIndexes(newCollection<IndexReaderPtr>(r1, r2));
    writer->close();

    IndexReaderPtr r3 = IndexReader::open(dir, true);
    EXPECT_EQ(5, r3->numDocs());
    r3->close();

    r1->close();
    r2->close();

    dir2->close();
    dir->close();
}

TEST_F(IndexWriterTest, testIndexStoreCombos) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ByteArray b = ByteArray::newInstance(50);
    for (int32_t i = 0; i < 50; ++i) {
        b[i] = (uint8_t)(i + 77);
    }
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"binary", b, 10, 17, Field::STORE_YES);

    f->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc1field1")));
    FieldPtr f2 = newLucene<Field>(L"string", L"value", Field::STORE_YES, Field::INDEX_ANALYZED);
    f2->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc1field2")));
    doc->add(f);
    doc->add(f2);
    writer->addDocument(doc);

    // add 2 docs to test in-memory merging
    f->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc2field1")));
    f2->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc2field2")));
    writer->addDocument(doc);

    // force segment flush so we can force a segment merge with doc3 later.
    writer->commit();

    f->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc3field1")));
    f2->setTokenStream(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"doc3field2")));

    writer->addDocument(doc);
    writer->commit();
    writer->optimize(); // force segment merge.

    IndexReaderPtr reader = IndexReader::open(dir, true);
    doc = reader->document(0);
    f = doc->getField(L"binary");
    b = f->getBinaryValue();
    EXPECT_TRUE(b);
    EXPECT_EQ(17, b.size());
    EXPECT_EQ(87, b[0]);

    EXPECT_TRUE(reader->document(0)->getFieldable(L"binary")->isBinary());
    EXPECT_TRUE(reader->document(1)->getFieldable(L"binary")->isBinary());
    EXPECT_TRUE(reader->document(2)->getFieldable(L"binary")->isBinary());

    EXPECT_EQ(L"value", reader->document(0)->get(L"string"));
    EXPECT_EQ(L"value", reader->document(1)->get(L"string"));
    EXPECT_EQ(L"value", reader->document(2)->get(L"string"));

    // test that the terms were indexed.
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"binary", L"doc1field1"))->next());
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"binary", L"doc2field1"))->next());
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"binary", L"doc3field1"))->next());
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"string", L"doc1field2"))->next());
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"string", L"doc2field2"))->next());
    EXPECT_TRUE(reader->termDocs(newLucene<Term>(L"string", L"doc3field2"))->next());

    reader->close();
    dir->close();
}

/// Make sure doc fields are stored in order
TEST_F(IndexWriterTest, testStoredFieldsOrder) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"zzz", L"a b c", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"aaa", L"a b c", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"zzz", L"1 2 3", Field::STORE_YES, Field::INDEX_NO));
    writer->addDocument(doc);
    IndexReaderPtr reader = writer->getReader();
    doc = reader->document(0);
    Collection<FieldablePtr> fields = doc->getFields();

    EXPECT_EQ(3, fields.size());
    EXPECT_EQ(fields[0]->name(), L"zzz");
    EXPECT_EQ(fields[0]->stringValue(), L"a b c");
    EXPECT_EQ(fields[1]->name(), L"aaa");
    EXPECT_EQ(fields[1]->stringValue(), L"a b c");
    EXPECT_EQ(fields[2]->name(), L"zzz");
    EXPECT_EQ(fields[2]->stringValue(), L"1 2 3");
    reader->close();
    writer->close();
    dir->close();
}

TEST_F(IndexWriterTest, testEmbeddedFFFF) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();

    const wchar_t _field[] = {L'a', L' ', L'a', UTF8Base::UNICODE_TERMINATOR, L'b'};
    String field(_field, SIZEOF_ARRAY(_field));

    doc->add(newLucene<Field>(L"field", field, Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    checkIndex(dir);
    dir->close();
}

TEST_F(IndexWriterTest, testNoDocsIndex) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setUseCompoundFile(false);
    writer->addDocument(newLucene<Document>());
    writer->close();

    checkIndex(dir);
    dir->close();
}

namespace TestCommitThreadSafety {

class CommitThread : public LuceneThread {
public:
    CommitThread(int32_t finalI, const IndexWriterPtr& writer, const DirectoryPtr& dir, int64_t endTime) {
        this->finalI = finalI;
        this->writer = writer;
        this->dir = dir;
        this->endTime = endTime;
    }

    virtual ~CommitThread() {
    }

    LUCENE_CLASS(CommitThread);

protected:
    int32_t finalI;
    IndexWriterPtr writer;
    DirectoryPtr dir;
    int64_t endTime;

public:
    virtual void run() {
        try {
            DocumentPtr doc = newLucene<Document>();
            IndexReaderPtr reader = IndexReader::open(dir);
            FieldPtr field = newLucene<Field>(L"f", L"", Field::STORE_NO, Field::INDEX_NOT_ANALYZED);
            doc->add(field);
            int32_t count = 0;
            while ((int64_t)MiscUtils::currentTimeMillis() < endTime) {
                for (int32_t j = 0; j < 10; ++j) {
                    String s = StringUtils::toString(finalI) + L"_" + StringUtils::toString(count++);
                    field->setValue(s);
                    writer->addDocument(doc);
                    writer->commit();
                    IndexReaderPtr reader2 = reader->reopen();
                    EXPECT_NE(reader2, reader);
                    reader->close();
                    reader = reader2;
                    EXPECT_EQ(1, reader->docFreq(newLucene<Term>(L"f", s)));
                }
            }
            reader->close();
        } catch (...) {
            FAIL() << "Unexpected exception";
        }
    }
};

}

/// make sure with multiple threads commit doesn't return until all changes are in fact in the index
TEST_F(IndexWriterTest, testCommitThreadSafety) {
    static const int32_t NUM_THREADS = 5;
    double RUN_SEC = 0.5;
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->commit();
    Collection<LuceneThreadPtr> threads = Collection<LuceneThreadPtr>::newInstance(NUM_THREADS);
    int64_t endTime = (int64_t)MiscUtils::currentTimeMillis() + (int64_t)(RUN_SEC * 1000.0);
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i] = newLucene<TestCommitThreadSafety::CommitThread>(i, writer, dir, endTime);
    }
    for (int32_t i = 0; i < NUM_THREADS; ++i) {
        threads[i]->join();
    }
    writer->close();
    dir->close();
}

namespace TestCorruptionAfterDiskFullDuringMerge {

DECLARE_SHARED_PTR(FailTwiceDuringMerge)

class FailTwiceDuringMerge : public MockDirectoryFailure {
public:
    FailTwiceDuringMerge() {
        didFail1 = false;
        didFail2 = false;
    }

    virtual ~FailTwiceDuringMerge() {
    }

public:
    bool didFail1;
    bool didFail2;

public:
    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (!doFail) {
            return;
        }
        if (TestPoint::getTestPoint(L"SegmentMerger", L"mergeTerms") && !didFail1) {
            didFail1 = true;
            boost::throw_exception(IOException(L"fake disk full during mergeTerms"));
        }
        if (TestPoint::getTestPoint(L"BitVector", L"write") && !didFail2) {
            didFail2 = true;
            boost::throw_exception(IOException(L"fake disk full while writing BitVector"));
        }
    }
};

}

TEST_F(IndexWriterTest, testCorruptionAfterDiskFullDuringMerge) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    dir->setPreventDoubleWrite(false);
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    w->setMergeScheduler(newLucene<SerialMergeScheduler>());

    boost::dynamic_pointer_cast<LogMergePolicy>(w->getMergePolicy())->setMergeFactor(2);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"f", L"doctor who", Field::STORE_YES, Field::INDEX_ANALYZED));
    w->addDocument(doc);

    w->commit();

    w->deleteDocuments(newLucene<Term>(L"f", L"who"));
    w->addDocument(doc);

    // disk fills up!
    TestCorruptionAfterDiskFullDuringMerge::FailTwiceDuringMergePtr ftdm = newLucene<TestCorruptionAfterDiskFullDuringMerge::FailTwiceDuringMerge>();
    ftdm->setDoFail();
    dir->failOn(ftdm);

    try {
        w->commit();
        FAIL() << "fake disk full IOExceptions not hit";
    } catch (IOException&) {
    }

    EXPECT_TRUE(ftdm->didFail1 || ftdm->didFail2);

    checkIndex(dir);
    ftdm->clearDoFail();
    w->addDocument(doc);
    w->close();

    checkIndex(dir);
    dir->close();
}

namespace TestFutureCommit {

class NoDeletionPolicy : public IndexDeletionPolicy {
public:
    virtual ~NoDeletionPolicy() {
    }

    LUCENE_CLASS(NoDeletionPolicy);

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
    }
};

}

TEST_F(IndexWriterTest, testFutureCommit) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();

    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), (IndexDeletionPolicyPtr)newLucene<TestFutureCommit::NoDeletionPolicy>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    w->addDocument(doc);

    // commit to "first"
    MapStringString commitData = MapStringString::newInstance();
    commitData.put(L"tag", L"first");
    w->commit(commitData);

    // commit to "second"
    w->addDocument(doc);
    commitData.put(L"tag", L"second");
    w->commit(commitData);
    w->close();

    // open "first" with IndexWriter
    IndexCommitPtr commit;
    Collection<IndexCommitPtr> commits = IndexReader::listCommits(dir);
    for (Collection<IndexCommitPtr>::iterator c = commits.begin(); c != commits.end(); ++c) {
        String tag = (*c)->getUserData().get(L"tag");
        if (tag == L"first") {
            commit = *c;
            break;
        }
    }

    EXPECT_TRUE(commit);

    w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), newLucene<TestFutureCommit::NoDeletionPolicy>(), IndexWriter::MaxFieldLengthUNLIMITED, commit);

    EXPECT_EQ(1, w->numDocs());

    // commit IndexWriter to "third"
    w->addDocument(doc);
    commitData.put(L"tag", L"third");
    w->commit(commitData);
    w->close();

    // make sure "second" commit is still there
    commit.reset();
    commits = IndexReader::listCommits(dir);
    for (Collection<IndexCommitPtr>::iterator c = commits.begin(); c != commits.end(); ++c) {
        String tag = (*c)->getUserData().get(L"tag");
        if (tag == L"second") {
            commit = *c;
            break;
        }
    }

    EXPECT_TRUE(commit);

    IndexReaderPtr r = IndexReader::open(commit, true);
    EXPECT_EQ(2, r->numDocs());
    r->close();

    // open "second", with writeable IndexReader & commit
    r = IndexReader::open(commit, newLucene<TestFutureCommit::NoDeletionPolicy>(), false);
    EXPECT_EQ(2, r->numDocs());
    r->deleteDocument(0);
    r->deleteDocument(1);
    commitData.put(L"tag", L"fourth");
    r->commit(commitData);
    r->close();

    // make sure "third" commit is still there
    commit.reset();
    commits = IndexReader::listCommits(dir);
    for (Collection<IndexCommitPtr>::iterator c = commits.begin(); c != commits.end(); ++c) {
        String tag = (*c)->getUserData().get(L"tag");
        if (tag == L"third") {
            commit = *c;
            break;
        }
    }

    EXPECT_TRUE(commit);

    dir->close();
}

TEST_F(IndexWriterTest, testNoUnwantedTVFiles) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr indexWriter = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    indexWriter->setRAMBufferSizeMB(0.01);
    indexWriter->setUseCompoundFile(false);

    String BIG = L"alskjhlaksjghlaksjfhalksvjepgjioefgjnsdfjgefgjhelkgjhqewlrkhgwlekgrhwelkgjhwelkgrhwlkejg";
    BIG += BIG + BIG + BIG;

    for (int32_t i = 0; i < 2; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", StringUtils::toString(i) + BIG, Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS));
        doc->add(newLucene<Field>(L"str", StringUtils::toString(i) + BIG, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"str2", StringUtils::toString(i) + BIG, Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"str3", StringUtils::toString(i) + BIG, Field::STORE_YES, Field::INDEX_ANALYZED_NO_NORMS));
        indexWriter->addDocument(doc);
    }

    indexWriter->close();

    checkIndex(dir);

    checkNoUnreferencedFiles(dir);
    HashSet<String> files = dir->listAll();
    for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file) {
        EXPECT_TRUE(!boost::ends_with(*file, IndexFileNames::VECTORS_FIELDS_EXTENSION()));
        EXPECT_TRUE(!boost::ends_with(*file, IndexFileNames::VECTORS_INDEX_EXTENSION()));
        EXPECT_TRUE(!boost::ends_with(*file, IndexFileNames::VECTORS_DOCUMENTS_EXTENSION()));
    }

    dir->close();
}
