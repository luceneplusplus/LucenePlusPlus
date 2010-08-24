/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseTokenStreamFixture.h"
#include "ReverseStringFilter.h"
#include "WhitespaceTokenizer.h"
#include "StringReader.h"
#include "TermAttribute.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(ReverseStringFilterTest, BaseTokenStreamFixture)

BOOST_AUTO_TEST_CASE(testFilter)
{
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"Do have a nice day")); // 1-4 length string
    ReverseStringFilterPtr filter = newLucene<ReverseStringFilter>(stream);
    TermAttributePtr text = filter->getAttribute<TermAttribute>();
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"oD", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"evah", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"a", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"ecin", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(L"yad", text->term());
    BOOST_CHECK(!filter->incrementToken());
}

BOOST_AUTO_TEST_CASE(testFilterWithMark)
{
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"Do have a nice day")); // 1-4 length string
    ReverseStringFilterPtr filter = newLucene<ReverseStringFilter>(stream, (wchar_t)0x0001);
    TermAttributePtr text = filter->getAttribute<TermAttribute>();
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(String(1, (wchar_t)0x0001) + L"oD", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(String(1, (wchar_t)0x0001) + L"evah", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(String(1, (wchar_t)0x0001) + L"a", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(String(1, (wchar_t)0x0001) + L"ecin", text->term());
    BOOST_CHECK(filter->incrementToken());
    BOOST_CHECK_EQUAL(String(1, (wchar_t)0x0001) + L"yad", text->term());
    BOOST_CHECK(!filter->incrementToken());
}

BOOST_AUTO_TEST_SUITE_END()
