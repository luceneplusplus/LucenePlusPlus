/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(DirectoryTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testDetectDirectoryClose)
{
    RAMDirectoryPtr dir(newLucene<RAMDirectory>());
    dir->close();
    BOOST_CHECK_EXCEPTION(dir->createOutput(L"test"), LuceneException, check_exception(LuceneException::AlreadyClosed));
}

BOOST_AUTO_TEST_CASE(testDetectFSDirectoryClose)
{
    DirectoryPtr dir = FSDirectory::open(getTempDir());
    dir->close();
    BOOST_CHECK_EXCEPTION(dir->createOutput(L"test"), LuceneException, check_exception(LuceneException::AlreadyClosed));
}

template < class FSDirectory1, class FSDirectory2 >
void TestInstantiationPair(FSDirectory1& first, FSDirectory2& second, const String& fileName, const String& lockName)
{
    BOOST_CHECK_NO_THROW(first.ensureOpen());
    
    IndexOutputPtr out = first.createOutput(fileName);
    BOOST_CHECK_NO_THROW(out->writeByte(123));
    BOOST_CHECK_NO_THROW(out->close());
    
    BOOST_CHECK_NO_THROW(first.ensureOpen());
    BOOST_CHECK(first.fileExists(fileName));
    BOOST_CHECK_EQUAL(first.fileLength(fileName), 1);
    
    // don't test read on MMapDirectory, since it can't really be closed and will cause a failure to delete the file.
    if (!first.isMMapDirectory())
    {
        IndexInputPtr input = first.openInput(fileName);
        BOOST_CHECK_EQUAL(input->readByte(), 123);
        BOOST_CHECK_NO_THROW(input->close());
    }
    
    BOOST_CHECK_NO_THROW(second.ensureOpen());
    BOOST_CHECK(second.fileExists(fileName));
    BOOST_CHECK_EQUAL(second.fileLength(fileName), 1);
    
    if (!second.isMMapDirectory())
    {
        IndexInputPtr input = second.openInput(fileName);
        BOOST_CHECK_EQUAL(input->readByte(), 123);
        BOOST_CHECK_NO_THROW(input->close());
    }
    
    // delete with a different dir
    second.deleteFile(fileName);
    
    BOOST_CHECK(!first.fileExists(fileName));
    BOOST_CHECK(!second.fileExists(fileName));
    
    LockPtr lock = first.makeLock(lockName);
    BOOST_CHECK(lock->obtain());
    
    LockPtr lock2 = first.makeLock(lockName);
    BOOST_CHECK_EXCEPTION(lock2->obtain(1), LuceneException, check_exception(LuceneException::LockObtainFailed));
    
    lock->release();
    
    lock = second.makeLock(lockName);
    BOOST_CHECK(lock->obtain());
    lock->release();
}

namespace TestDirectInstantiation
{
    class TestableSimpleFSDirectory : public SimpleFSDirectory
    {
    public:
        TestableSimpleFSDirectory(const String& path) : SimpleFSDirectory(path) {}
        virtual ~TestableSimpleFSDirectory() {}
        using SimpleFSDirectory::ensureOpen;
        bool isMMapDirectory() { return false; }
    };

    class TestableMMapDirectory : public MMapDirectory
    {
    public:
        TestableMMapDirectory(const String& path) : MMapDirectory(path) {}
        virtual ~TestableMMapDirectory() {}
        using MMapDirectory::ensureOpen;
        bool isMMapDirectory() { return true; }
    };
}

// Test that different instances of FSDirectory can coexist on the same
// path, can read, write, and lock files.
BOOST_AUTO_TEST_CASE(testDirectInstantiation)
{
    TestDirectInstantiation::TestableSimpleFSDirectory fsDir(getTempDir());
    fsDir.ensureOpen();

    TestDirectInstantiation::TestableMMapDirectory mmapDir(getTempDir());
    mmapDir.ensureOpen();

    TestInstantiationPair(fsDir, mmapDir, L"foo.0", L"foo0.lck");
    TestInstantiationPair(mmapDir, fsDir, L"foo.1", L"foo1.lck");
}

BOOST_AUTO_TEST_CASE(testDontCreate)
{
    String path(FileUtils::joinPath(getTempDir(), L"doesnotexist"));
    try
    {
        BOOST_CHECK(!FileUtils::fileExists(path));
        SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
        BOOST_CHECK(!FileUtils::fileExists(path));
    }
    catch (...)
    {
    }
    FileUtils::removeDirectory(path);
}

void checkDirectoryFilter(DirectoryPtr dir)
{
    String name(L"file");
    dir->createOutput(name)->close();
    BOOST_CHECK(dir->fileExists(name));
    HashSet<String> dirFiles(dir->listAll());
    BOOST_CHECK(dirFiles.contains(name));
}

BOOST_AUTO_TEST_CASE(testRAMDirectoryFilter)
{
    checkDirectoryFilter(newLucene<RAMDirectory>());
}

BOOST_AUTO_TEST_CASE(testFSDirectoryFilter)
{
    checkDirectoryFilter(newLucene<SimpleFSDirectory>(getTempDir()));
}

BOOST_AUTO_TEST_CASE(testCopySubdir)
{
    String path(FileUtils::joinPath(getTempDir(), L"testsubdir"));
    try
    {
        FileUtils::createDirectory(path);
        String subpath(FileUtils::joinPath(path, L"subdir"));
        FileUtils::createDirectory(subpath);
        SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
        BOOST_CHECK(newLucene<RAMDirectory>(fsDir)->listAll().empty());
    }
    catch (...)
    {
    }
    FileUtils::removeDirectory(path);
}

BOOST_AUTO_TEST_CASE(testNotDirectory)
{
    String path(FileUtils::joinPath(getTempDir(), L"testnotdir"));
    SimpleFSDirectoryPtr fsDir(newLucene<SimpleFSDirectory>(path));
    try
    {
        IndexOutputPtr out = fsDir->createOutput(L"afile");
        out->close();
        BOOST_CHECK(fsDir->fileExists(L"afile"));
        BOOST_CHECK_EXCEPTION(newLucene<SimpleFSDirectory>(FileUtils::joinPath(path, L"afile")), LuceneException, check_exception(LuceneException::NoSuchDirectory));
    }
    catch (...)
    {
    }
    FileUtils::removeDirectory(path);
}

BOOST_AUTO_TEST_SUITE_END()
