/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "LuceneTestFixture.h"
#include "DateTools.h"

using namespace Lucene;
using namespace boost::posix_time;
using namespace boost::gregorian;

BOOST_FIXTURE_TEST_SUITE(DateToolsTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testDateToString)
{
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_YEAR), L"2010");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_MONTH), L"201001");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_DAY), L"20100114");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_HOUR), L"2010011403");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_MINUTE), L"201001140341");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_SECOND), L"20100114034105");
    BOOST_CHECK_EQUAL(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MILLISECOND), L"20100114034105123");
}

BOOST_AUTO_TEST_CASE(testTimeToString)
{
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_YEAR), L"2010");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_MONTH), L"201001");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_DAY), L"20100114");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_HOUR), L"2010011403");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_MINUTE), L"201001140341");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_SECOND), L"20100114034105");
    BOOST_CHECK_EQUAL(DateTools::timeToString(1263440465123LL, DateTools::RESOLUTION_MILLISECOND), L"20100114034105123");
}

BOOST_AUTO_TEST_CASE(testStringToTime)
{
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"2010"), 1262304000000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"201001"), 1262304000000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"20100114"), 1263427200000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"2010011403"), 1263438000000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"201001140341"), 1263440460000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"20100114034105"), 1263440465000LL);
    BOOST_CHECK_EQUAL(DateTools::stringToTime(L"20100114034105123"), 1263440465123LL);
}

BOOST_AUTO_TEST_CASE(testDateRound)
{
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_YEAR), ptime(date(2010, Jan, 1)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MONTH), ptime(date(2010, Feb, 1)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_DAY), ptime(date(2010, Feb, 16)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_HOUR), ptime(date(2010, Feb, 16), hours(3)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MINUTE), ptime(date(2010, Feb, 16), hours(3) + minutes(41)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_SECOND), ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5)));
    BOOST_CHECK_EQUAL(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MILLISECOND), ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)));
}

BOOST_AUTO_TEST_CASE(testParseDateGB)
{
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01122005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"011205"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/12/2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/12/05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/12/2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/12/05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/1/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/Jan/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/Jan/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/Jan/2005"), ptime(date(2005, 01, 01)));
}

BOOST_AUTO_TEST_CASE(testParseDateUS)
{
    DateTools::setDateOrder(DateTools::DATEORDER_MDY);
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"12012005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"120105"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/01/2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/01/05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/1/2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/1/05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/1/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/1/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/01/05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/01/2005"), ptime(date(2005, 01, 01)));
}

BOOST_AUTO_TEST_CASE(testParseDateLocale)
{
    bool hasThisLocale = false;
    
    try
    {
        std::locale("en_GB.UTF-8");
        hasThisLocale = true;
    }
    catch (...)
    {
    }

    if (hasThisLocale)
    {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"01122005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"011205", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/12/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/12/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/12/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/12/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/1/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/Jan/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/Jan/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"01/Jan/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
    }

    try
    {
        std::locale("en_US.UTF-8");
        hasThisLocale = true;
    }
    catch (...)
    {
        hasThisLocale = false;
    }

    if (hasThisLocale)
    {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"12012005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"120105", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/01/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/01/05", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/1/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"12/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"1/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/01/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        BOOST_CHECK_EQUAL(DateTools::parseDate(L"Jan/01/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
    }
}

BOOST_AUTO_TEST_CASE(testParseDateSeparator)
{
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01122005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"011205"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01-12-2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01 12 05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1.12.2005"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1.12.05"), ptime(date(2005, 12, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1 1 05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"1 Jan 05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01-Jan-05"), ptime(date(2005, 01, 01)));
    BOOST_CHECK_EQUAL(DateTools::parseDate(L"01,Jan,2005"), ptime(date(2005, 01, 01)));
}

BOOST_AUTO_TEST_SUITE_END()
