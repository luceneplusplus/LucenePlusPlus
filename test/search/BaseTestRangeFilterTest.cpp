/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTestRangeFilterFixture.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(BaseTestRangeFilterTest, BaseTestRangeFilterFixture)

BOOST_AUTO_TEST_CASE(testPad)
{
    Collection<int32_t> tests = newCollection<int32_t>(-9999999, -99560, -100, -1, 0, 3, 9, 10, 1000, 999999999);
    for (int32_t i = 0; i < tests.size() - 1; ++i)
    {
        int32_t a = tests[i];
        int32_t b = tests[i + 1];
        String aa = pad(a);
        String bb = pad(b);
        BOOST_CHECK_EQUAL(aa.length(), bb.length());
        BOOST_CHECK(aa.compare(bb) < 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()
