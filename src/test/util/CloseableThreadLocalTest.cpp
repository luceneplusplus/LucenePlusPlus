/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "CloseableThreadLocal.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CloseableThreadLocalTest, LuceneTestFixture)

static const String TEST_VALUE = L"initvaluetest";

class TestString : public gc_object // todo: see if we can minimise the number of "public gc_object"
{
public:
    TestString(const String& str = L"") : string(str)
    {
    }

    virtual ~TestString()
    {
    }

    String string;
};

DECLARE_LUCENE_PTR(TestString)

class InitValueThreadLocal : public CloseableThreadLocal<TestString>
{
public:
    virtual ~InitValueThreadLocal()
    {
    }

protected:
    virtual gc_ptr<TestString> initialValue()
    {
        return new_gc<TestString>(TEST_VALUE);
    }
};

BOOST_AUTO_TEST_CASE(testInitValue)
{
    InitValueThreadLocal tl;
    String str = tl.get()->string;
    BOOST_CHECK_EQUAL(TEST_VALUE, str);
}

/// Tests that null can be set as a valid value.
BOOST_AUTO_TEST_CASE(testNullValue)
{
    CloseableThreadLocal<TestString> ctl;
    ctl.set(gc_ptr<TestString>());
    BOOST_CHECK(!ctl.get());
}

/// Make sure default get returns null, twice in a row
BOOST_AUTO_TEST_CASE(testDefaultValueWithoutSetting)
{
    CloseableThreadLocal<TestString> ctl;
    BOOST_CHECK(!ctl.get());
    BOOST_CHECK(!ctl.get());
}

BOOST_AUTO_TEST_SUITE_END()
