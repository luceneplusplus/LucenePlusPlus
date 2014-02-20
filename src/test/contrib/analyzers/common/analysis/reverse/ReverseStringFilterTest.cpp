/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "ReverseStringFilter.h"
#include "WhitespaceTokenizer.h"
#include "StringReader.h"
#include "TermAttribute.h"

using namespace Lucene;

typedef BaseTokenStreamFixture ReverseStringFilterTest;

TEST_F(ReverseStringFilterTest, testFilter) {
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"Do have a nice day")); // 1-4 length string
    ReverseStringFilterPtr filter = newLucene<ReverseStringFilter>(stream);
    TermAttributePtr text = filter->getAttribute<TermAttribute>();
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"oD", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"evah", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"a", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"ecin", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"yad", text->term());
    EXPECT_TRUE(!filter->incrementToken());
}

TEST_F(ReverseStringFilterTest, testFilterWithMark) {
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"Do have a nice day")); // 1-4 length string
    ReverseStringFilterPtr filter = newLucene<ReverseStringFilter>(stream, (wchar_t)0x0001);
    TermAttributePtr text = filter->getAttribute<TermAttribute>();
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(String(1, (wchar_t)0x0001) + L"oD", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(String(1, (wchar_t)0x0001) + L"evah", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(String(1, (wchar_t)0x0001) + L"a", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(String(1, (wchar_t)0x0001) + L"ecin", text->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(String(1, (wchar_t)0x0001) + L"yad", text->term());
    EXPECT_TRUE(!filter->incrementToken());
}
