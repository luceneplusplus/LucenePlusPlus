/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "StringReader.h"

using namespace Lucene;

typedef LuceneTestFixture StringReaderTest;

TEST_F(StringReaderTest, testStringReaderChar) {
    StringReader reader(L"Test string");
    EXPECT_EQ((wchar_t)reader.read(), L'T');
    EXPECT_EQ((wchar_t)reader.read(), L'e');
    EXPECT_EQ((wchar_t)reader.read(), L's');
    EXPECT_EQ((wchar_t)reader.read(), L't');
    EXPECT_EQ((wchar_t)reader.read(), L' ');
    EXPECT_EQ((wchar_t)reader.read(), L's');
    EXPECT_EQ((wchar_t)reader.read(), L't');
    EXPECT_EQ((wchar_t)reader.read(), L'r');
    EXPECT_EQ((wchar_t)reader.read(), L'i');
    EXPECT_EQ((wchar_t)reader.read(), L'n');
    EXPECT_EQ((wchar_t)reader.read(), L'g');
    EXPECT_EQ(reader.read(), StringReader::READER_EOF);
}

TEST_F(StringReaderTest, testStringReaderBuffer) {
    StringReader reader(L"Longer test string");

    wchar_t buffer[50];

    EXPECT_EQ(reader.read(buffer, 0, 6), 6);
    EXPECT_EQ(String(buffer, 6), L"Longer");
    EXPECT_EQ(reader.read(buffer, 0, 1), 1);
    EXPECT_EQ(String(buffer, 1), L" ");
    EXPECT_EQ(reader.read(buffer, 0, 4), 4);
    EXPECT_EQ(String(buffer, 4), L"test");
    EXPECT_EQ(reader.read(buffer, 0, 1), 1);
    EXPECT_EQ(String(buffer, 1), L" ");
    EXPECT_EQ(reader.read(buffer, 0, 6), 6);
    EXPECT_EQ(String(buffer, 6), L"string");
    EXPECT_EQ(reader.read(buffer, 0, 1), StringReader::READER_EOF);
}

TEST_F(StringReaderTest, testStringReaderReset) {
    StringReader reader(L"Longer test string");

    wchar_t buffer[50];

    EXPECT_EQ(reader.read(buffer, 0, 6), 6);
    EXPECT_EQ(String(buffer, 6), L"Longer");
    reader.reset();
    EXPECT_EQ(reader.read(buffer, 0, 6), 6);
    EXPECT_EQ(String(buffer, 6), L"Longer");
}

TEST_F(StringReaderTest, testStringReaderPastEOF) {
    StringReader reader(L"Short string");

    wchar_t buffer[50];

    EXPECT_EQ(reader.read(buffer, 0, 20), 12);
    EXPECT_EQ(reader.read(buffer, 0, 1), StringReader::READER_EOF);
}
