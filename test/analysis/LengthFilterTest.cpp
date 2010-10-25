/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "WhitespaceTokenizer.h"
#include "TokenStream.h"
#include "StringReader.h"
#include "LengthFilter.h"
#include "TermAttribute.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(LengthFilterTest, BaseTokenStreamFixture)

BOOST_AUTO_TEST_CASE(testFilter)
{
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"short toolong evenmuchlongertext a ab toolong foo"));
    LengthFilterPtr filter = newLucene<LengthFilter>(stream, 2, 6);
    TermAttributePtr termAtt = filter->getAttribute<TermAttribute>();

    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"short", termAtt->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"ab", termAtt->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"foo", termAtt->term());
    BOOST_CHECK(!filter->incrementToken());
}

BOOST_AUTO_TEST_SUITE_END()
