/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "OpenBitSet.h"
#include "OpenBitSetIterator.h"
#include "BitSet.h"
#include "BitUtil.h"
#include "Random.h"

using namespace Lucene;

typedef LuceneTestFixture OpenBitSetTest;

static RandomPtr randBitSet = newLucene<Random>(123);

static void doGet(const BitSetPtr& a, const OpenBitSetPtr& b) {
    int32_t max = a->size();
    for (int32_t i = 0; i < max; ++i) {
        EXPECT_EQ(a->get(i), b->get(i));
    }
}

static void doNextSetBit(const BitSetPtr& a, const OpenBitSetPtr& b) {
    int32_t aa = -1;
    int32_t bb = -1;
    do {
        aa = a->nextSetBit(aa + 1);
        bb = b->nextSetBit(bb + 1);
        EXPECT_EQ(aa, bb);
    } while (aa >= 0);
}

static void doIterate1(const BitSetPtr& a, const OpenBitSetPtr& b) {
    int32_t aa = -1;
    int32_t bb = -1;
    OpenBitSetIteratorPtr iterator = newLucene<OpenBitSetIterator>(b);
    do {
        aa = a->nextSetBit(aa + 1);
        bb = (randBitSet->nextInt() % 2 == 0) ? iterator->nextDoc() : iterator->advance(bb + 1);
        EXPECT_EQ(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
    } while (aa >= 0);
}

static void doIterate2(const BitSetPtr& a, const OpenBitSetPtr& b) {
    int32_t aa = -1;
    int32_t bb = -1;
    OpenBitSetIteratorPtr iterator = newLucene<OpenBitSetIterator>(b);
    do {
        aa = a->nextSetBit(aa + 1);
        bb = (randBitSet->nextInt() % 2 == 0) ? iterator->nextDoc() : iterator->advance(bb + 1);
        EXPECT_EQ(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
    } while (aa >= 0);
}

static void doIterate(const BitSetPtr& a, const OpenBitSetPtr& b, int32_t mode) {
    if (mode == 1) {
        doIterate1(a, b);
    } else if (mode == 2) {
        doIterate2(a, b);
    }
}

static void doRandomSets(int32_t maxSize, int32_t iter, int32_t mode) {
    BitSetPtr a0;
    OpenBitSetPtr b0;

    for (int32_t i = 0; i < iter; ++i) {
        int32_t sz = randBitSet->nextInt(maxSize);
        BitSetPtr a = newLucene<BitSet>(sz);
        OpenBitSetPtr b = newLucene<OpenBitSet>(sz);

        // test the various ways of setting bits
        if (sz > 0) {
            int32_t nOper = randBitSet->nextInt(sz);
            for (int32_t j = 0; j < nOper; ++j) {
                int32_t idx = randBitSet->nextInt(sz);
                a->set(idx);
                b->fastSet(idx);
                idx = randBitSet->nextInt(sz);
                a->clear(idx);
                b->fastClear(idx);
                idx = randBitSet->nextInt(sz);
                a->flip(idx);
                b->fastFlip(idx);

                bool val = b->flipAndGet(idx);
                bool val2 = b->flipAndGet(idx);
                EXPECT_NE(val, val2);

                val = b->getAndSet(idx);
                EXPECT_EQ(val2, val);
                EXPECT_TRUE(b->get(idx));

                if (!val) {
                    b->fastClear(idx);
                }
                EXPECT_EQ(b->get(idx), val);
            }
        }

        // test that the various ways of accessing the bits are equivalent
        doGet(a, b);

        // test ranges, including possible extension
        int32_t fromIndex = randBitSet->nextInt(sz + 80);
        int32_t toIndex = fromIndex + randBitSet->nextInt((sz >> 1) + 1);
        BitSetPtr aa = boost::dynamic_pointer_cast<BitSet>(a->clone());
        aa->flip(fromIndex, toIndex);
        OpenBitSetPtr bb = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
        bb->flip(fromIndex, toIndex);

        doIterate(aa, bb, mode); // a problem here is from flip or doIterate

        fromIndex = randBitSet->nextInt(sz + 80);
        toIndex = fromIndex + randBitSet->nextInt((sz >> 1) + 1);
        aa = boost::dynamic_pointer_cast<BitSet>(a->clone());
        aa->clear(fromIndex, toIndex);
        bb = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
        bb->clear(fromIndex, toIndex);

        doNextSetBit(aa, bb); // a problem here is from clear() or nextSetBit

        fromIndex = randBitSet->nextInt(sz + 80);
        toIndex = fromIndex + randBitSet->nextInt((sz >> 1) + 1);
        aa = boost::dynamic_pointer_cast<BitSet>(a->clone());
        aa->set((uint32_t)fromIndex, (uint32_t)toIndex);
        bb = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
        bb->set(fromIndex, toIndex);

        doNextSetBit(aa, bb); // a problem here is from set() or nextSetBit

        if (a0) {
            EXPECT_EQ(a->equals(a0), b->equals(b0));
            EXPECT_EQ(a->cardinality(), b->cardinality());

            BitSetPtr a_and = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_and->_and(a0);
            BitSetPtr a_or = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_or->_or(a0);
            BitSetPtr a_xor = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_xor->_xor(a0);
            BitSetPtr a_andn = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_andn->andNot(a0);

            OpenBitSetPtr b_and = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            EXPECT_TRUE(b->equals(b_and));
            b_and->_and(b0);
            OpenBitSetPtr b_or = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_or->_or(b0);
            OpenBitSetPtr b_xor = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_xor->_xor(b0);
            OpenBitSetPtr b_andn = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_andn->andNot(b0);

            doIterate(a_and, b_and, mode);
            doIterate(a_or, b_or, mode);
            doIterate(a_xor, b_xor, mode);
            doIterate(a_andn, b_andn, mode);

            EXPECT_EQ(a_and->cardinality(), b_and->cardinality());
            EXPECT_EQ(a_or->cardinality(), b_or->cardinality());
            EXPECT_EQ(a_xor->cardinality(), b_xor->cardinality());
            EXPECT_EQ(a_andn->cardinality(), b_andn->cardinality());

            // test non-mutating popcounts
            EXPECT_EQ(b_and->cardinality(), OpenBitSet::intersectionCount(b, b0));
            EXPECT_EQ(b_or->cardinality(), OpenBitSet::unionCount(b, b0));
            EXPECT_EQ(b_xor->cardinality(), OpenBitSet::xorCount(b, b0));
            EXPECT_EQ(b_andn->cardinality(), OpenBitSet::andNotCount(b, b0));
        }

        a0=a;
        b0=b;
    }
}

TEST_F(OpenBitSetTest, testSmall) {
    randBitSet->setSeed(17);
    doRandomSets(1200, 1000, 1);
    doRandomSets(1200, 1000, 2);
}

/*
TEST_F(OpenBitSetTest, testBig)
{
    randBitSet->setSeed(17);
    doRandomSets(2000, 200000, 1);
    doRandomSets(2000, 200000, 2);
}
*/

TEST_F(OpenBitSetTest, testEquals) {
    randBitSet->setSeed(17);
    OpenBitSetPtr b1 = newLucene<OpenBitSet>(1111);
    OpenBitSetPtr b2 = newLucene<OpenBitSet>(2222);
    EXPECT_TRUE(b1->equals(b2));
    EXPECT_TRUE(b2->equals(b1));
    b1->set(10);
    EXPECT_TRUE(!b1->equals(b2));
    EXPECT_TRUE(!b2->equals(b1));
    b2->set(10);
    EXPECT_TRUE(b1->equals(b2));
    EXPECT_TRUE(b2->equals(b1));
    b2->set(2221);
    EXPECT_TRUE(!b1->equals(b2));
    EXPECT_TRUE(!b2->equals(b1));
    b1->set(2221);
    EXPECT_TRUE(b1->equals(b2));
    EXPECT_TRUE(b2->equals(b1));
}

TEST_F(OpenBitSetTest, testBitUtils) {
    randBitSet->setSeed(17);
    int64_t num = 100000;
    EXPECT_EQ(5, BitUtil::ntz(num));
    EXPECT_EQ(5, BitUtil::ntz2(num));
    EXPECT_EQ(5, BitUtil::ntz3(num));

    num = 10;
    EXPECT_EQ(1, BitUtil::ntz(num));
    EXPECT_EQ(1, BitUtil::ntz2(num));
    EXPECT_EQ(1, BitUtil::ntz3(num));

    for (int32_t i = 0; i < 64; ++i) {
        num = (int64_t)1 << i;
        EXPECT_EQ(i, BitUtil::ntz(num));
        EXPECT_EQ(i, BitUtil::ntz2(num));
        EXPECT_EQ(i, BitUtil::ntz3(num));
    }
}

TEST_F(OpenBitSetTest, testHashCodeEquals) {
    OpenBitSetPtr bs1 = newLucene<OpenBitSet>(200);
    OpenBitSetPtr bs2 = newLucene<OpenBitSet>(64);
    bs1->set(3);
    bs2->set(3);
    EXPECT_TRUE(bs1->equals(bs2));
    EXPECT_EQ(bs1->hashCode(), bs2->hashCode());
}
