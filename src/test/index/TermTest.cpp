/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Term.h"

using namespace Lucene;

typedef LuceneTestFixture TermTest;

TEST_F(TermTest, testEquals) {
    TermPtr base = newLucene<Term>(L"same", L"same");
    TermPtr same = newLucene<Term>(L"same", L"same");
    TermPtr differentField = newLucene<Term>(L"different", L"same");
    TermPtr differentText = newLucene<Term>(L"same", L"different");
    EXPECT_TRUE(base->equals(base));
    EXPECT_TRUE(base->equals(same));
    EXPECT_TRUE(!base->equals(differentField));
    EXPECT_TRUE(!base->equals(differentText));
}
