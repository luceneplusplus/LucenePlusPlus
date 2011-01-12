/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get(), testArray.size(), expectedUft8), 22);
    BOOST_CHECK_EQUAL(SingleString((char*)expectedUft8.get(), 22), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8ArrayWithOffset)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.size()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    ByteArray expectedUft8(ByteArray::newInstance(30));
    int32_t offset = 10; // "ascii string"
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get() + offset, testArray.size() - offset, expectedUft8), 12);
    BOOST_CHECK_EQUAL(SingleString((char*)expectedUft8.get(), 12), "ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8Result)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.size()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    UTF8ResultPtr utf8Result(newLucene<UTF8Result>());
    StringUtils::toUTF8(testArray.get(), testArray.size(), utf8Result);
    BOOST_CHECK_EQUAL(utf8Result->length, 22);
    BOOST_CHECK_EQUAL(SingleString((char*)utf8Result->result.get(), 22), "this is a ascii string");
}

BOOST_AUTO_TEST_CASE(testToUtf8ArrayWithTerminator)
{
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(50));
    std::copy(testString.begin(), testString.end(), testArray.get());
    testArray[testString.size()] = UTF8Base::UNICODE_TERMINATOR; // terminator
    ByteArray expectedUft8(ByteArray::newInstance(30));
    BOOST_CHECK_EQUAL(StringUtils::toUTF8(testArray.get(), testArray.size(), expectedUft8), 22);
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
    StringUtils::toUnicode(testArray.get(), testArray.size(), unicodeResult);
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

BOOST_AUTO_TEST_CASE(testUTF8Performance)
{
    uint64_t startTime = MiscUtils::currentTimeMillis();
    static const int32_t maxIter = 1000000;
    
    String unicode = L"this is a unicode string";
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUTF8(unicode);
    
    BOOST_TEST_MESSAGE("Encode utf8 (string): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
    
    const wchar_t* unicodeChar = unicode.c_str();
    int32_t unicodeLength = (int32_t)unicode.length();
    
    startTime = MiscUtils::currentTimeMillis();
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUTF8(unicodeChar, unicodeLength);
    
    BOOST_TEST_MESSAGE("Encode utf8 (pointer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
    
    ByteArray utf8 = ByteArray::newInstance(unicodeLength * StringUtils::MAX_ENCODING_UTF8_SIZE);
    
    startTime = MiscUtils::currentTimeMillis();
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUTF8(unicodeChar, unicodeLength, utf8);
    
    BOOST_TEST_MESSAGE("Encode utf8 (buffer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
}

BOOST_AUTO_TEST_CASE(testUnicodePerformance)
{
    uint64_t startTime = MiscUtils::currentTimeMillis();
    static const int32_t maxIter = 1000000;
    
    SingleString utf8 = "this is a utf8 string";
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUnicode(utf8);
    
    BOOST_TEST_MESSAGE("Decode utf8 (string): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
    
    const uint8_t* utf8Char = (const uint8_t*)utf8.c_str();
    int32_t utf8Length = (int32_t)utf8.length();
    
    startTime = MiscUtils::currentTimeMillis();
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUnicode(utf8Char, utf8Length);
    
    BOOST_TEST_MESSAGE("Decode utf8 (pointer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
    
    CharArray unicode = CharArray::newInstance(utf8Length);
    
    startTime = MiscUtils::currentTimeMillis();
    
    for (int32_t i = 0; i < maxIter; ++i)
        StringUtils::toUnicode(utf8Char, utf8Length, unicode);
    
    BOOST_TEST_MESSAGE("Decode utf8 (buffer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms");
}

BOOST_AUTO_TEST_SUITE_END()
