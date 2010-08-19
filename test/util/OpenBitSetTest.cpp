/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "OpenBitSet.h"
#include "OpenBitSetIterator.h"
#include "BitSet.h"
#include "BitUtil.h"
#include "Random.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(OpenBitSetTest, LuceneTestFixture)

static RandomPtr randBitSet = newLucene<Random>();

static void doGet(BitSetPtr a, OpenBitSetPtr b)
{
    int32_t max = a->size();
    for (int32_t i = 0; i < max; ++i)
        BOOST_CHECK_EQUAL(a->get(i), b->get(i));
}

static void doNextSetBit(BitSetPtr a, OpenBitSetPtr b)
{
    int32_t aa = -1;
    int32_t bb = -1;
    do
    {
        aa = a->nextSetBit(aa + 1);
        bb = b->nextSetBit(bb + 1);
        BOOST_CHECK_EQUAL(aa, bb);
    }
    while (aa >= 0);
}

static void doIterate1(BitSetPtr a, OpenBitSetPtr b)
{
    int32_t aa = -1;
    int32_t bb = -1;
    OpenBitSetIteratorPtr iterator = newLucene<OpenBitSetIterator>(b);
    do
    {
        aa = a->nextSetBit(aa + 1);
        bb = (randBitSet->nextInt() % 2 == 0) ? iterator->nextDoc() : iterator->advance(bb + 1);
        BOOST_CHECK_EQUAL(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
    }
    while (aa >= 0);
}

static void doIterate2(BitSetPtr a, OpenBitSetPtr b)
{
    int32_t aa = -1;
    int32_t bb = -1;
    OpenBitSetIteratorPtr iterator = newLucene<OpenBitSetIterator>(b);
    do
    {
        aa = a->nextSetBit(aa + 1);
        bb = (randBitSet->nextInt() % 2 == 0) ? iterator->nextDoc() : iterator->advance(bb + 1);
        BOOST_CHECK_EQUAL(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
    }
    while (aa >= 0);
}

static void doIterate(BitSetPtr a, OpenBitSetPtr b, int32_t mode)
{
    if (mode == 1)
        doIterate1(a, b);
    else if (mode == 2)
        doIterate2(a, b);
}

static void doRandomSets(int32_t maxSize, int32_t iter, int32_t mode)
{
    BitSetPtr a0;
    OpenBitSetPtr b0;
    
    for (int32_t i = 0; i < iter; ++i)
    {
        int32_t sz = randBitSet->nextInt(maxSize);
        BitSetPtr a = newLucene<BitSet>(sz);
        OpenBitSetPtr b = newLucene<OpenBitSet>(sz);
        
        // test the various ways of setting bits
        if (sz > 0)
        {
            int32_t nOper = randBitSet->nextInt(sz);
            for (int32_t j = 0; j < nOper; ++j)
            {
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
                BOOST_CHECK_NE(val, val2);

                val = b->getAndSet(idx);
                BOOST_CHECK_EQUAL(val2, val);
                BOOST_CHECK(b->get(idx));
                
                if (!val)
                    b->fastClear(idx);
                BOOST_CHECK_EQUAL(b->get(idx), val);
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
        
        if (a0)
        {
            BOOST_CHECK_EQUAL(a->equals(a0), b->equals(b0));
            BOOST_CHECK_EQUAL(a->cardinality(), b->cardinality());

            BitSetPtr a_and = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_and->andBitSet(a0);
            BitSetPtr a_or = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_or->orBitSet(a0);
            BitSetPtr a_xor = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_xor->xorBitSet(a0);
            BitSetPtr a_andn = boost::dynamic_pointer_cast<BitSet>(a->clone());
            a_andn->andNotBitSet(a0);

            OpenBitSetPtr b_and = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            BOOST_CHECK(b->equals(b_and));
            b_and->andBitSet(b0);
            OpenBitSetPtr b_or = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_or->orBitSet(b0);
            OpenBitSetPtr b_xor = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_xor->xorBitSet(b0);
            OpenBitSetPtr b_andn = boost::dynamic_pointer_cast<OpenBitSet>(b->clone());
            b_andn->andNotBitSet(b0);

            doIterate(a_and, b_and, mode);
            doIterate(a_or, b_or, mode);
            doIterate(a_xor, b_xor, mode);
            doIterate(a_andn, b_andn, mode);
            
            BOOST_CHECK_EQUAL(a_and->cardinality(), b_and->cardinality());
            BOOST_CHECK_EQUAL(a_or->cardinality(), b_or->cardinality());
            BOOST_CHECK_EQUAL(a_xor->cardinality(), b_xor->cardinality());
            BOOST_CHECK_EQUAL(a_andn->cardinality(), b_andn->cardinality());
            
            // test non-mutating popcounts
            BOOST_CHECK_EQUAL(b_and->cardinality(), OpenBitSet::intersectionCount(b, b0));
            BOOST_CHECK_EQUAL(b_or->cardinality(), OpenBitSet::unionCount(b, b0));
            BOOST_CHECK_EQUAL(b_xor->cardinality(), OpenBitSet::xorCount(b, b0));
            BOOST_CHECK_EQUAL(b_andn->cardinality(), OpenBitSet::andNotCount(b, b0));
        }
        
        a0=a;
        b0=b;
    }
}

BOOST_AUTO_TEST_CASE(testSmall)
{
    randBitSet->setSeed(17);
    doRandomSets(1200, 1000, 1);
    doRandomSets(1200, 1000, 2);
}

/*
BOOST_AUTO_TEST_CASE(testBig)
{
    randBitSet->setSeed(17);
    doRandomSets(2000, 200000, 1);
    doRandomSets(2000, 200000, 2);
}
*/

BOOST_AUTO_TEST_CASE(testEquals)
{
    randBitSet->setSeed(17);
    OpenBitSetPtr b1 = newLucene<OpenBitSet>(1111);
    OpenBitSetPtr b2 = newLucene<OpenBitSet>(2222);
    BOOST_CHECK(b1->equals(b2));
    BOOST_CHECK(b2->equals(b1));
    b1->set(10);
    BOOST_CHECK(!b1->equals(b2));
    BOOST_CHECK(!b2->equals(b1));
    b2->set(10);
    BOOST_CHECK(b1->equals(b2));
    BOOST_CHECK(b2->equals(b1));
    b2->set(2221);
    BOOST_CHECK(!b1->equals(b2));
    BOOST_CHECK(!b2->equals(b1));
    b1->set(2221);
    BOOST_CHECK(b1->equals(b2));
    BOOST_CHECK(b2->equals(b1));
}

BOOST_AUTO_TEST_CASE(testBitUtils)
{
    randBitSet->setSeed(17);
    uint64_t num = 100000;
    BOOST_CHECK_EQUAL(5, BitUtil::ntz(num));
    BOOST_CHECK_EQUAL(5, BitUtil::ntz2(num));
    BOOST_CHECK_EQUAL(5, BitUtil::ntz3(num));

    num = 10;
    BOOST_CHECK_EQUAL(1, BitUtil::ntz(num));
    BOOST_CHECK_EQUAL(1, BitUtil::ntz2(num));
    BOOST_CHECK_EQUAL(1, BitUtil::ntz3(num));
    
    for (int32_t i = 0; i < 64; ++i)
    {
        num = (uint64_t)1 << i;
        BOOST_CHECK_EQUAL(i, BitUtil::ntz(num));
        BOOST_CHECK_EQUAL(i, BitUtil::ntz2(num));
        BOOST_CHECK_EQUAL(i, BitUtil::ntz3(num));
    }
}

BOOST_AUTO_TEST_SUITE_END()
