/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "UnicodeUtils.h"
#include "UTF8Stream.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(StringUtilsTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testToUtf8)
{
    String testString(L"this is a ascii string");
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testString), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8CharArray)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    ByteArray expectedUft8(ByteArray::newInstance(30));
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get(), testArray.length(), expectedUft8), 22);
    BOOST_CHECK_EQUAL(SingleString((char*)expectedUft8.get(), 22), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8ArrayWithOffset)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    ByteArray expectedUft8(ByteArray::newInstance(30));
    int32_t offset = 10; // "ascii string"
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get() + offset, testArray.length() - offset, expectedUft8), 12);
    BOOST_CHECK_EQUAL(SingleString((char*)expectedUft8.get(), 12), "ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8Result)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    UTF8ResultPtr utf8Result(newLucene<UTF8Result>());
    StringUtils::toUTF8(testArray.get(), testArray.length(), utf8Result);
    BOOST_CHECK_EQUAL(utf8Result->length, 22);
    BOOST_CHECK_EQUAL(SingleString((char*)utf8Result->result.get(), 22), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8ArrayWithTerminator)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(50));
    std::copy(testString.begin(), testString.end(), testArray.get());
    testArray[testString.length()] = UTF8Stream::UNICODE_TERMINATOR; // terminator
    ByteArray expectedUft8(ByteArray::newInstance(30));
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get(), testArray.length(), expectedUft8), 22);
    BOOST_CHECK_EQUAL(SingleString((char*)expectedUft8.get(), 22), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUnicode)
{
    SingleString testString("this is a unicode string");
    BOOST_CHECK_EQUAL(StringUtils::toUnicode(testString), L"this is a unicode string");
}

BOOST_AUTO_TEST_CASE(testToUnicodeResult)
{
    SingleString testString("this is a unicode string");
    ByteArray testArray(ByteArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    UnicodeResultPtr unicodeResult(newLucene<UnicodeResult>());
    StringUtils::toUnicode(testArray.get(), testArray.length(), unicodeResult);
    BOOST_CHECK_EQUAL(unicodeResult->length, 24);
    BOOST_CHECK_EQUAL(String(unicodeResult->result.get(), 24), L"this is a unicode string");
}


BOOST_AUTO_TEST_CASE(testToStringInteger)
{
    BOOST_CHECK_EQUAL(StringUtils::toString((int32_t)1234), L"1234");
}

BOOST_AUTO_TEST_CASE(testToStringLong)
{
    BOOST_CHECK_EQUAL(StringUtils::toString((int64_t)1234), L"1234");
}

BOOST_AUTO_TEST_CASE(testToStringBase)
{
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 4), L"103102");
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 10), L"1234");
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 16), L"4d2");
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, StringUtils::CHARACTER_MAX_RADIX), L"ya");
}

BOOST_AUTO_TEST_CASE(testToLongBase)
{
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"1234", 4), 112);
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"1234", 10), 1234);
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"1234", 16), 4660);
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"1234", StringUtils::CHARACTER_MAX_RADIX), 49360);
}

BOOST_AUTO_TEST_CASE(testToStringLongBase)
{
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 4), L"103102");
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"103102", 4), 1234);
    
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 10), L"1234");
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"1234", 10), 1234);
    
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, 16), L"4d2");
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"4d2", 16), 1234);
    
    BOOST_CHECK_EQUAL(StringUtils::toString(1234, StringUtils::CHARACTER_MAX_RADIX), L"ya");
    BOOST_CHECK_EQUAL(StringUtils::toLong(L"ya", StringUtils::CHARACTER_MAX_RADIX), 1234);
}

BOOST_AUTO_TEST_CASE(testToHash)
{
    BOOST_CHECK_EQUAL(StringUtils::hashCode(L"test"), 3556498);
    BOOST_CHECK_EQUAL(StringUtils::hashCode(L"string"), -891985903);
}

BOOST_AUTO_TEST_SUITE_END()
