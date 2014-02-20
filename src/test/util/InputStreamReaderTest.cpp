/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FileReader.h"
#include "InputStreamReader.h"
#include "BufferedReader.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture InputStreamReaderTest;

TEST_F(InputStreamReaderTest, testInputStreamReaderChar) {
    InputStreamReaderPtr stream = newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    EXPECT_EQ((wchar_t)stream->read(), L't');
    EXPECT_EQ((wchar_t)stream->read(), L'e');
    EXPECT_EQ((wchar_t)stream->read(), L's');
    EXPECT_EQ((wchar_t)stream->read(), L't');
    EXPECT_EQ((wchar_t)stream->read(), L' ');
    EXPECT_EQ((wchar_t)stream->read(), L'f');
    EXPECT_EQ((wchar_t)stream->read(), L'i');
    EXPECT_EQ((wchar_t)stream->read(), L'l');
    EXPECT_EQ((wchar_t)stream->read(), L'e');
}

TEST_F(InputStreamReaderTest, testInputStreamReaderCharUtf8) {
    InputStreamReaderPtr stream = newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_uft8.txt")));

    const uint8_t chinese[] = {0xe4, 0xb8, 0xad, 0xe5, 0x8d, 0x8e, 0xe4, 0xba, 0xba, 0xe6, 0xb0,
                               0x91, 0xe5, 0x85, 0xb1, 0xe5, 0x92, 0x8c, 0xe5, 0x9b, 0xbd
                              };
    String expectedChinese(UTF8_TO_STRING(chinese));

    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[0]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[1]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[2]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[3]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[4]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[5]);
    EXPECT_EQ((wchar_t)stream->read(), expectedChinese[6]);
}

TEST_F(InputStreamReaderTest, testInputStreamReaderReadLine) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt"))));

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

TEST_F(InputStreamReaderTest, testInputStreamReaderReadLineUtf8) {
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_uft8.txt"))));

    Collection<String> readLines = Collection<String>::newInstance();
    String line;

    while (reader->readLine(line)) {
        readLines.add(line);
    }

    const uint8_t chinese[] = {0xe4, 0xb8, 0xad, 0xe5, 0x8d, 0x8e, 0xe4, 0xba, 0xba, 0xe6, 0xb0,
                               0x91, 0xe5, 0x85, 0xb1, 0xe5, 0x92, 0x8c, 0xe5, 0x9b, 0xbd
                              };

    const uint8_t persian[] = {0xd9, 0x86, 0xd8, 0xaf, 0xd8, 0xa7, 0xd8, 0xb4, 0xd8, 0xaa, 0xd9, 0x87};

    const uint8_t russian[] = {0xd0, 0xb0, 0xd0, 0xb1, 0xd0, 0xb8, 0xd1, 0x81, 0xd1, 0x81, 0xd0,
                               0xb8, 0xd0, 0xbd, 0xd0, 0xb8, 0xd1, 0x8e
                              };

    EXPECT_EQ(readLines.size(), 80);
    EXPECT_EQ(readLines[0], UTF8_TO_STRING(chinese));
    EXPECT_EQ(readLines[1], UTF8_TO_STRING(persian));
    EXPECT_EQ(readLines[2], UTF8_TO_STRING(russian));
}
