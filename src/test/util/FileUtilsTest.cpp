/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <fstream>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FileUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(FileUtilsTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testFileExists)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    BOOST_CHECK(FileUtils::fileExists(fileDir));
}

BOOST_AUTO_TEST_CASE(testFileModified)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    
    uint64_t fileModified = FileUtils::fileModified(fileDir);
    BOOST_CHECK_NE(fileModified, 0);
    
    struct tm *fileTime = localtime((const time_t*)&fileModified);
    BOOST_CHECK(fileTime != NULL);
}

BOOST_AUTO_TEST_CASE(testInvalidFileModified)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    BOOST_CHECK_EQUAL(FileUtils::fileModified(fileDir), 0);
}

BOOST_AUTO_TEST_CASE(testTouchFile)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    
    BOOST_CHECK(FileUtils::touchFile(fileDir));
    
    uint64_t fileModified = FileUtils::fileModified(fileDir);
    BOOST_CHECK_NE(fileModified, 0);
    
    struct tm *fileTime = localtime((const time_t*)&fileModified);
    BOOST_CHECK(fileTime != NULL);
    
    time_t current = time(NULL);
    struct tm *currentTime = localtime((const time_t*)&current);
    
    BOOST_CHECK_EQUAL(fileTime->tm_year, currentTime->tm_year);
    BOOST_CHECK_EQUAL(fileTime->tm_mon, currentTime->tm_mon);
    BOOST_CHECK_EQUAL(fileTime->tm_mday, currentTime->tm_mday);
}

BOOST_AUTO_TEST_CASE(testInvalidTouchFile)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    BOOST_CHECK(!FileUtils::touchFile(fileDir));
}

BOOST_AUTO_TEST_CASE(testFileLength)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfilesize1.txt"));
    int64_t fileLength = FileUtils::fileLength(fileDir);
    BOOST_CHECK_EQUAL(fileLength, 29);
}

BOOST_AUTO_TEST_CASE(testInvalidFileLength)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    BOOST_CHECK_EQUAL(FileUtils::fileLength(fileDir), 0);
}

BOOST_AUTO_TEST_CASE(testSetFileLength)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfilesize2.txt"));
    
    BOOST_CHECK(FileUtils::setFileLength(fileDir, 1234));
    
    int64_t fileLengthGrow = FileUtils::fileLength(fileDir);
    BOOST_CHECK_EQUAL(fileLengthGrow, 1234);
    
    BOOST_CHECK(FileUtils::setFileLength(fileDir, 29));
    
    int64_t fileLengthShrink = FileUtils::fileLength(fileDir);
    BOOST_CHECK_EQUAL(fileLengthShrink, 29);
}

BOOST_AUTO_TEST_CASE(testInvalidSetFileLength)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    BOOST_CHECK(!FileUtils::setFileLength(fileDir, 1234));
}

BOOST_AUTO_TEST_CASE(testRemoveFile)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testdelete.txt"));
    
    std::ofstream f(StringUtils::toUTF8(fileDir).c_str(), std::ios::binary | std::ios::out);
    f.close();
    
    BOOST_CHECK(FileUtils::fileExists(fileDir));
    BOOST_CHECK(FileUtils::removeFile(fileDir));
    BOOST_CHECK(!FileUtils::fileExists(fileDir));
}

BOOST_AUTO_TEST_CASE(testInvalidRemoveFile)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    BOOST_CHECK(!FileUtils::removeFile(fileDir));
}

BOOST_AUTO_TEST_CASE(testIsDirectory)
{
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));
    BOOST_CHECK(FileUtils::isDirectory(fileDir));
}

BOOST_AUTO_TEST_CASE(testNotDirectory)
{
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    BOOST_CHECK(!FileUtils::isDirectory(fileDir));
}

BOOST_AUTO_TEST_CASE(testNotDirectoryEmpty)
{
    BOOST_CHECK(!FileUtils::isDirectory(L""));
}

BOOST_AUTO_TEST_CASE(testListDirectory)
{
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));
    
    HashSet<String> list(HashSet<String>::newInstance());
    BOOST_CHECK(FileUtils::listDirectory(fileDir, false, list));
    
    Collection<String> expectedList(Collection<String>::newInstance(list.begin(), list.end()));
    std::sort(expectedList.begin(), expectedList.end());
    
    BOOST_CHECK_EQUAL(expectedList.size(), 6);
    BOOST_CHECK_EQUAL(expectedList[0], L"subdirectory");
    BOOST_CHECK_EQUAL(expectedList[1], L"testfile1.txt");
    BOOST_CHECK_EQUAL(expectedList[2], L"testfile2.txt");
    BOOST_CHECK_EQUAL(expectedList[3], L"testfile3.txt");
    BOOST_CHECK_EQUAL(expectedList[4], L"testfilesize1.txt");
    BOOST_CHECK_EQUAL(expectedList[5], L"testfilesize2.txt");
}

BOOST_AUTO_TEST_CASE(testListDirectoryFiles)
{
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));
    
    HashSet<String> list(HashSet<String>::newInstance());
    BOOST_CHECK(FileUtils::listDirectory(fileDir, true, list));
    
    Collection<String> expectedList(Collection<String>::newInstance(list.begin(), list.end()));
    std::sort(expectedList.begin(), expectedList.end());
    
    BOOST_CHECK_EQUAL(expectedList.size(), 5);
    BOOST_CHECK_EQUAL(expectedList[0], L"testfile1.txt");
    BOOST_CHECK_EQUAL(expectedList[1], L"testfile2.txt");
    BOOST_CHECK_EQUAL(expectedList[2], L"testfile3.txt");
    BOOST_CHECK_EQUAL(expectedList[3], L"testfilesize1.txt");
    BOOST_CHECK_EQUAL(expectedList[4], L"testfilesize2.txt");
}

BOOST_AUTO_TEST_CASE(testJoinPath)
{
    #if defined(_WIN32) || defined(_WIN64)
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"c:\\test", L"\\testfile.txt"), L"c:\\test\\testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"c:\\test", L"testfile.txt"), L"c:\\test\\testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"c:\\test", L""), L"c:\\test");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"\\test", L"\\testfile.txt"), L"\\test\\testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"\\test", L"testfile.txt"), L"\\test\\testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"\\test", L""), L"\\test");
    #else
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"/test", L"/testfile.txt"), L"/test/testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"/test", L"testfile.txt"), L"/test/testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::joinPath(L"/test", L""), L"/test");
    #endif
}

BOOST_AUTO_TEST_CASE(testExtractPath)
{
    #if defined(_WIN32) || defined(_WIN64)
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"c:\\test"), L"c:\\");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"c:\\test\\testfile.txt"), L"c:\\test");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L""), L"");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"\\test"), L"\\");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"\\test\\testfile.txt"), L"\\test");
    #else
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"/test"), L"/");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L"/test/testfile.txt"), L"/test");
    BOOST_CHECK_EQUAL(FileUtils::extractPath(L""), L"");
    #endif
}

BOOST_AUTO_TEST_CASE(testExtractFile)
{
    #if defined(_WIN32) || defined(_WIN64)
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"c:\\test"), L"test");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"c:\\test\\testfile.txt"), L"testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L""), L"");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"\\test"), L"test");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"\\test\\testfile.txt"), L"testfile.txt");
    #else
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"/test"), L"test");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L"/test/testfile.txt"), L"testfile.txt");
    BOOST_CHECK_EQUAL(FileUtils::extractFile(L""), L"");
    #endif
}

BOOST_AUTO_TEST_SUITE_END()
