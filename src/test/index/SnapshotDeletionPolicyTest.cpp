/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FSDirectory.h"
#include "MockRAMDirectory.h"
#include "SnapshotDeletionPolicy.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "LuceneThread.h"
#include "IndexFileDeleter.h"
#include "IndexInput.h"
#include "MiscUtils.h"
#include "FileUtils.h"

using namespace Lucene;

class SnapshotThread : public LuceneThread {
public:
    SnapshotThread(int64_t stopTime, const IndexWriterPtr& writer) {
        this->stopTime = stopTime;
        this->writer = writer;
    }

    virtual ~SnapshotThread() {
    }

    LUCENE_CLASS(SnapshotThread);

protected:
    int64_t stopTime;
    IndexWriterPtr writer;

public:
    virtual void run() {
        try {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
            do {
                for (int32_t i = 0; i < 27; ++i) {
                    writer->addDocument(doc);
                    if (i % 2 == 0) {
                        writer->commit();
                    }
                }
                LuceneThread::threadSleep(1);
            } while ((int64_t)MiscUtils::currentTimeMillis() < stopTime);
        } catch (LuceneException& e) {
            FAIL() << "Unexpected exception: " << e.getError();
        }
    }
};

class SnapshotDeletionPolicyTest : public LuceneTestFixture {
public:
    SnapshotDeletionPolicyTest() {
        buffer = ByteArray::newInstance(4096);
    }

    virtual ~SnapshotDeletionPolicyTest() {
    }

public:
    static const String INDEX_PATH;
    ByteArray buffer;

public:
    void runTest(const DirectoryPtr& dir) {
        // Run for ~1 seconds
        int64_t stopTime = MiscUtils::currentTimeMillis() + 1000;

        SnapshotDeletionPolicyPtr dp = newLucene<SnapshotDeletionPolicy>(newLucene<KeepOnlyLastCommitDeletionPolicy>());
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), (IndexDeletionPolicyPtr)dp, IndexWriter::MaxFieldLengthUNLIMITED);

        // Force frequent flushes
        writer->setMaxBufferedDocs(2);

        LuceneThreadPtr thread = newLucene<SnapshotThread>(stopTime, writer);
        thread->start();

        // While the above indexing thread is running, take many backups
        do {
            backupIndex(dir, dp);
            LuceneThread::threadSleep(20);
            if (!thread->isAlive()) {
                break;
            }
        } while ((int64_t)MiscUtils::currentTimeMillis() < stopTime);

        thread->join();

        // Add one more document to force writer to commit a final segment, so deletion policy has a chance to delete again
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->addDocument(doc);

        // Make sure we don't have any leftover files in the directory
        writer->close();
        checkNoUnreferencedFiles(dir);
    }

    /// Example showing how to use the SnapshotDeletionPolicy to take a backup.  This method does not
    /// really do a backup; instead, it reads every byte of every file just to test that the files
    /// indeed exist and are readable even while the index is changing.
    void backupIndex(const DirectoryPtr& dir, const SnapshotDeletionPolicyPtr& dp) {
        // To backup an index we first take a snapshot
        LuceneException finally;
        try {
            copyFiles(dir, boost::dynamic_pointer_cast<IndexCommit>(dp->snapshot()));
        } catch (LuceneException& e) {
            finally = e;
        }

        // Make sure to release the snapshot, otherwise these files will never be deleted during this
        // IndexWriter session
        dp->release();
        finally.throwException();
    }

    void copyFiles(const DirectoryPtr& dir, const IndexCommitPtr& cp) {
        // While we hold the snapshot, and nomatter how long we take to do the backup, the IndexWriter will
        // never delete the files in the snapshot
        HashSet<String> files = cp->getFileNames();
        for (HashSet<String>::iterator fileName = files.begin(); fileName != files.end(); ++fileName) {
            // NOTE: in a real backup you would not use readFile; you would need to use something else
            // that copies the file to a backup location.  This could even be a spawned shell process
            // (eg "tar", "zip") that takes the list of files and builds a backup.
            readFile(dir, *fileName);
        }
    }

    void readFile(const DirectoryPtr& dir, const String& name) {
        IndexInputPtr input = dir->openInput(name);

        LuceneException finally;
        try {
            int64_t size = dir->fileLength(name);
            int64_t bytesLeft = size;
            while (bytesLeft > 0) {
                int32_t numToRead = bytesLeft < buffer.size() ? (int32_t)bytesLeft : buffer.size();
                input->readBytes(buffer.get(), 0, numToRead, false);
                bytesLeft -= numToRead;
            }
            // Don't do this in your real backups!  This is just to force a backup to take a somewhat
            // long time, to make sure we are exercising the fact that the IndexWriter should not delete
            // this file even when I take my time reading it.
            LuceneThread::threadSleep(1);
        } catch (LuceneException& e) {
            finally = e;
        }
        input->close();
        finally.throwException();
    }

    void checkNoUnreferencedFiles(const DirectoryPtr& dir) {
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
};

const String SnapshotDeletionPolicyTest::INDEX_PATH = L"test.snapshots";

TEST_F(SnapshotDeletionPolicyTest, testSnapshotDeletionPolicy) {
    String dir = getTempDir(INDEX_PATH);

    LuceneException finally;
    try {
        DirectoryPtr fsDir = FSDirectory::open(dir);
        runTest(fsDir);
        fsDir->close();
    } catch (LuceneException& e) {
        finally = e;
    }
    FileUtils::removeDirectory(dir);
    finally.throwException();

    MockRAMDirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    runTest(dir2);
    dir2->close();
}

TEST_F(SnapshotDeletionPolicyTest, testNoCommits) {
    // Tests that if there were no commits when snapshot() is called, then
    // IllegalStateException is thrown rather than NPE.
    SnapshotDeletionPolicyPtr sdp = newLucene<SnapshotDeletionPolicy>(newLucene<KeepOnlyLastCommitDeletionPolicy>());
    try {
        sdp->snapshot();
    } catch (IllegalStateException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalState)(e));
    }
}
