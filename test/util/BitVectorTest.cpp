/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "BitVector.h"
#include "RAMDirectory.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(BitVectorTest, LuceneTestFixture)

static const int32_t subsetPattern[] = {1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1};

static bool compareBitVectors(BitVectorPtr bv, BitVectorPtr compare)
{
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        // bits must be equal
        if (bv->get(i) != compare->get(i))
            return false;
    }
    return true;
}

static void doTestConstructOfSize(int32_t n)
{
    BitVectorPtr bv = newLucene<BitVector>(n);
    BOOST_CHECK_EQUAL(n, bv->size());
}

/// Test the default constructor on BitVectors of various sizes.
BOOST_AUTO_TEST_CASE(testConstructSize)
{
    doTestConstructOfSize(8);
    doTestConstructOfSize(20);
    doTestConstructOfSize(100);
    doTestConstructOfSize(1000);
}

static void doTestGetSetVectorOfSize(int32_t n)
{
    BitVectorPtr bv = newLucene<BitVector>(n);
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        BOOST_CHECK(!bv->get(i));
        bv->set(i);
        BOOST_CHECK(bv->get(i));
    }
}

/// Test the get() and set() methods on BitVectors of various sizes.
BOOST_AUTO_TEST_CASE(testGetSet)
{
    doTestGetSetVectorOfSize(8);
    doTestGetSetVectorOfSize(20);
    doTestGetSetVectorOfSize(100);
    doTestGetSetVectorOfSize(1000);
}

static void doTestClearVectorOfSize(int32_t n)
{
    BitVectorPtr bv = newLucene<BitVector>(n);
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        BOOST_CHECK(!bv->get(i));
        bv->set(i);
        BOOST_CHECK(bv->get(i));
        bv->clear(i);
        BOOST_CHECK(!bv->get(i));
    }
}

/// Test the clear() method on BitVectors of various sizes.
BOOST_AUTO_TEST_CASE(testClear)
{
    doTestClearVectorOfSize(8);
    doTestClearVectorOfSize(20);
    doTestClearVectorOfSize(100);
    doTestClearVectorOfSize(1000);
}

static void doTestCountVectorOfSize(int32_t n)
{
    BitVectorPtr bv = newLucene<BitVector>(n);
    // test count when incrementally setting bits
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        BOOST_CHECK(!bv->get(i));
        BOOST_CHECK_EQUAL(i, bv->count());
        bv->set(i);
        BOOST_CHECK(bv->get(i));
        BOOST_CHECK_EQUAL(i + 1, bv->count());
    }
    
    bv = newLucene<BitVector>(n);
    // test count when setting then clearing bits
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        BOOST_CHECK(!bv->get(i));
        BOOST_CHECK_EQUAL(0, bv->count());
        bv->set(i);
        BOOST_CHECK(bv->get(i));
        BOOST_CHECK_EQUAL(1, bv->count());
        bv->clear(i);
        BOOST_CHECK(!bv->get(i));
        BOOST_CHECK_EQUAL(0, bv->count());
    }
}

/// Test the count() method on BitVectors of various sizes.
BOOST_AUTO_TEST_CASE(testCount)
{
    doTestCountVectorOfSize(8);
    doTestCountVectorOfSize(20);
    doTestCountVectorOfSize(100);
    doTestCountVectorOfSize(1000);
}

static void doTestWriteRead(int32_t n)
{
    DirectoryPtr d = newLucene<RAMDirectory>();
    BitVectorPtr bv = newLucene<BitVector>(n);
    // test count when incrementally setting bits
    for (int32_t i = 0; i < bv->size(); ++i)
    {
        BOOST_CHECK(!bv->get(i));
        BOOST_CHECK_EQUAL(i, bv->count());
        bv->set(i);
        BOOST_CHECK(bv->get(i));
        BOOST_CHECK_EQUAL(i + 1, bv->count());
        bv->write(d, L"TESTBV");
        BitVectorPtr compare = newLucene<BitVector>(d, L"TESTBV");
        // compare bit vectors with bits set incrementally
        BOOST_CHECK(compareBitVectors(bv, compare));
    }
}

/// Test writing and construction to/from Directory.
BOOST_AUTO_TEST_CASE(testWriteRead)
{
    doTestWriteRead(8);
    doTestWriteRead(20);
    doTestWriteRead(100);
    doTestWriteRead(1000);
}

static void doTestDgaps(int32_t size, int32_t count1, int32_t count2)
{
    DirectoryPtr d = newLucene<RAMDirectory>();
    BitVectorPtr bv = newLucene<BitVector>(size);
    for (int32_t i = 0; i < count1; ++i)
    {
        bv->set(i);
        BOOST_CHECK_EQUAL(i + 1, bv->count());
    }
    bv->write(d, L"TESTBV");
    // gradually increase number of set bits
    for (int32_t i = count1; i < count2; ++i)
    {
        BitVectorPtr bv2 = newLucene<BitVector>(d, L"TESTBV");
        BOOST_CHECK(compareBitVectors(bv, bv2));
        bv = bv2;
        bv->set(i);
        BOOST_CHECK_EQUAL(i + 1, bv->count());
        bv->write(d, L"TESTBV");
    }
    // now start decreasing number of set bits
    for (int32_t i = count2 - 1; i >= count1; --i)
    {
        BitVectorPtr bv2 = newLucene<BitVector>(d, L"TESTBV");
        BOOST_CHECK(compareBitVectors(bv, bv2));
        bv = bv2;
        bv->clear(i);
        BOOST_CHECK_EQUAL(i, bv->count());
        bv->write(d, L"TESTBV");
    }
}

/// Test r/w when size/count cause switching between bit-set and d-gaps file formats.
BOOST_AUTO_TEST_CASE(testDgaps)
{
    doTestDgaps(1, 0, 1);
    doTestDgaps(10, 0, 1);
    doTestDgaps(100, 0, 1);
    doTestDgaps(1000, 4, 7);
    doTestDgaps(10000, 40, 43);
    doTestDgaps(100000, 415, 418);
    doTestDgaps(1000000, 3123, 3126);
}

static BitVectorPtr createSubsetTestVector()
{
    int32_t length = SIZEOF_ARRAY(subsetPattern);
    BitVectorPtr bv = newLucene<BitVector>(length);
    for (int32_t i = 0; i < length; ++i)
    {
        if (subsetPattern[i] == 1)
            bv->set(i);
    }
    return bv;
}

/// Compare a subset against the corresponding portion of the test pattern
static void doTestSubset(int32_t start, int32_t end)
{
    BitVectorPtr full = createSubsetTestVector();
    BitVectorPtr subset = full->subset(start, end);
    BOOST_CHECK_EQUAL(end - start, subset->size());
    int32_t count = 0;
    for (int32_t i = start, j = 0; i < end; ++i, ++j)
    {
        if (subsetPattern[i] == 1)
        {
            ++count;
            BOOST_CHECK(subset->get(j));
        }
        else
            BOOST_CHECK(!subset->get(j));
    }
    BOOST_CHECK_EQUAL(count, subset->count());
}

/// Tests BitVector.subset() against a pattern
BOOST_AUTO_TEST_CASE(testSubset)
{
    doTestSubset(0, 0);
    doTestSubset(0, 20);
    doTestSubset(0, 7);
    doTestSubset(0, 8);
    doTestSubset(0, 9);
    doTestSubset(0, 15);
    doTestSubset(0, 16);
    doTestSubset(0, 17);
    doTestSubset(1, 7);
    doTestSubset(1, 8);
    doTestSubset(1, 9);
    doTestSubset(1, 15);
    doTestSubset(1, 16);
    doTestSubset(1, 17);
    doTestSubset(2, 20);
    doTestSubset(3, 20);
    doTestSubset(4, 20);
    doTestSubset(5, 20);
    doTestSubset(6, 20);
    doTestSubset(7, 14);
    doTestSubset(7, 15);
    doTestSubset(7, 16);
    doTestSubset(8, 15);
    doTestSubset(9, 20);
    doTestSubset(10, 20);
    doTestSubset(11, 20);
    doTestSubset(12, 20);
    doTestSubset(13, 20);
}

BOOST_AUTO_TEST_SUITE_END()
