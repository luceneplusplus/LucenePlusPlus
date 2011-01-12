/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "StringReader.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(StringReaderTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testStringReaderChar)
{
    StringReader reader(L"Test string");
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'T');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'e');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L's');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L' ');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L's');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'r');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'i');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'n');
    BOOST_CHECK_EQUAL((wchar_t)reader.read(), L'g');
    BOOST_CHECK_EQUAL(reader.read(), StringReader::READER_EOF);
}

BOOST_AUTO_TEST_CASE(testStringReaderBuffer)
{
    StringReader reader(L"Longer test string");
    
    wchar_t buffer[50];
    
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 6), 6);
    BOOST_CHECK_EQUAL(String(buffer, 6), L"Longer");
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 1), 1);
    BOOST_CHECK_EQUAL(String(buffer, 1), L" ");
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 4), 4);
    BOOST_CHECK_EQUAL(String(buffer, 4), L"test");
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 1), 1);
    BOOST_CHECK_EQUAL(String(buffer, 1), L" ");
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 6), 6);
    BOOST_CHECK_EQUAL(String(buffer, 6), L"string");
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 1), StringReader::READER_EOF);
}

BOOST_AUTO_TEST_CASE(testStringReaderReset)
{
    StringReader reader(L"Longer test string");
    
    wchar_t buffer[50];
    
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 6), 6);
    BOOST_CHECK_EQUAL(String(buffer, 6), L"Longer");
    reader.reset();
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 6), 6);
    BOOST_CHECK_EQUAL(String(buffer, 6), L"Longer");
}

BOOST_AUTO_TEST_CASE(testStringReaderPastEOF)
{
    StringReader reader(L"Short string");
    
    wchar_t buffer[50];
    
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 20), 12);
    BOOST_CHECK_EQUAL(reader.read(buffer, 0, 1), StringReader::READER_EOF);
}

BOOST_AUTO_TEST_SUITE_END()
