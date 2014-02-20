/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "UTF8Stream.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

using namespace Lucene;

typedef LuceneTestFixture StringUtilsTest;

TEST_F(StringUtilsTest, testToUtf8) {
    String testString(L"this is a ascii string");
    EXPECT_EQ(StringUtils::toUTF8(testString), "this is a ascii string");
}

TEST_F(StringUtilsTest, testToUtf8CharArray) {
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    ByteArray expectedUft8(ByteArray::newInstance(30));
    EXPECT_EQ(StringUtils::toUTF8(testArray.get(), testArray.size(), expectedUft8), 22);
    EXPECT_EQ(SingleString((char*)expectedUft8.get(), 22), "this is a ascii string");
}

TEST_F(StringUtilsTest, testToUtf8ArrayWithOffset) {
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.size()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    ByteArray expectedUft8(ByteArray::newInstance(30));
    int32_t offset = 10; // "ascii string"
    EXPECT_EQ(StringUtils::toUTF8(testArray.get() + offset, testArray.size() - offset, expectedUft8), 12);
    EXPECT_EQ(SingleString((char*)expectedUft8.get(), 12), "ascii string");
}

TEST_F(StringUtilsTest, testToUtf8Result) {
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(testString.size()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    UTF8ResultPtr utf8Result(newLucene<UTF8Result>());
    StringUtils::toUTF8(testArray.get(), testArray.size(), utf8Result);
    EXPECT_EQ(utf8Result->length, 22);
    EXPECT_EQ(SingleString((char*)utf8Result->result.get(), 22), "this is a ascii string");
}

TEST_F(StringUtilsTest, testToUtf8ArrayWithTerminator) {
    String testString(L"this is a ascii string");
    CharArray testArray(CharArray::newInstance(50));
    std::copy(testString.begin(), testString.end(), testArray.get());
    testArray[testString.size()] = UTF8Base::UNICODE_TERMINATOR; // terminator
    ByteArray expectedUft8(ByteArray::newInstance(30));
    EXPECT_EQ(StringUtils::toUTF8(testArray.get(), testArray.size(), expectedUft8), 22);
    EXPECT_EQ(SingleString((char*)expectedUft8.get(), 22), "this is a ascii string");
}

TEST_F(StringUtilsTest, testToUnicode) {
    SingleString testString("this is a unicode string");
    EXPECT_EQ(StringUtils::toUnicode(testString), L"this is a unicode string");
}

TEST_F(StringUtilsTest, testToUnicodeResult) {
    SingleString testString("this is a unicode string");
    ByteArray testArray(ByteArray::newInstance(testString.length()));
    std::copy(testString.begin(), testString.end(), testArray.get());
    UnicodeResultPtr unicodeResult(newLucene<UnicodeResult>());
    StringUtils::toUnicode(testArray.get(), testArray.size(), unicodeResult);
    EXPECT_EQ(unicodeResult->length, 24);
    EXPECT_EQ(String(unicodeResult->result.get(), 24), L"this is a unicode string");
}


TEST_F(StringUtilsTest, testToStringInteger) {
    EXPECT_EQ(StringUtils::toString((int32_t)1234), L"1234");
}

TEST_F(StringUtilsTest, testToStringLong) {
    EXPECT_EQ(StringUtils::toString((int64_t)1234), L"1234");
}

TEST_F(StringUtilsTest, testToStringBase) {
    EXPECT_EQ(StringUtils::toString(1234, 4), L"103102");
    EXPECT_EQ(StringUtils::toString(1234, 10), L"1234");
    EXPECT_EQ(StringUtils::toString(1234, 16), L"4d2");
    EXPECT_EQ(StringUtils::toString(1234, StringUtils::CHARACTER_MAX_RADIX), L"ya");
}

TEST_F(StringUtilsTest, testToLongBase) {
    EXPECT_EQ(StringUtils::toLong(L"1234", 4), 112);
    EXPECT_EQ(StringUtils::toLong(L"1234", 10), 1234);
    EXPECT_EQ(StringUtils::toLong(L"1234", 16), 4660);
    EXPECT_EQ(StringUtils::toLong(L"1234", StringUtils::CHARACTER_MAX_RADIX), 49360);
}

TEST_F(StringUtilsTest, testToStringLongBase) {
    EXPECT_EQ(StringUtils::toString(1234, 4), L"103102");
    EXPECT_EQ(StringUtils::toLong(L"103102", 4), 1234);

    EXPECT_EQ(StringUtils::toString(1234, 10), L"1234");
    EXPECT_EQ(StringUtils::toLong(L"1234", 10), 1234);

    EXPECT_EQ(StringUtils::toString(1234, 16), L"4d2");
    EXPECT_EQ(StringUtils::toLong(L"4d2", 16), 1234);

    EXPECT_EQ(StringUtils::toString(1234, StringUtils::CHARACTER_MAX_RADIX), L"ya");
    EXPECT_EQ(StringUtils::toLong(L"ya", StringUtils::CHARACTER_MAX_RADIX), 1234);
}

TEST_F(StringUtilsTest, testToHash) {
    EXPECT_EQ(StringUtils::hashCode(L"test"), 3556498);
    EXPECT_EQ(StringUtils::hashCode(L"string"), -891985903);
}

TEST_F(StringUtilsTest, testUTF8Performance) {
    uint64_t startTime = MiscUtils::currentTimeMillis();
    static const int32_t maxIter = 1000000;

    String unicode = L"this is a unicode string";

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUTF8(unicode);
    }

    // std::cout << "Encode utf8 (string): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";

    const wchar_t* unicodeChar = unicode.c_str();
    int32_t unicodeLength = (int32_t)unicode.length();

    startTime = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUTF8(unicodeChar, unicodeLength);
    }

    // std::cout << "Encode utf8 (pointer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";

    ByteArray utf8 = ByteArray::newInstance(unicodeLength * StringUtils::MAX_ENCODING_UTF8_SIZE);

    startTime = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUTF8(unicodeChar, unicodeLength, utf8);
    }

    // std::cout << "Encode utf8 (buffer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";
}

TEST_F(StringUtilsTest, testUnicodePerformance) {
    uint64_t startTime = MiscUtils::currentTimeMillis();
    static const int32_t maxIter = 1000000;

    SingleString utf8 = "this is a utf8 string";

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUnicode(utf8);
    }

    // std::cout << "Decode utf8 (string): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";

    const uint8_t* utf8Char = (const uint8_t*)utf8.c_str();
    int32_t utf8Length = (int32_t)utf8.length();

    startTime = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUnicode(utf8Char, utf8Length);
    }

    // std::cout << "Decode utf8 (pointer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";

    CharArray unicode = CharArray::newInstance(utf8Length);

    startTime = MiscUtils::currentTimeMillis();

    for (int32_t i = 0; i < maxIter; ++i) {
        StringUtils::toUnicode(utf8Char, utf8Length, unicode);
    }

    // std::cout << "Decode utf8 (buffer): " << (MiscUtils::currentTimeMillis() - startTime) << "ms";
}
