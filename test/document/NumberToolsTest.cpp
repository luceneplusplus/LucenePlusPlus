/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "NumberTools.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(NumberToolsTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testMinValue)
{
    BOOST_CHECK_EQUAL(NumberTools::MIN_STRING_VALUE(), L"-0000000000000");
}

BOOST_AUTO_TEST_CASE(testMaxValue)
{
    BOOST_CHECK_EQUAL(NumberTools::MAX_STRING_VALUE(), L"01y2p0ij32e8e7");
}

BOOST_AUTO_TEST_CASE(testValueSize)
{
    BOOST_CHECK_EQUAL(NumberTools::STR_SIZE(), 14);
}

BOOST_AUTO_TEST_CASE(testLongToString)
{
    BOOST_CHECK_EQUAL(NumberTools::longToString(LLONG_MIN), L"-0000000000000");
    BOOST_CHECK_EQUAL(NumberTools::longToString(LLONG_MAX), L"01y2p0ij32e8e7");
    BOOST_CHECK_EQUAL(NumberTools::longToString(1LL), L"00000000000001");
    BOOST_CHECK_EQUAL(NumberTools::longToString(999LL), L"000000000000rr");
    BOOST_CHECK_EQUAL(NumberTools::longToString(34234LL), L"00000000000qey");
    BOOST_CHECK_EQUAL(NumberTools::longToString(4345325254LL), L"00000001zv3efa");
    BOOST_CHECK_EQUAL(NumberTools::longToString(986778657657575LL), L"00009ps7uuwdlz");
    BOOST_CHECK_EQUAL(NumberTools::longToString(23232143543434234LL), L"0006cr3vell8my");
}

BOOST_AUTO_TEST_CASE(testStringToLong)
{
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"-0000000000000"), LLONG_MIN);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"01y2p0ij32e8e7"), LLONG_MAX);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"00000000000001"), 1LL);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"000000000000rr"), 999LL);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"00000000000qey"), 34234LL);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"00000001zv3efa"), 4345325254LL);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"00009ps7uuwdlz"), 986778657657575LL);
    BOOST_CHECK_EQUAL(NumberTools::stringToLong(L"0006cr3vell8my"), 23232143543434234LL);
    BOOST_CHECK_EXCEPTION(NumberTools::stringToLong(L"32132"), LuceneException, check_exception(LuceneException::NumberFormat)); // wrong length
    BOOST_CHECK_EXCEPTION(NumberTools::stringToLong(L"9006cr3vell8my"), LuceneException, check_exception(LuceneException::NumberFormat)); // wrong prefix
}

BOOST_AUTO_TEST_SUITE_END()
