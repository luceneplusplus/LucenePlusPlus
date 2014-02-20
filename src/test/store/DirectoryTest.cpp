/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Directory.h"
#include "Lock.h"
#include "FSDirectory.h"
#include "SimpleFSDirectory.h"
#include "MMapDirectory.h"
#include "RAMDirectory.h"
#include "IndexInput.h"
#include "IndexOutput.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture DirectoryTest;

TEST_F(DirectoryTest, testDetectDirectoryClose) {
    RAMDirectoryPtr dir(newLucene<RAMDirectory>());
    dir->close();
    try {
        dir->createOutput(L"test");
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::AlreadyClosed)(e));
    }
}

TEST_F(DirectoryTest, testDetectFSDirectoryClose) {
    DirectoryPtr dir = FSDirectory::open(getTempDir());
    dir->close();
    try {
        dir->createOutput(L"test");
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::AlreadyClosed)(e));
    }
}

template < class FSDirectory1, class FSDirectory2 >
void TestInstantiationPair(FSDirectory1& first, FSDirectory2& second, const String& fileName, const String& lockName) {
    EXPECT_NO_THROW(first.ensureOpen());

    IndexOutputPtr out = first.createOutput(fileName);
    EXPECT_NO_THROW(out->writeByte(123));
    EXPECT_NO_THROW(out->close());

    EXPECT_NO_THROW(first.ensureOpen());
    EXPECT_TRUE(first.fileExists(fileName));
    EXPECT_EQ(first.fileLength(fileName), 1);

    // don't test read on MMapDirectory, since it can't really be closed and will cause a failure to delete the file.
    if (!first.isMMapDirectory()) {
        IndexInputPtr input = first.openInput(fileName);
        EXPECT_EQ(input->readByte(), 123);
        EXPECT_NO_THROW(input->close());
    }

    EXPECT_NO_THROW(second.ensureOpen());
    EXPECT_TRUE(second.fileExists(fileName));
    EXPECT_EQ(second.fileLength(fileName), 1);

    if (!second.isMMapDirectory()) {
        IndexInputPtr input = second.openInput(fileName);
        EXPECT_EQ(input->readByte(), 123);
        EXPECT_NO_THROW(input->close());
    }

    // delete with a different dir
    second.deleteFile(fileName);

    EXPECT_TRUE(!first.fileExists(fileName));
    EXPECT_TRUE(!second.fileExists(fileName));

    LockPtr lock = first.makeLock(lockName);
    EXPECT_TRUE(lock->obtain());

    LockPtr lock2 = first.makeLock(lockName);
    try {
        lock2->obtain(1);
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::LockObtainFailed)(e));
    }

    lock->release();

    lock = second.makeLock(lockName);
    EXPECT_TRUE(lock->obtain());
    lock->release();
}

namespace TestDirectInstantiation {

class TestableSimpleFSDirectory : public SimpleFSDirectory {
public:
    TestableSimpleFSDirectory(const String& path) : SimpleFSDirectory(path) {}
    virtual ~TestableSimpleFSDirectory() {}
    using SimpleFSDirectory::ensureOpen;
    bool isMMapDirectory() {
        return false;
    }
};

class TestableMMapDirectory : public MMapDirectory {
public:
    TestableMMapDirectory(const String& path) : MMapDirectory(path) {}
    virtual ~TestableMMapDirectory() {}
    using MMapDirectory::ensureOpen;
    bool isMMapDirectory() {
        return true;
    }
};

}

// Test that different instances of FSDirectory can coexist on the same
// path, can read, write, and lock files.
TEST_F(DirectoryTest, testDirectInstantiation) {
    TestDirectInstantiation::TestableSimpleFSDirectory fsDir(getTempDir());
    fsDir.ensureOpen();

    TestDirectInstantiation::TestableMMapDirectory mmapDir(getTempDir());
    mmapDir.ensureOpen();

    TestInstantiationPair(fsDir, mmapDir, L"foo.0", L"foo0.lck");
    TestInstantiationPair(mmapDir, fsDir, L"foo.1", L"foo1.lck");
}

TEST_F(DirectoryTest, testDontCreate) {
    String path(FileUtils::joinPath(getTempDir(), L"doesnotexist"));
    try {
        EXPECT_TRUE(!FileUtils::fileExists(path));
        SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
        EXPECT_TRUE(!FileUtils::fileExists(path));
    } catch (...) {
    }
    FileUtils::removeDirectory(path);
}

void checkDirectoryFilter(const DirectoryPtr& dir) {
    String name(L"file");
    dir->createOutput(name)->close();
    EXPECT_TRUE(dir->fileExists(name));
    HashSet<String> dirFiles(dir->listAll());
    EXPECT_TRUE(dirFiles.contains(name));
}

TEST_F(DirectoryTest, testRAMDirectoryFilter) {
    checkDirectoryFilter(newLucene<RAMDirectory>());
}

TEST_F(DirectoryTest, testFSDirectoryFilter) {
    checkDirectoryFilter(newLucene<SimpleFSDirectory>(getTempDir()));
}

TEST_F(DirectoryTest, testCopySubdir) {
    String path(FileUtils::joinPath(getTempDir(), L"testsubdir"));
    try {
        FileUtils::createDirectory(path);
        String subpath(FileUtils::joinPath(path, L"subdir"));
        FileUtils::createDirectory(subpath);
        SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
        EXPECT_TRUE(newLucene<RAMDirectory>(fsDir)->listAll().empty());
    } catch (...) {
    }
    FileUtils::removeDirectory(path);
}

TEST_F(DirectoryTest, testNotDirectory) {
    String path(FileUtils::joinPath(getTempDir(), L"testnotdir"));
    SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
    try {
        IndexOutputPtr out = fsDir->createOutput(L"afile");
        out->close();
        EXPECT_TRUE(fsDir->fileExists(L"afile"));
        try {
            newLucene<SimpleFSDirectory>(FileUtils::joinPath(path, L"afile"));
        } catch (LuceneException& e) {
            EXPECT_TRUE(check_exception(LuceneException::NoSuchDirectory)(e));
        }
    } catch (...) {
    }
    FileUtils::removeDirectory(path);
}
