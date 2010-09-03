/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockIndexInput.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(IndexInputTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testReadInt)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 1, 2, 3, 4 };
    std::memcpy(inputBytes.get(), input, 4);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readInt(), 16909060);
}

BOOST_AUTO_TEST_CASE(testReadVInt)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 200, 201, 150, 96 };
    std::memcpy(inputBytes.get(), input, 4);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readVInt(), 201696456);
}

BOOST_AUTO_TEST_CASE(testReadLong)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 32, 43, 32, 96, 12, 54, 22, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readLong(), 2317982030106072672LL);
}

BOOST_AUTO_TEST_CASE(testReadVLong)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 213, 143, 132, 196, 172, 154, 129, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readVLong(), -926873643);
}

BOOST_AUTO_TEST_CASE(testReadString)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readString(), L"test string");
}

BOOST_AUTO_TEST_CASE(testReadModifiedUTF8String)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readModifiedUTF8String(), L"test string");
}

BOOST_AUTO_TEST_CASE(testReadChars)
{
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[11] = { 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 11);
    
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    
    ByteArray outputChars(ByteArray::newInstance(40 * sizeof(wchar_t)));
    is->readChars((wchar_t*)outputChars.get(), 0, 11);
    
    wchar_t expected[11] = { L't', L'e', L's', L't', L' ', L's', L't', L'r', L'i', L'n', L'g' };
    BOOST_CHECK_EQUAL(std::memcmp((wchar_t*)outputChars.get(), expected, 11 * sizeof(wchar_t)), 0);
}

BOOST_AUTO_TEST_CASE(testSkipOneChar)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(1);
    BOOST_CHECK_EQUAL(is->getFilePointer(), 1);
}

BOOST_AUTO_TEST_CASE(testSkipTwoChars)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    BOOST_CHECK_EQUAL(is->getFilePointer(), 2);
}

BOOST_AUTO_TEST_CASE(testSkipTwoCharsAdditionalChar)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 132, 132, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    BOOST_CHECK_EQUAL(is->getFilePointer(), 3);
}

BOOST_AUTO_TEST_CASE(testSkipTwoCharsAdditionalTwoChars)
{
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 232, 232, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    BOOST_CHECK_EQUAL(is->getFilePointer(), 4);
}

BOOST_AUTO_TEST_CASE(testRead)
{
    ByteArray inputBytes(ByteArray::newInstance(100));
    
    uint8_t input[88] = { 0x80, 0x01, 0xFF, 0x7F, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06,
                          'L', 'u', 'c', 'e', 'n', 'e',
                          
                          // 2-byte UTF-8 (U+00BF "INVERTED QUESTION MARK") 
                          0x02, 0xC2, 0xBF, 0x0A, 'L', 'u', 0xC2, 0xBF, 'c', 'e', 0xC2, 0xBF, 'n', 'e',
                          
                          // 3-byte UTF-8 (U+2620 "SKULL AND CROSSBONES") 
                          0x03, 0xE2, 0x98, 0xA0, 0x0C, 'L', 'u', 0xE2, 0x98, 0xA0, 'c', 'e', 0xE2, 0x98, 0xA0, 'n', 'e',
                          
                          // surrogate pairs
                          // (U+1D11E "MUSICAL SYMBOL G CLEF")
                          // (U+1D160 "MUSICAL SYMBOL EIGHTH NOTE")
                          0x04, 0xF0, 0x9D, 0x84, 0x9E, 0x08, 0xF0, 0x9D, 0x84, 0x9E, 0xF0, 0x9D, 0x85, 0xA0, 0x0E,
                          'L', 'u', 0xF0, 0x9D, 0x84, 0x9E, 'c', 'e', 0xF0, 0x9D, 0x85, 0xA0, 'n', 'e',
                          
                          // null bytes
                          0x01, 0x00, 0x08, 'L', 'u', 0x00, 'c', 'e', 0x00, 'n', 'e' };
    std::memcpy(inputBytes.get(), input, 88);
    
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    
    BOOST_CHECK_EQUAL(is->readVInt(), 128);
    BOOST_CHECK_EQUAL(is->readVInt(), 16383);
    BOOST_CHECK_EQUAL(is->readVInt(), 16384);
    BOOST_CHECK_EQUAL(is->readVInt(), 16385);
    BOOST_CHECK_EQUAL(is->readString(), L"Lucene");
    BOOST_CHECK_EQUAL(is->readString(), L"\u00BF");
    BOOST_CHECK_EQUAL(is->readString(), L"Lu\u00BFce\u00BFne");
    BOOST_CHECK_EQUAL(is->readString(), L"\u2620");
    BOOST_CHECK_EQUAL(is->readString(), L"Lu\u2620ce\u2620ne");
    
    String readString(is->readString());
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], 55348);
    BOOST_CHECK_EQUAL(readString[1], 56606);
    #else
    BOOST_CHECK_EQUAL(readString[0], 119070);
    #endif
    
    readString = is->readString();	
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], 55348);
    BOOST_CHECK_EQUAL(readString[1], 56606);
    BOOST_CHECK_EQUAL(readString[2], 55348);
    BOOST_CHECK_EQUAL(readString[3], 56672);
    #else
    BOOST_CHECK_EQUAL(readString[0], 119070);
    BOOST_CHECK_EQUAL(readString[1], 119136);
    #endif
    
    readString = is->readString();
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 55348);
    BOOST_CHECK_EQUAL(readString[3], 56606);
    BOOST_CHECK_EQUAL(readString[4], L'c');
    BOOST_CHECK_EQUAL(readString[5], L'e');
    BOOST_CHECK_EQUAL(readString[6], 55348);
    BOOST_CHECK_EQUAL(readString[7], 56672);
    BOOST_CHECK_EQUAL(readString[8], L'n');
    BOOST_CHECK_EQUAL(readString[9], L'e');
    #else
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 119070);
    BOOST_CHECK_EQUAL(readString[3], L'c');
    BOOST_CHECK_EQUAL(readString[4], L'e');
    BOOST_CHECK_EQUAL(readString[5], 119136);
    BOOST_CHECK_EQUAL(readString[6], L'n');
    BOOST_CHECK_EQUAL(readString[7], L'e');
    #endif
    
    readString = is->readString();
    BOOST_CHECK_EQUAL(readString[0], 0);
    
    readString = is->readString();
    BOOST_CHECK_EQUAL(readString[0], L'L');
    BOOST_CHECK_EQUAL(readString[1], L'u');
    BOOST_CHECK_EQUAL(readString[2], 0);
    BOOST_CHECK_EQUAL(readString[3], L'c');
    BOOST_CHECK_EQUAL(readString[4], L'e');
    BOOST_CHECK_EQUAL(readString[5], 0);
    BOOST_CHECK_EQUAL(readString[6], L'n');
    BOOST_CHECK_EQUAL(readString[7], L'e');
}

BOOST_AUTO_TEST_CASE(testSkipChars)
{
    ByteArray inputBytes(ByteArray::newInstance(100));
    uint8_t input[17] = { 0x80, 0x01, 0xFF, 0x7F, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06,
                          'L', 'u', 'c', 'e', 'n', 'e' };
    std::memcpy(inputBytes.get(), input, 17);
    
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    BOOST_CHECK_EQUAL(is->readVInt(), 128);
    BOOST_CHECK_EQUAL(is->readVInt(), 16383);
    BOOST_CHECK_EQUAL(is->readVInt(), 16384);
    BOOST_CHECK_EQUAL(is->readVInt(), 16385);
    BOOST_CHECK_EQUAL(is->readVInt(), 6);
    is->skipChars(3);
    ByteArray remainingBytes(ByteArray::newInstance(4 * sizeof(wchar_t)));
    is->readChars((wchar_t*)remainingBytes.get(), 0, 3);
    BOOST_CHECK_EQUAL(String((wchar_t*)remainingBytes.get(), 3), L"ene");
}

struct lessKey
{
    inline bool operator()(const MapStringString::key_value& first, const MapStringString::key_value& second) const
    {
        return (first.first < second.first);
    }
};

BOOST_AUTO_TEST_CASE(testReadStringMap)
{
    ByteArray inputBytes(ByteArray::newInstance(100));
    uint8_t input[34] = { 0, 0, 0, 3, 
                          4, 'k', 'e', 'y', '1', 4, 'v', 'a', 'l', '1',
                          4, 'k', 'e', 'y', '2', 4, 'v', 'a', 'l', '2',
                          4, 'k', 'e', 'y', '3', 4, 'v', 'a', 'l', '3' };
    std::memcpy(inputBytes.get(), input, 34);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    
    MapStringString map = is->readStringStringMap();
    BOOST_CHECK_EQUAL(map.size(), 3);
    
    Collection<MapStringString::key_value> orderedMap(Collection<MapStringString::key_value>::newInstance(map.begin(), map.end()));
    
    // order map by key
    std::sort(orderedMap.begin(), orderedMap.end(), lessKey());
    
    int32_t count = 1;
    for (Collection<MapStringString::key_value>::iterator mapEntry = orderedMap.begin(); mapEntry != orderedMap.end(); ++mapEntry, ++count)
    {
        BOOST_CHECK_EQUAL(mapEntry->first, L"key" + StringUtils::toString(count));
        BOOST_CHECK_EQUAL(mapEntry->second, L"val" + StringUtils::toString(count));
    }
}

BOOST_AUTO_TEST_SUITE_END()
