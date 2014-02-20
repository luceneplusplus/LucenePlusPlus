/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "DateField.h"

using namespace Lucene;

typedef LuceneTestFixture DateFieldTest;

TEST_F(DateFieldTest, testMinDate) {
    EXPECT_EQ(DateField::MIN_DATE_STRING(), L"000000000");
}

TEST_F(DateFieldTest, testMaxDate) {
    EXPECT_EQ(DateField::MAX_DATE_STRING(), L"zzzzzzzzz");
}

TEST_F(DateFieldTest, testDateToString) {
    EXPECT_EQ(DateField::dateToString(boost::posix_time::ptime(boost::gregorian::date(2010, boost::gregorian::Jan, 14))), L"0g4erxmo0");
}

TEST_F(DateFieldTest, testTimeToString) {
    EXPECT_EQ(DateField::timeToString(1263427200000LL), L"0g4erxmo0");
}

TEST_F(DateFieldTest, testStringToTime) {
    EXPECT_EQ(DateField::stringToTime(L"0g4erxmo0"), 1263427200000LL);
}
