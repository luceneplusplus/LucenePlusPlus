/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FileReader.h"
#include "BufferedReader.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture BufferedReaderTest;

TEST_F(BufferedReaderTest, testBufferedReaderChar) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    EXPECT_EQ((wchar_t)reader->read(), L't');
    EXPECT_EQ((wchar_t)reader->read(), L'e');
    EXPECT_EQ((wchar_t)reader->read(), L's');
    EXPECT_EQ((wchar_t)reader->read(), L't');
    EXPECT_EQ((wchar_t)reader->read(), L' ');
    EXPECT_EQ((wchar_t)reader->read(), L'f');
    EXPECT_EQ((wchar_t)reader->read(), L'i');
    EXPECT_EQ((wchar_t)reader->read(), L'l');
    EXPECT_EQ((wchar_t)reader->read(), L'e');
}

TEST_F(BufferedReaderTest, testBufferedReaderRead) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));

    wchar_t buffer[80];
    int32_t length = reader->read(buffer, 0, 80);
    String bufferString(buffer, length);

    boost::replace_all(bufferString, L"\r\n", L"\n"); // account for windows newline characters

    EXPECT_EQ(bufferString, L"test file\nthat contains\nmultiple lines of text\n\n\n1 2 3 4\n");
    EXPECT_EQ(reader->read(buffer, 0, 1), FileReader::FILE_EOF);
}

TEST_F(BufferedReaderTest, testBufferedReaderReadLine) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));

    Collection<String> readLines = Collection<String>::newInstance();
    String line;

    while (reader->readLine(line)) {
        readLines.add(line);
    }

    EXPECT_EQ(reader->read(), FileReader::FILE_EOF);
    EXPECT_EQ(readLines.size(), 6);
    EXPECT_EQ(readLines[0], L"test file");
    EXPECT_EQ(readLines[1], L"that contains");
    EXPECT_EQ(readLines[2], L"multiple lines of text");
    EXPECT_EQ(readLines[3], L"");
    EXPECT_EQ(readLines[4], L"");
    EXPECT_EQ(readLines[5], L"1 2 3 4");
}

TEST_F(BufferedReaderTest, testBufferedReaderReset) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));

    wchar_t buffer[20];
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
    reader->reset();
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
}

TEST_F(BufferedReaderTest, testBufferedReaderCharsSmallBuffer) {
    static const int32_t bufferSize = 5;

    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);

    EXPECT_EQ((wchar_t)reader->read(), L't');
    EXPECT_EQ((wchar_t)reader->read(), L'e');
    EXPECT_EQ((wchar_t)reader->read(), L's');
    EXPECT_EQ((wchar_t)reader->read(), L't');
    EXPECT_EQ((wchar_t)reader->read(), L' ');
    EXPECT_EQ((wchar_t)reader->read(), L'f');
    EXPECT_EQ((wchar_t)reader->read(), L'i');
    EXPECT_EQ((wchar_t)reader->read(), L'l');
    EXPECT_EQ((wchar_t)reader->read(), L'e');
}

TEST_F(BufferedReaderTest, testBufferedReaderReadSmallBuffer) {
    static const int32_t bufferSize = 5;

    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);

    wchar_t buffer[80];
    int32_t length = reader->read(buffer, 0, 80);
    String bufferString(buffer, length);

    boost::replace_all(bufferString, L"\r\n", L"\n"); // account for windows newline characters

    EXPECT_EQ(bufferString, L"test file\nthat contains\nmultiple lines of text\n\n\n1 2 3 4\n");
    EXPECT_EQ(reader->read(buffer, 0, 1), FileReader::FILE_EOF);
}

TEST_F(BufferedReaderTest, testBufferedReaderResetSmallBuffer) {
    static const int32_t bufferSize = 5;

    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);

    wchar_t buffer[20];
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
    reader->reset();
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
}

TEST_F(BufferedReaderTest, testBufferedReaderReadLineSmallBuffer) {
    static const int32_t bufferSize = 5;

    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);

    Collection<String> readLines = Collection<String>::newInstance();
    String line;

    while (reader->readLine(line)) {
        readLines.add(line);
    }

    EXPECT_EQ(reader->read(), FileReader::FILE_EOF);
    EXPECT_EQ(readLines.size(), 6);
    EXPECT_EQ(readLines[0], L"test file");
    EXPECT_EQ(readLines[1], L"that contains");
    EXPECT_EQ(readLines[2], L"multiple lines of text");
    EXPECT_EQ(readLines[3], L"");
    EXPECT_EQ(readLines[4], L"");
    EXPECT_EQ(readLines[5], L"1 2 3 4");
}
