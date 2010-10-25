/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "CloseableThreadLocal.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CloseableThreadLocalTest, LuceneTestFixture)

static const String TEST_VALUE = L"initvaluetest";

BOOST_AUTO_TEST_CASE(testInitValue)
{
    class InitValueThreadLocal : public CloseableThreadLocal<String>
    {
    public:
        virtual ~InitValueThreadLocal()
        {
        }

    protected:
        virtual boost::shared_ptr<String> initialValue()
        {
            return newInstance<String>(TEST_VALUE);
        }
    };

    InitValueThreadLocal tl;
    String str = *(tl.get());
    BOOST_CHECK_EQUAL(TEST_VALUE, str);
}

/// Tests that null can be set as a valid value.
BOOST_AUTO_TEST_CASE(testNullValue)
{
    CloseableThreadLocal<String> ctl;
    ctl.set(boost::shared_ptr<String>());
    BOOST_CHECK(!ctl.get());
}

/// Make sure default get returns null, twice in a row
BOOST_AUTO_TEST_CASE(testDefaultValueWithoutSetting)
{
    CloseableThreadLocal<String> ctl;
    BOOST_CHECK(!ctl.get());
    BOOST_CHECK(!ctl.get());
}

BOOST_AUTO_TEST_SUITE_END()
