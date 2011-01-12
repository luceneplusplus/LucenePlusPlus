/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "DateField.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(DateFieldTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testMinDate)
{
    BOOST_CHECK_EQUAL(DateField::MIN_DATE_STRING(), L"000000000");
}

BOOST_AUTO_TEST_CASE(testMaxDate)
{
    BOOST_CHECK_EQUAL(DateField::MAX_DATE_STRING(), L"zzzzzzzzz");
}

BOOST_AUTO_TEST_CASE(testDateToString)
{
    BOOST_CHECK_EQUAL(DateField::dateToString(boost::posix_time::ptime(boost::gregorian::date(2010, boost::gregorian::Jan, 14))), L"0g4erxmo0");
}

BOOST_AUTO_TEST_CASE(testTimeToString)
{
    BOOST_CHECK_EQUAL(DateField::timeToString(1263427200000LL), L"0g4erxmo0");
}

BOOST_AUTO_TEST_CASE(testStringToTime)
{
    BOOST_CHECK_EQUAL(DateField::stringToTime(L"0g4erxmo0"), 1263427200000LL);
}

BOOST_AUTO_TEST_SUITE_END()
