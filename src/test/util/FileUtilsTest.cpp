/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <fstream>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture FileUtilsTest;

TEST_F(FileUtilsTest, testFileExists) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    EXPECT_TRUE(FileUtils::fileExists(fileDir));
}

TEST_F(FileUtilsTest, testFileModified) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));

    uint64_t fileModified = FileUtils::fileModified(fileDir);
    EXPECT_NE(fileModified, 0);

    struct tm* fileTime = localtime((const time_t*)&fileModified);
    EXPECT_TRUE(fileTime != NULL);
}

TEST_F(FileUtilsTest, testInvalidFileModified) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    EXPECT_EQ(FileUtils::fileModified(fileDir), 0);
}

TEST_F(FileUtilsTest, testTouchFile) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));

    EXPECT_TRUE(FileUtils::touchFile(fileDir));

    uint64_t fileModified = FileUtils::fileModified(fileDir);
    EXPECT_NE(fileModified, 0);

    struct tm* fileTime = localtime((const time_t*)&fileModified);
    EXPECT_TRUE(fileTime != NULL);

    time_t current = time(NULL);
    struct tm* currentTime = localtime((const time_t*)&current);

    EXPECT_EQ(fileTime->tm_year, currentTime->tm_year);
    EXPECT_EQ(fileTime->tm_mon, currentTime->tm_mon);
    EXPECT_EQ(fileTime->tm_mday, currentTime->tm_mday);
}

TEST_F(FileUtilsTest, testInvalidTouchFile) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    EXPECT_TRUE(!FileUtils::touchFile(fileDir));
}

TEST_F(FileUtilsTest, testFileLength) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfilesize1.txt"));
    int64_t fileLength = FileUtils::fileLength(fileDir);
    EXPECT_EQ(fileLength, 29);
}

TEST_F(FileUtilsTest, testInvalidFileLength) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    EXPECT_EQ(FileUtils::fileLength(fileDir), 0);
}

TEST_F(FileUtilsTest, testSetFileLength) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfilesize2.txt"));

    EXPECT_TRUE(FileUtils::setFileLength(fileDir, 1234));

    int64_t fileLengthGrow = FileUtils::fileLength(fileDir);
    EXPECT_EQ(fileLengthGrow, 1234);

    EXPECT_TRUE(FileUtils::setFileLength(fileDir, 29));

    int64_t fileLengthShrink = FileUtils::fileLength(fileDir);
    EXPECT_EQ(fileLengthShrink, 29);
}

TEST_F(FileUtilsTest, testInvalidSetFileLength) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    EXPECT_TRUE(!FileUtils::setFileLength(fileDir, 1234));
}

TEST_F(FileUtilsTest, testRemoveFile) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testdelete.txt"));

    std::ofstream f(StringUtils::toUTF8(fileDir).c_str(), std::ios::binary | std::ios::out);
    f.close();

    EXPECT_TRUE(FileUtils::fileExists(fileDir));
    EXPECT_TRUE(FileUtils::removeFile(fileDir));
    EXPECT_TRUE(!FileUtils::fileExists(fileDir));
}

TEST_F(FileUtilsTest, testInvalidRemoveFile) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"invalid"));
    EXPECT_TRUE(!FileUtils::removeFile(fileDir));
}

TEST_F(FileUtilsTest, testIsDirectory) {
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));
    EXPECT_TRUE(FileUtils::isDirectory(fileDir));
}

TEST_F(FileUtilsTest, testNotDirectory) {
    String fileDir(FileUtils::joinPath(FileUtils::joinPath(getTestDir(), L"testdirectory"), L"testfile1.txt"));
    EXPECT_TRUE(!FileUtils::isDirectory(fileDir));
}

TEST_F(FileUtilsTest, testNotDirectoryEmpty) {
    EXPECT_TRUE(!FileUtils::isDirectory(L""));
}

TEST_F(FileUtilsTest, testListDirectory) {
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));

    HashSet<String> list(HashSet<String>::newInstance());
    EXPECT_TRUE(FileUtils::listDirectory(fileDir, false, list));

    Collection<String> expectedList(Collection<String>::newInstance(list.begin(), list.end()));
    std::sort(expectedList.begin(), expectedList.end());

    EXPECT_EQ(expectedList.size(), 6);
    EXPECT_EQ(expectedList[0], L"subdirectory");
    EXPECT_EQ(expectedList[1], L"testfile1.txt");
    EXPECT_EQ(expectedList[2], L"testfile2.txt");
    EXPECT_EQ(expectedList[3], L"testfile3.txt");
    EXPECT_EQ(expectedList[4], L"testfilesize1.txt");
    EXPECT_EQ(expectedList[5], L"testfilesize2.txt");
}

TEST_F(FileUtilsTest, testListDirectoryFiles) {
    String fileDir(FileUtils::joinPath(getTestDir(), L"testdirectory"));

    HashSet<String> list(HashSet<String>::newInstance());
    EXPECT_TRUE(FileUtils::listDirectory(fileDir, true, list));

    Collection<String> expectedList(Collection<String>::newInstance(list.begin(), list.end()));
    std::sort(expectedList.begin(), expectedList.end());

    EXPECT_EQ(expectedList.size(), 5);
    EXPECT_EQ(expectedList[0], L"testfile1.txt");
    EXPECT_EQ(expectedList[1], L"testfile2.txt");
    EXPECT_EQ(expectedList[2], L"testfile3.txt");
    EXPECT_EQ(expectedList[3], L"testfilesize1.txt");
    EXPECT_EQ(expectedList[4], L"testfilesize2.txt");
}

TEST_F(FileUtilsTest, testJoinPath) {
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(FileUtils::joinPath(L"c:\\test", L"\\testfile.txt"), L"c:\\test\\testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"c:\\test", L"testfile.txt"), L"c:\\test\\testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"c:\\test", L""), L"c:\\test");
    EXPECT_EQ(FileUtils::joinPath(L"\\test", L"\\testfile.txt"), L"\\test\\testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"\\test", L"testfile.txt"), L"\\test\\testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"\\test", L""), L"\\test");
#else
    EXPECT_EQ(FileUtils::joinPath(L"/test", L"/testfile.txt"), L"/test/testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"/test", L"testfile.txt"), L"/test/testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"", L"testfile.txt"), L"testfile.txt");
    EXPECT_EQ(FileUtils::joinPath(L"/test", L""), L"/test");
#endif
}

TEST_F(FileUtilsTest, testExtractPath) {
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(FileUtils::extractPath(L"c:\\test"), L"c:\\");
    EXPECT_EQ(FileUtils::extractPath(L"c:\\test\\testfile.txt"), L"c:\\test");
    EXPECT_EQ(FileUtils::extractPath(L""), L"");
    EXPECT_EQ(FileUtils::extractPath(L"\\test"), L"\\");
    EXPECT_EQ(FileUtils::extractPath(L"\\test\\testfile.txt"), L"\\test");
#else
    EXPECT_EQ(FileUtils::extractPath(L"/test"), L"/");
    EXPECT_EQ(FileUtils::extractPath(L"/test/testfile.txt"), L"/test");
    EXPECT_EQ(FileUtils::extractPath(L""), L"");
#endif
}

TEST_F(FileUtilsTest, testExtractFile) {
#if defined(_WIN32) || defined(_WIN64)
    EXPECT_EQ(FileUtils::extractFile(L"c:\\test"), L"test");
    EXPECT_EQ(FileUtils::extractFile(L"c:\\test\\testfile.txt"), L"testfile.txt");
    EXPECT_EQ(FileUtils::extractFile(L""), L"");
    EXPECT_EQ(FileUtils::extractFile(L"\\test"), L"test");
    EXPECT_EQ(FileUtils::extractFile(L"\\test\\testfile.txt"), L"testfile.txt");
#else
    EXPECT_EQ(FileUtils::extractFile(L"/test"), L"test");
    EXPECT_EQ(FileUtils::extractFile(L"/test/testfile.txt"), L"testfile.txt");
    EXPECT_EQ(FileUtils::extractFile(L""), L"");
#endif
}
