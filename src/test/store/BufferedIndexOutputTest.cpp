/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "BufferedIndexOutput.h"
#include "BufferedIndexInput.h"

using namespace Lucene;

typedef LuceneTestFixture BufferedIndexOutputTest;

class TestableBufferedIndexOutput : public BufferedIndexOutput {
public:
    using BufferedIndexOutput::writeBytes;

    TestableBufferedIndexOutput(uint8_t* b, int32_t length) : outputBytes(b), outputLength(length), nextByte(0) {
    }

    virtual void flushBuffer(const uint8_t* b, int32_t offset, int32_t length) {
        std::copy(b + offset, b + offset + length, outputBytes + nextByte + offset);
        nextByte += length;
    }

    virtual int64_t length() {
        return outputLength;
    }

protected:
    uint8_t* outputBytes;
    int32_t outputLength;
    int32_t nextByte;
};

TEST_F(BufferedIndexOutputTest, testWriteInt) {
    ByteArray outputBytes(ByteArray::newInstance(10));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 10);
    indexOutput.writeInt(1234);
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 4);
    uint8_t expected[4] = { 0, 0, 4, 210 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 4), 0);
}

TEST_F(BufferedIndexOutputTest, testWriteVInt) {
    ByteArray outputBytes(ByteArray::newInstance(10));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 10);
    indexOutput.writeVInt(1234);
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 2);
    uint8_t expected[2] = { 210, 9 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 2), 0);
}

TEST_F(BufferedIndexOutputTest, testWriteLong) {
    ByteArray outputBytes(ByteArray::newInstance(10));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 10);
    indexOutput.writeLong(1234123412341234LL);
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 8);
    uint8_t expected[8] = { 0, 4, 98, 109, 191, 154, 1, 242 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 8), 0);
}

TEST_F(BufferedIndexOutputTest, testWriteVLong) {
    ByteArray outputBytes(ByteArray::newInstance(10));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 10);
    indexOutput.writeVLong(1234123412341234LL);
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 8);
    uint8_t expected[8] = { 242, 131, 232, 252, 219, 205, 152, 2 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 8), 0);
}

TEST_F(BufferedIndexOutputTest, testWriteString) {
    ByteArray outputBytes(ByteArray::newInstance(30));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 30);
    indexOutput.writeString(L"test string");
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 12);
    uint8_t expected[12] = { 11, 116, 101, 115, 116, 32, 115, 116, 114, 105, 110, 103 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 12), 0);
}

TEST_F(BufferedIndexOutputTest, testWriteChars) {
    ByteArray outputBytes(ByteArray::newInstance(30));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 30);
    indexOutput.writeChars(L"test string", 5, 6);
    indexOutput.flush();
    EXPECT_EQ(indexOutput.getFilePointer(), 6);
    uint8_t expected[6] = { 115, 116, 114, 105, 110, 103 };
    EXPECT_EQ(memcmp(outputBytes.get(), expected, 6), 0);
}

namespace TestCopyBytes {

class SourceIndexInput : public BufferedIndexInput {
public:
    SourceIndexInput(const uint8_t* b, int32_t length) : inputBytes(b), inputLength(length), nextByte(0) {
    }

    virtual void readInternal(uint8_t* b, int32_t offset, int32_t length) {
        std::copy(inputBytes + nextByte + offset, inputBytes + nextByte + offset + length, b + offset);
        nextByte += length;
    }

    virtual void seekInternal(int64_t pos) {
    }

    virtual int64_t length() {
        return inputLength;
    }

    virtual IndexInputPtr clone() {
        return IndexInputPtr();
    }

protected:
    const uint8_t* inputBytes;
    int32_t inputLength;
    int32_t nextByte;
};

}

TEST_F(BufferedIndexOutputTest, testCopyBytes) {
    ByteArray sourceBytes(ByteArray::newInstance(32768));
    std::generate(sourceBytes.get(), sourceBytes.get() + 32768, rand);
    BufferedIndexInputPtr indexSource(newLucene<TestCopyBytes::SourceIndexInput>(sourceBytes.get(), 32768));

    ByteArray outputBytes(ByteArray::newInstance(32768));
    TestableBufferedIndexOutput indexOutput(outputBytes.get(), 32768);

    indexOutput.copyBytes(indexSource, 20000);
    indexOutput.flush();

    EXPECT_EQ(memcmp(outputBytes.get(), sourceBytes.get(), 20000), 0);
}
