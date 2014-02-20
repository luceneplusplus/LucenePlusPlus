/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "CloseableThreadLocal.h"

using namespace Lucene;

typedef LuceneTestFixture CloseableThreadLocalTest;

static const String TEST_VALUE = L"initvaluetest";

TEST_F(CloseableThreadLocalTest, testInitValue) {
    class InitValueThreadLocal : public CloseableThreadLocal<String> {
    public:
        virtual ~InitValueThreadLocal() {
        }

    protected:
        virtual boost::shared_ptr<String> initialValue() {
            return newInstance<String>(TEST_VALUE);
        }
    };

    InitValueThreadLocal tl;
    String str = *(tl.get());
    EXPECT_EQ(TEST_VALUE, str);
}

/// Tests that null can be set as a valid value.
TEST_F(CloseableThreadLocalTest, testNullValue) {
    CloseableThreadLocal<String> ctl;
    ctl.set(boost::shared_ptr<String>());
    EXPECT_TRUE(!ctl.get());
}

/// Make sure default get returns null, twice in a row
TEST_F(CloseableThreadLocalTest, testDefaultValueWithoutSetting) {
    CloseableThreadLocal<String> ctl;
    EXPECT_TRUE(!ctl.get());
    EXPECT_TRUE(!ctl.get());
}
