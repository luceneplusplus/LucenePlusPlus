/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "NumberTools.h"

using namespace Lucene;

typedef LuceneTestFixture NumberToolsTest;

TEST_F(NumberToolsTest, testMinValue) {
    EXPECT_EQ(NumberTools::MIN_STRING_VALUE(), L"-0000000000000");
}

TEST_F(NumberToolsTest, testMaxValue) {
    EXPECT_EQ(NumberTools::MAX_STRING_VALUE(), L"01y2p0ij32e8e7");
}

TEST_F(NumberToolsTest, testValueSize) {
    EXPECT_EQ(NumberTools::STR_SIZE(), 14);
}

TEST_F(NumberToolsTest, testLongToString) {
    EXPECT_EQ(NumberTools::longToString(LLONG_MIN), L"-0000000000000");
    EXPECT_EQ(NumberTools::longToString(LLONG_MAX), L"01y2p0ij32e8e7");
    EXPECT_EQ(NumberTools::longToString(1LL), L"00000000000001");
    EXPECT_EQ(NumberTools::longToString(999LL), L"000000000000rr");
    EXPECT_EQ(NumberTools::longToString(34234LL), L"00000000000qey");
    EXPECT_EQ(NumberTools::longToString(4345325254LL), L"00000001zv3efa");
    EXPECT_EQ(NumberTools::longToString(986778657657575LL), L"00009ps7uuwdlz");
    EXPECT_EQ(NumberTools::longToString(23232143543434234LL), L"0006cr3vell8my");
}

TEST_F(NumberToolsTest, testStringToLong) {
    EXPECT_EQ(NumberTools::stringToLong(L"-0000000000000"), LLONG_MIN);
    EXPECT_EQ(NumberTools::stringToLong(L"01y2p0ij32e8e7"), LLONG_MAX);
    EXPECT_EQ(NumberTools::stringToLong(L"00000000000001"), 1LL);
    EXPECT_EQ(NumberTools::stringToLong(L"000000000000rr"), 999LL);
    EXPECT_EQ(NumberTools::stringToLong(L"00000000000qey"), 34234LL);
    EXPECT_EQ(NumberTools::stringToLong(L"00000001zv3efa"), 4345325254LL);
    EXPECT_EQ(NumberTools::stringToLong(L"00009ps7uuwdlz"), 986778657657575LL);
    EXPECT_EQ(NumberTools::stringToLong(L"0006cr3vell8my"), 23232143543434234LL);

    try {
        NumberTools::stringToLong(L"32132");
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::NumberFormat)(e)); // wrong length
    }

    try {
        NumberTools::stringToLong(L"9006cr3vell8my");
    } catch (LuceneException& e) {
        EXPECT_TRUE(check_exception(LuceneException::NumberFormat)(e)); // wrong prefix
    }
}
