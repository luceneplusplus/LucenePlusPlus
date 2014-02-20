/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef LuceneTestFixture DateToolsTest;

TEST_F(DateToolsTest, testDateToString) {
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_YEAR), L"2010");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_MONTH), L"201001");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14)), DateTools::RESOLUTION_DAY), L"20100114");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_HOUR), L"2010011403");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_MINUTE), L"201001140341");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5)), DateTools::RESOLUTION_SECOND), L"20100114034105");
    EXPECT_EQ(DateTools::dateToString(ptime(date(2010, Jan, 14), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MILLISECOND), L"20100114034105123");
}

TEST_F(DateToolsTest, testTimeToString) {
    EXPECT_EQ(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_YEAR), L"2010");
    EXPECT_EQ(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_MONTH), L"201001");
    EXPECT_EQ(DateTools::timeToString(1263427200000LL, DateTools::RESOLUTION_DAY), L"20100114");
    EXPECT_EQ(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_HOUR), L"2010011403");
    EXPECT_EQ(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_MINUTE), L"201001140341");
    EXPECT_EQ(DateTools::timeToString(1263440465000LL, DateTools::RESOLUTION_SECOND), L"20100114034105");
    EXPECT_EQ(DateTools::timeToString(1263440465123LL, DateTools::RESOLUTION_MILLISECOND), L"20100114034105123");
}

TEST_F(DateToolsTest, testStringToTime) {
    EXPECT_EQ(DateTools::stringToTime(L"2010"), 1262304000000LL);
    EXPECT_EQ(DateTools::stringToTime(L"201001"), 1262304000000LL);
    EXPECT_EQ(DateTools::stringToTime(L"20100114"), 1263427200000LL);
    EXPECT_EQ(DateTools::stringToTime(L"2010011403"), 1263438000000LL);
    EXPECT_EQ(DateTools::stringToTime(L"201001140341"), 1263440460000LL);
    EXPECT_EQ(DateTools::stringToTime(L"20100114034105"), 1263440465000LL);
    EXPECT_EQ(DateTools::stringToTime(L"20100114034105123"), 1263440465123LL);
}

TEST_F(DateToolsTest, testDateRound) {
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_YEAR), ptime(date(2010, Jan, 1)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MONTH), ptime(date(2010, Feb, 1)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_DAY), ptime(date(2010, Feb, 16)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_HOUR), ptime(date(2010, Feb, 16), hours(3)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MINUTE), ptime(date(2010, Feb, 16), hours(3) + minutes(41)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_SECOND), ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5)));
    EXPECT_EQ(DateTools::round(ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)), DateTools::RESOLUTION_MILLISECOND), ptime(date(2010, Feb, 16), hours(3) + minutes(41) + seconds(5) + milliseconds(123)));
}

TEST_F(DateToolsTest, testParseDateGB) {
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    EXPECT_EQ(DateTools::parseDate(L"01122005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"011205"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01/12/2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01/12/05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1/12/2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1/12/05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1/1/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1/Jan/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01/Jan/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01/Jan/2005"), ptime(date(2005, 01, 01)));
}

TEST_F(DateToolsTest, testParseDateUS) {
    DateTools::setDateOrder(DateTools::DATEORDER_MDY);
    EXPECT_EQ(DateTools::parseDate(L"12012005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"120105"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"12/01/2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"12/01/05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"12/1/2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"12/1/05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1/1/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"Jan/1/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"Jan/01/05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"Jan/01/2005"), ptime(date(2005, 01, 01)));
}

TEST_F(DateToolsTest, testParseDateLocale) {
    bool hasThisLocale = false;

    try {
        std::locale("en_GB.UTF-8");
        hasThisLocale = true;
    } catch (...) {
    }

    if (hasThisLocale) {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        EXPECT_EQ(DateTools::parseDate(L"01122005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"011205", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"01/12/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"01/12/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"1/12/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"1/12/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"1/1/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"1/Jan/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"01/Jan/05", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"01/Jan/2005", std::locale("en_GB.UTF-8")), ptime(date(2005, 01, 01)));
    }

    try {
        std::locale("en_US.UTF-8");
        hasThisLocale = true;
    } catch (...) {
        hasThisLocale = false;
    }

    if (hasThisLocale) {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        EXPECT_EQ(DateTools::parseDate(L"12012005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"120105", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"12/01/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"12/01/05", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"12/1/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"12/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 12, 01)));
        EXPECT_EQ(DateTools::parseDate(L"1/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"Jan/1/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"Jan/01/05", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
        EXPECT_EQ(DateTools::parseDate(L"Jan/01/2005", std::locale("en_US.UTF-8")), ptime(date(2005, 01, 01)));
    }
}

TEST_F(DateToolsTest, testParseDateSeparator) {
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    EXPECT_EQ(DateTools::parseDate(L"01122005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"011205"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01-12-2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01 12 05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1.12.2005"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1.12.05"), ptime(date(2005, 12, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1 1 05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"1 Jan 05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01-Jan-05"), ptime(date(2005, 01, 01)));
    EXPECT_EQ(DateTools::parseDate(L"01,Jan,2005"), ptime(date(2005, 01, 01)));
}
