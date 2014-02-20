/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <iostream>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "ByteBlockPool.h"
#include "DocumentsWriter.h"
#include "ByteSliceWriter.h"
#include "ByteSliceReader.h"
#include "IndexWriter.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

DECLARE_SHARED_PTR(TestByteBlockAllocator)

class TestByteBlockAllocator : public ByteBlockPoolAllocatorBase {
public:
    TestByteBlockAllocator() {
        this->freeByteBlocks = Collection<ByteArray>::newInstance();
    }

    virtual ~TestByteBlockAllocator() {
    }

    LUCENE_CLASS(TestByteBlockAllocator);

public:
    Collection<ByteArray> freeByteBlocks;

public:
    virtual ByteArray getByteBlock(bool trackAllocations) {
        SyncLock syncLock(this);
        int32_t size = freeByteBlocks.size();
        ByteArray b;
        if (size == 0) {
            b = ByteArray::newInstance(DocumentsWriter::BYTE_BLOCK_SIZE);
            MiscUtils::arrayFill(b.get(), 0, b.size(), 0);
        } else {
            b = freeByteBlocks.removeLast();
        }
        return b;
    }

    virtual void recycleByteBlocks(Collection<ByteArray> blocks, int32_t start, int32_t end) {
        SyncLock syncLock(this);
        for (int32_t i = start; i < end; ++i) {
            freeByteBlocks.add(blocks[i]);
        }
    }

    virtual void recycleByteBlocks(Collection<ByteArray> blocks) {
        SyncLock syncLock(this);
        int32_t size = blocks.size();
        for (int32_t i = 0; i < size; ++i) {
            freeByteBlocks.add(blocks[i]);
        }
    }
};

typedef LuceneTestFixture ByteSlicesTest;

TEST_F(ByteSlicesTest, testBasic) {
    ByteBlockPoolPtr pool = newLucene<ByteBlockPool>(newLucene<TestByteBlockAllocator>(), false);

    int32_t NUM_STREAM = 25;

    ByteSliceWriterPtr writer = newLucene<ByteSliceWriter>(pool);

    Collection<int32_t> starts = Collection<int32_t>::newInstance(NUM_STREAM);
    Collection<int32_t> uptos = Collection<int32_t>::newInstance(NUM_STREAM);
    Collection<int32_t> counters = Collection<int32_t>::newInstance(NUM_STREAM);

    RandomPtr r = newLucene<Random>();
    ByteSliceReaderPtr reader = newLucene<ByteSliceReader>();

    for (int32_t ti = 0; ti < 100; ++ti) {
        for (int32_t stream = 0; stream < NUM_STREAM; ++stream) {
            starts[stream] = -1;
            counters[stream] = 0;
        }

        bool debug = false;

        for (int32_t iter = 0; iter < 10000; ++iter) {
            int32_t stream = r->nextInt(NUM_STREAM);
            if (debug) {
                std::wcout << L"write stream=" << stream << L"\n";
            }

            if (starts[stream] == -1) {
                int32_t spot = pool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE());
                uptos[stream] = spot + pool->byteOffset;
                starts[stream] = uptos[stream];
                if (debug) {
                    std::wcout << L"  init to " << starts[stream] << L"\n";
                }
            }

            writer->init(uptos[stream]);
            int32_t numValue = r->nextInt(20);
            for (int32_t j = 0; j < numValue; ++j) {
                if (debug) {
                    std::wcout << L"    write " << (counters[stream] + j) << L"\n";
                }
                writer->writeVInt(counters[stream] + j);
            }
            counters[stream] += numValue;
            uptos[stream] = writer->getAddress();
            if (debug) {
                std::wcout << L"    addr now " << uptos[stream] << L"\n";
            }
        }

        for (int32_t stream = 0; stream < NUM_STREAM; ++stream) {
            if (debug) {
                std::wcout << L"  stream=" << stream << L" count=" << counters[stream] << L"\n";
            }

            if (starts[stream] != uptos[stream]) {
                reader->init(pool, starts[stream], uptos[stream]);
                for (int32_t j = 0; j < counters[stream]; ++j) {
                    EXPECT_EQ(j, reader->readVInt());
                }
            }
        }

        pool->reset();
    }
}
