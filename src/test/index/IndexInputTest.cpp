/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockIndexInput.h"

using namespace Lucene;

typedef LuceneTestFixture IndexInputTest;

TEST_F(IndexInputTest, testReadInt) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 1, 2, 3, 4 };
    std::memcpy(inputBytes.get(), input, 4);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readInt(), 16909060);
}

TEST_F(IndexInputTest, testReadVInt) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[4] = { 200, 201, 150, 96 };
    std::memcpy(inputBytes.get(), input, 4);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readVInt(), 201696456);
}

TEST_F(IndexInputTest, testReadLong) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 32, 43, 32, 96, 12, 54, 22, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readLong(), 2317982030106072672LL);
}

TEST_F(IndexInputTest, testReadVLong) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[8] = { 213, 143, 132, 196, 172, 154, 129, 96 };
    std::memcpy(inputBytes.get(), input, 8);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readVLong(), 54048498881988565LL);
}

TEST_F(IndexInputTest, testReadString) {
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readString(), L"test string");
}

TEST_F(IndexInputTest, testReadModifiedUTF8String) {
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[12] = { 11, 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 12);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readModifiedUTF8String(), L"test string");
}

TEST_F(IndexInputTest, testReadChars) {
    ByteArray inputBytes(ByteArray::newInstance(30));
    uint8_t input[11] = { 't', 'e', 's', 't', ' ', 's', 't', 'r', 'i', 'n', 'g' };
    std::memcpy(inputBytes.get(), input, 11);

    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);

    ByteArray outputChars(ByteArray::newInstance(40 * sizeof(wchar_t)));
    is->readChars((wchar_t*)outputChars.get(), 0, 11);

    wchar_t expected[11] = { L't', L'e', L's', L't', L' ', L's', L't', L'r', L'i', L'n', L'g' };
    EXPECT_EQ(std::memcmp((wchar_t*)outputChars.get(), expected, 11 * sizeof(wchar_t)), 0);
}

TEST_F(IndexInputTest, testSkipOneChar) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(1);
    EXPECT_EQ(is->getFilePointer(), 1);
}

TEST_F(IndexInputTest, testSkipTwoChars) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 2, 3, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    EXPECT_EQ(is->getFilePointer(), 2);
}

TEST_F(IndexInputTest, testSkipTwoCharsAdditionalChar) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 132, 132, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    EXPECT_EQ(is->getFilePointer(), 3);
}

TEST_F(IndexInputTest, testSkipTwoCharsAdditionalTwoChars) {
    ByteArray inputBytes(ByteArray::newInstance(10));
    uint8_t input[5] = { 1, 232, 232, 4, 5 };
    std::memcpy(inputBytes.get(), input, 5);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    is->skipChars(2);
    EXPECT_EQ(is->getFilePointer(), 4);
}

TEST_F(IndexInputTest, testRead) {
    ByteArray inputBytes(ByteArray::newInstance(100));

    uint8_t input[88] = {0x80, 0x01, 0xff, 0x7f, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06,
                         'L', 'u', 'c', 'e', 'n', 'e',

                         // 2-byte UTF-8 (U+00BF "INVERTED QUESTION MARK")
                         0x02, 0xc2, 0xbf, 0x0a, 'L', 'u', 0xc2, 0xbf, 'c', 'e', 0xc2, 0xbf, 'n', 'e',

                         // 3-byte UTF-8 (U+2620 "SKULL AND CROSSBONES")
                         0x03, 0xe2, 0x98, 0xa0, 0x0c, 'L', 'u', 0xe2, 0x98, 0xa0, 'c', 'e', 0xe2, 0x98, 0xa0, 'n', 'e',

                         // surrogate pairs
                         // (U+1D11E "MUSICAL SYMBOL G CLEF")
                         // (U+1D160 "MUSICAL SYMBOL EIGHTH NOTE")
                         0x04, 0xf0, 0x9d, 0x84, 0x9e, 0x08, 0xf0, 0x9d, 0x84, 0x9e, 0xf0, 0x9d, 0x85, 0xa0, 0x0e,
                         'L', 'u', 0xf0, 0x9d, 0x84, 0x9e, 'c', 'e', 0xf0, 0x9d, 0x85, 0xa0, 'n', 'e',

                         // null bytes
                         0x01, 0x00, 0x08, 'L', 'u', 0x00, 'c', 'e', 0x00, 'n', 'e'
                        };
    std::memcpy(inputBytes.get(), input, 88);

    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);

    EXPECT_EQ(is->readVInt(), 128);
    EXPECT_EQ(is->readVInt(), 16383);
    EXPECT_EQ(is->readVInt(), 16384);
    EXPECT_EQ(is->readVInt(), 16385);
    EXPECT_EQ(is->readString(), L"Lucene");

    const uint8_t question[] = {0xc2, 0xbf};
    EXPECT_EQ(is->readString(), UTF8_TO_STRING(question));
    const uint8_t skull[] = {0x4c, 0x75, 0xc2, 0xbf, 0x63, 0x65, 0xc2, 0xbf, 0x6e, 0x65};
    EXPECT_EQ(is->readString(), UTF8_TO_STRING(skull));
    const uint8_t gclef[] = {0xe2, 0x98, 0xa0};
    EXPECT_EQ(is->readString(), UTF8_TO_STRING(gclef));
    const uint8_t eighthnote[] = {0x4c, 0x75, 0xe2, 0x98, 0xa0, 0x63, 0x65, 0xe2, 0x98, 0xa0, 0x6e, 0x65};
    EXPECT_EQ(is->readString(), UTF8_TO_STRING(eighthnote));

    String readString(is->readString());
#ifdef LPP_UNICODE_CHAR_SIZE_2
    EXPECT_EQ(readString[0], 55348);
    EXPECT_EQ(readString[1], 56606);
#else
    EXPECT_EQ(readString[0], 119070);
#endif

    readString = is->readString();
#ifdef LPP_UNICODE_CHAR_SIZE_2
    EXPECT_EQ(readString[0], 55348);
    EXPECT_EQ(readString[1], 56606);
    EXPECT_EQ(readString[2], 55348);
    EXPECT_EQ(readString[3], 56672);
#else
    EXPECT_EQ(readString[0], 119070);
    EXPECT_EQ(readString[1], 119136);
#endif

    readString = is->readString();
#ifdef LPP_UNICODE_CHAR_SIZE_2
    EXPECT_EQ(readString[0], L'L');
    EXPECT_EQ(readString[1], L'u');
    EXPECT_EQ(readString[2], 55348);
    EXPECT_EQ(readString[3], 56606);
    EXPECT_EQ(readString[4], L'c');
    EXPECT_EQ(readString[5], L'e');
    EXPECT_EQ(readString[6], 55348);
    EXPECT_EQ(readString[7], 56672);
    EXPECT_EQ(readString[8], L'n');
    EXPECT_EQ(readString[9], L'e');
#else
    EXPECT_EQ(readString[0], L'L');
    EXPECT_EQ(readString[1], L'u');
    EXPECT_EQ(readString[2], 119070);
    EXPECT_EQ(readString[3], L'c');
    EXPECT_EQ(readString[4], L'e');
    EXPECT_EQ(readString[5], 119136);
    EXPECT_EQ(readString[6], L'n');
    EXPECT_EQ(readString[7], L'e');
#endif

    readString = is->readString();
    EXPECT_EQ(readString[0], 0);

    readString = is->readString();
    EXPECT_EQ(readString[0], L'L');
    EXPECT_EQ(readString[1], L'u');
    EXPECT_EQ(readString[2], 0);
    EXPECT_EQ(readString[3], L'c');
    EXPECT_EQ(readString[4], L'e');
    EXPECT_EQ(readString[5], 0);
    EXPECT_EQ(readString[6], L'n');
    EXPECT_EQ(readString[7], L'e');
}

TEST_F(IndexInputTest, testSkipChars) {
    ByteArray inputBytes(ByteArray::newInstance(100));
    uint8_t input[17] = {0x80, 0x01, 0xff, 0x7f, 0x80, 0x80, 0x01, 0x81, 0x80, 0x01, 0x06, 'L', 'u', 'c', 'e', 'n', 'e'};
    std::memcpy(inputBytes.get(), input, 17);

    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);
    EXPECT_EQ(is->readVInt(), 128);
    EXPECT_EQ(is->readVInt(), 16383);
    EXPECT_EQ(is->readVInt(), 16384);
    EXPECT_EQ(is->readVInt(), 16385);
    EXPECT_EQ(is->readVInt(), 6);
    is->skipChars(3);
    ByteArray remainingBytes(ByteArray::newInstance(4 * sizeof(wchar_t)));
    is->readChars((wchar_t*)remainingBytes.get(), 0, 3);
    EXPECT_EQ(String((wchar_t*)remainingBytes.get(), 3), L"ene");
}

struct lessKey {
    inline bool operator()(const MapStringString::key_value& first, const MapStringString::key_value& second) const {
        return (first.first < second.first);
    }
};

TEST_F(IndexInputTest, testReadStringMap) {
    ByteArray inputBytes(ByteArray::newInstance(100));
    uint8_t input[34] = { 0, 0, 0, 3,
                          4, 'k', 'e', 'y', '1', 4, 'v', 'a', 'l', '1',
                          4, 'k', 'e', 'y', '2', 4, 'v', 'a', 'l', '2',
                          4, 'k', 'e', 'y', '3', 4, 'v', 'a', 'l', '3'
                        };
    std::memcpy(inputBytes.get(), input, 34);
    IndexInputPtr is = newLucene<MockIndexInput>(inputBytes);

    MapStringString map = is->readStringStringMap();
    EXPECT_EQ(map.size(), 3);

    Collection<MapStringString::key_value> orderedMap(Collection<MapStringString::key_value>::newInstance(map.begin(), map.end()));

    // order map by key
    std::sort(orderedMap.begin(), orderedMap.end(), lessKey());

    int32_t count = 1;
    for (Collection<MapStringString::key_value>::iterator mapEntry = orderedMap.begin(); mapEntry != orderedMap.end(); ++mapEntry, ++count) {
        EXPECT_EQ(mapEntry->first, L"key" + StringUtils::toString(count));
        EXPECT_EQ(mapEntry->second, L"val" + StringUtils::toString(count));
    }
}
