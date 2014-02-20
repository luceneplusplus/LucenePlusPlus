/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <fstream>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockLock.h"
#include "MockLockFactory.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "NoLockFactory.h"
#include "SimpleFSLockFactory.h"
#include "NativeFSLockFactory.h"
#include "SingleInstanceLockFactory.h"
#include "FSDirectory.h"
#include "LuceneThread.h"
#include "TermQuery.h"
#include "Term.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture LockFactoryTest;

static void addDoc(const IndexWriterPtr& writer) {
    DocumentPtr doc(newLucene<Document>());
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

// Verify: we can provide our own LockFactory implementation, the right
// methods are called at the right time, locks are created, etc.
TEST_F(LockFactoryTest, testCustomLockFactory) {
    DirectoryPtr dir(newLucene<RAMDirectory>());
    MockLockFactoryPtr lf(newLucene<MockLockFactory>());
    dir->setLockFactory(lf);

    // Lock prefix should have been set
    EXPECT_TRUE(lf->lockPrefixSet);

    IndexWriterPtr writer(newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED));

    // add 100 documents (so that commit lock is used)
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }

    // Both write lock and commit lock should have been created
    EXPECT_EQ(lf->locksCreated.size(), 1); // # of unique locks created (after instantiating IndexWriter)
    EXPECT_TRUE(lf->makeLockCount >= 1); // # calls to makeLock is 0 (after instantiating IndexWriter)

    for (MapStringLock::iterator lockName = lf->locksCreated.begin(); lockName != lf->locksCreated.end(); ++lockName) {
        MockLockPtr lock(boost::dynamic_pointer_cast<MockLock>(lockName->second));
        EXPECT_TRUE(lock->lockAttempts > 0); // # calls to Lock.obtain is 0 (after instantiating IndexWriter)
    }

    writer->close();
}

// Verify: we can use the NoLockFactory with RAMDirectory with no exceptions raised
// Verify: NoLockFactory allows two IndexWriters
TEST_F(LockFactoryTest, testRAMDirectoryNoLocking) {
    DirectoryPtr dir(newLucene<RAMDirectory>());
    dir->setLockFactory(NoLockFactory::getNoLockFactory());

    EXPECT_TRUE(dir->getLockFactory());

    IndexWriterPtr writer(newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED));

    IndexWriterPtr writer2;

    // Create a 2nd IndexWriter.  This is normally not allowed but it should run through since we're not using any locks
    EXPECT_NO_THROW(writer2 = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED));

    writer->close();
    if (writer2) {
        writer2->close();
    }
}

// Verify: SingleInstanceLockFactory is the default lock for RAMDirectory
// Verify: RAMDirectory does basic locking correctly (can't create two IndexWriters)
TEST_F(LockFactoryTest, testDefaultRAMDirectory) {
    DirectoryPtr dir(newLucene<RAMDirectory>());
    LockFactoryPtr lockFactory(dir->getLockFactory());
    EXPECT_TRUE(boost::dynamic_pointer_cast<SingleInstanceLockFactory>(lockFactory));

    IndexWriterPtr writer(newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED));

    IndexWriterPtr writer2;

    // Create a 2nd IndexWriter - This should fail
    try {
        writer2 = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::LockObtainFailed)(e));
    }

    writer->close();
    if (writer2) {
        writer2->close();
    }
}

// test string file instantiation
TEST_F(LockFactoryTest, testSimpleFSLockFactory) {
    EXPECT_NO_THROW(newLucene<SimpleFSLockFactory>(L"test"));
}

namespace LockFactoryTestNS {

DECLARE_SHARED_PTR(WriterThread)
DECLARE_SHARED_PTR(LockFactorySearcherThread)

class WriterThread : public LuceneThread {
public:
    WriterThread(int32_t numIteration, const DirectoryPtr& dir) {
        this->numIteration = numIteration;
        this->dir = dir;
        this->hitException = false;
    }

    virtual ~WriterThread() {
    }

    LUCENE_CLASS(WriterThread);

public:
    bool hitException;

protected:
    DirectoryPtr dir;
    int32_t numIteration;

public:
    virtual void run() {
        WhitespaceAnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
        IndexWriterPtr writer;
        for (int32_t i = 0; i < numIteration; ++i) {
            try {
                writer = newLucene<IndexWriter>(dir, analyzer, false, IndexWriter::MaxFieldLengthLIMITED);
            } catch (IOException& e) {
                if (e.getError().find(L" timed out:") == String::npos) {
                    hitException = true;
                    FAIL() << "Stress Test Index Writer: creation hit unexpected IO exception: " << e.getError();
                    break;
                } else {
                    // lock obtain timed out
                }
            } catch (LuceneException& e) {
                hitException = true;
                FAIL() << "Stress Test Index Writer: creation hit unexpected exception: " << e.getError();
                break;
            }
            if (writer) {
                try {
                    addDoc(writer);
                } catch (LuceneException& e) {
                    hitException = true;
                    FAIL() << "Stress Test Index Writer: addDoc hit unexpected exception: " << e.getError();
                    break;
                }
                try {
                    writer->close();
                } catch (LuceneException& e) {
                    hitException = true;
                    FAIL() << "Stress Test Index Writer: close hit unexpected exception: " << e.getError();
                    break;
                }
            }
        }
    }
};

class LockFactorySearcherThread : public LuceneThread {
public:
    LockFactorySearcherThread(int32_t numIteration, const DirectoryPtr& dir) {
        this->numIteration = numIteration;
        this->dir = dir;
        this->hitException = false;
    }

    virtual ~LockFactorySearcherThread() {
    }

    LUCENE_CLASS(LockFactorySearcherThread);

public:
    bool hitException;

protected:
    DirectoryPtr dir;
    int32_t numIteration;


public:
    virtual void run() {
        IndexSearcherPtr searcher;
        QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"content", L"aaa"));
        for (int32_t i = 0; i < numIteration; ++i) {
            try {
                searcher = newLucene<IndexSearcher>(dir, false);
            } catch (LuceneException& e) {
                hitException = true;
                FAIL() << "Stress Test Index Searcher: creation hit unexpected exception: " << e.getError();
                break;
            }
            if (searcher) {
                Collection<ScoreDocPtr> hits;
                try {
                    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
                } catch (LuceneException& e) {
                    hitException = true;
                    FAIL() << "Stress Test Index Searcher: search hit unexpected exception: " << e.getError();
                    break;
                }
                try {
                    searcher->close();
                } catch (LuceneException& e) {
                    hitException = true;
                    FAIL() << "Stress Test Index Searcher: close hit unexpected exception: " << e.getError();
                    break;
                }
            }
        }
    }
};

}

static void _testStressLocks(const LockFactoryPtr& lockFactory, const String& indexDir) {
    FSDirectoryPtr fs1 = FSDirectory::open(indexDir, lockFactory);

    // First create a 1 doc index
    IndexWriterPtr w = newLucene<IndexWriter>(fs1, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(w);
    w->close();

    LockFactoryTestNS::WriterThreadPtr writer = newLucene<LockFactoryTestNS::WriterThread>(100, fs1);
    LockFactoryTestNS::LockFactorySearcherThreadPtr searcher = newLucene<LockFactoryTestNS::LockFactorySearcherThread>(100, fs1);
    writer->start();
    searcher->start();

    writer->join();
    searcher->join();

    EXPECT_TRUE(!writer->hitException);
    EXPECT_TRUE(!searcher->hitException);

    FileUtils::removeDirectory(indexDir);
}

// Verify: do stress test, by opening IndexReaders and IndexWriters over and over in 2 threads and making sure
// no unexpected exceptions are raised
TEST_F(LockFactoryTest, testStressLocks) {
    _testStressLocks(LockFactoryPtr(), getTempDir(L"index.TestLockFactory6"));
}

// Verify: do stress test, by opening IndexReaders and IndexWriters over and over in 2 threads and making sure
// no unexpected exceptions are raised, but use NativeFSLockFactory
TEST_F(LockFactoryTest, testStressLocksNativeFSLockFactory) {
    String dir(getTempDir(L"index.TestLockFactory7"));
    _testStressLocks(newLucene<NativeFSLockFactory>(dir), dir);
}

// Verify: NativeFSLockFactory works correctly
TEST_F(LockFactoryTest, testNativeFSLockFactory) {
    NativeFSLockFactoryPtr f(newLucene<NativeFSLockFactory>(getTempDir()));

    f->setLockPrefix(L"test");
    LockPtr l(f->makeLock(L"commit"));
    LockPtr l2(f->makeLock(L"commit"));

    EXPECT_TRUE(l->obtain());
    EXPECT_TRUE(!l2->obtain());
    l->release();

    EXPECT_TRUE(l2->obtain());
    l2->release();

    // Make sure we can obtain first one again, test isLocked()
    EXPECT_TRUE(l->obtain());
    EXPECT_TRUE(l->isLocked());
    EXPECT_TRUE(l2->isLocked());
    l->release();
    EXPECT_TRUE(!l->isLocked());
    EXPECT_TRUE(!l2->isLocked());
}

// Verify: NativeFSLockFactory works correctly if the lock file exists
TEST_F(LockFactoryTest, testNativeFSLockFactoryLockExists) {
    String lockFile = getTempDir(L"test.lock");
    std::ofstream lockStream;
    lockStream.open(StringUtils::toUTF8(lockFile).c_str(), std::ios::binary | std::ios::in | std::ios::out);
    lockStream.close();

    LockPtr l = newLucene<NativeFSLockFactory>(getTempDir())->makeLock(L"test.lock");
    EXPECT_TRUE(l->obtain());
    l->release();
    EXPECT_TRUE(!l->isLocked());
    if (FileUtils::fileExists(lockFile)) {
        FileUtils::removeFile(lockFile);
    }
}

TEST_F(LockFactoryTest, testNativeFSLockReleaseByOtherLock) {
    NativeFSLockFactoryPtr f = newLucene<NativeFSLockFactory>(getTempDir());

    f->setLockPrefix(L"test");
    LockPtr l = f->makeLock(L"commit");
    LockPtr l2 = f->makeLock(L"commit");

    EXPECT_TRUE(l->obtain());
    EXPECT_TRUE(l2->isLocked());

    try {
        l2->release();
    } catch (LockReleaseFailedException& e) {
        EXPECT_TRUE(check_exception(LuceneException::LockReleaseFailed)(e));
    }

    l->release();
}

// Verify: NativeFSLockFactory assigns null as lockPrefix if the lockDir is inside directory
TEST_F(LockFactoryTest, testNativeFSLockFactoryPrefix) {
    String fdir1(getTempDir(L"TestLockFactory.8"));
    String fdir2(getTempDir(L"TestLockFactory.8.Lockdir"));

    DirectoryPtr dir1(FSDirectory::open(fdir1, newLucene<NativeFSLockFactory>(fdir1)));

    // same directory, but locks are stored somewhere else. The prefix of the lock factory should != null
    DirectoryPtr dir2(FSDirectory::open(fdir1, newLucene<NativeFSLockFactory>(fdir2)));

    String prefix1(dir1->getLockFactory()->getLockPrefix());
    EXPECT_TRUE(prefix1.empty()); // Lock prefix for lockDir same as directory should be null

    String prefix2(dir2->getLockFactory()->getLockPrefix());
    EXPECT_TRUE(!prefix2.empty()); // Lock prefix for lockDir outside of directory should be not null

    FileUtils::removeDirectory(fdir1);
    FileUtils::removeDirectory(fdir2);
}

// Verify: default LockFactory has no prefix (ie write.lock is stored in index)
TEST_F(LockFactoryTest, testDefaultFSLockFactoryPrefix) {
    // Make sure we get null prefix
    String dirName(getTempDir(L"TestLockFactory.10"));
    DirectoryPtr dir(FSDirectory::open(dirName));

    String prefix(dir->getLockFactory()->getLockPrefix());

    EXPECT_TRUE(prefix.empty()); // Default lock prefix should be null

    FileUtils::removeDirectory(dirName);
}
