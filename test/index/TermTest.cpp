/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Term.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(TermTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testEquals)
{
    TermPtr base = newLucene<Term>(L"same", L"same");
    TermPtr same = newLucene<Term>(L"same", L"same");
    TermPtr differentField = newLucene<Term>(L"different", L"same");
    TermPtr differentText = newLucene<Term>(L"same", L"different");
    BOOST_CHECK(base->equals(base));
    BOOST_CHECK(base->equals(same));
    BOOST_CHECK(!base->equals(differentField));
    BOOST_CHECK(!base->equals(differentText));
}

BOOST_AUTO_TEST_SUITE_END()
