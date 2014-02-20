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
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture FileReaderTest;

TEST_F(FileReaderTest, testFileReaderChar) {
    FileReaderPtr reader = newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt"));
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

TEST_F(FileReaderTest, testFileReaderRead) {
    FileReaderPtr reader = newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt"));

    wchar_t buffer[80];
    int32_t length = reader->read(buffer, 0, 80);
    String bufferString(buffer, length);

    boost::replace_all(bufferString, L"\r\n", L"\n"); // account for windows newline characters

    EXPECT_EQ(bufferString, L"test file\nthat contains\nmultiple lines of text\n\n\n1 2 3 4\n");
    EXPECT_EQ(reader->read(buffer, 0, 1), FileReader::FILE_EOF);
}

TEST_F(FileReaderTest, testFileReaderReset) {
    FileReaderPtr reader = newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt"));

    wchar_t buffer[20];
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
    reader->reset();
    EXPECT_EQ(reader->read(buffer, 0, 9), 9);
    EXPECT_EQ(String(buffer, 9), L"test file");
}
