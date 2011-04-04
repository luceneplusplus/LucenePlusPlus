/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(BufferedReaderTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testBufferedReaderChar)
{
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'e');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L's');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L' ');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'f');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'i');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'l');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'e');
}

BOOST_AUTO_TEST_CASE(testBufferedReaderRead)
{
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    
    wchar_t buffer[80];
    int32_t length = reader->read(buffer, 0, 80);
    String bufferString(buffer, length);
    
    boost::replace_all(bufferString, L"\r\n", L"\n"); // account for windows newline characters
    
    BOOST_CHECK_EQUAL(bufferString, L"test file\nthat contains\nmultiple lines of text\n\n\n1 2 3 4\n");
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 1), FileReader::FILE_EOF);
}

BOOST_AUTO_TEST_CASE(testBufferedReaderReadLine)
{
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    
    Collection<String> readLines = Collection<String>::newInstance();
    String line;
    
    while (reader->readLine(line))
        readLines.add(line);
    
    BOOST_CHECK_EQUAL(reader->read(), FileReader::FILE_EOF);
    BOOST_CHECK_EQUAL(readLines.size(), 6);
    BOOST_CHECK_EQUAL(readLines[0], L"test file");
    BOOST_CHECK_EQUAL(readLines[1], L"that contains");
    BOOST_CHECK_EQUAL(readLines[2], L"multiple lines of text");
    BOOST_CHECK_EQUAL(readLines[3], L"");
    BOOST_CHECK_EQUAL(readLines[4], L"");
    BOOST_CHECK_EQUAL(readLines[5], L"1 2 3 4");
}

BOOST_AUTO_TEST_CASE(testBufferedReaderReset)
{
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")));
    
    wchar_t buffer[20];
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 9), 9);
    BOOST_CHECK_EQUAL(String(buffer, 9), L"test file");
    reader->reset();
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 9), 9);
    BOOST_CHECK_EQUAL(String(buffer, 9), L"test file");
}

BOOST_AUTO_TEST_CASE(testBufferedReaderCharsSmallBuffer)
{
    static const int32_t bufferSize = 5;
    
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);
    
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'e');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L's');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L't');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L' ');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'f');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'i');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'l');
    BOOST_CHECK_EQUAL((wchar_t)reader->read(), L'e');
}

BOOST_AUTO_TEST_CASE(testBufferedReaderReadSmallBuffer)
{
    static const int32_t bufferSize = 5;
    
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);
    
    wchar_t buffer[80];
    int32_t length = reader->read(buffer, 0, 80);
    String bufferString(buffer, length);
    
    boost::replace_all(bufferString, L"\r\n", L"\n"); // account for windows newline characters
    
    BOOST_CHECK_EQUAL(bufferString, L"test file\nthat contains\nmultiple lines of text\n\n\n1 2 3 4\n");
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 1), FileReader::FILE_EOF);
}

BOOST_AUTO_TEST_CASE(testBufferedReaderResetSmallBuffer)
{
    static const int32_t bufferSize = 5;
    
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);
    
    wchar_t buffer[20];
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 9), 9);
    BOOST_CHECK_EQUAL(String(buffer, 9), L"test file");
    reader->reset();
    BOOST_CHECK_EQUAL(reader->read(buffer, 0, 9), 9);
    BOOST_CHECK_EQUAL(String(buffer, 9), L"test file");
}

BOOST_AUTO_TEST_CASE(testBufferedReaderReadLineSmallBuffer)
{
    static const int32_t bufferSize = 5;
    
    BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"testfile_text.txt")), bufferSize);
    
    Collection<String> readLines = Collection<String>::newInstance();
    String line;
    
    while (reader->readLine(line))
        readLines.add(line);
    
    BOOST_CHECK_EQUAL(reader->read(), FileReader::FILE_EOF);
    BOOST_CHECK_EQUAL(readLines.size(), 6);
    BOOST_CHECK_EQUAL(readLines[0], L"test file");
    BOOST_CHECK_EQUAL(readLines[1], L"that contains");
    BOOST_CHECK_EQUAL(readLines[2], L"multiple lines of text");
    BOOST_CHECK_EQUAL(readLines[3], L"");
    BOOST_CHECK_EQUAL(readLines[4], L"");
    BOOST_CHECK_EQUAL(readLines[5], L"1 2 3 4");
}

BOOST_AUTO_TEST_SUITE_END()
