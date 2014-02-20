/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef BaseTokenStreamFixture LengthFilterTest;

TEST_F(LengthFilterTest, testFilter) {
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"short toolong evenmuchlongertext a ab toolong foo"));
    LengthFilterPtr filter = newLucene<LengthFilter>(stream, 2, 6);
    TermAttributePtr termAtt = filter->getAttribute<TermAttribute>();

    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"short", termAtt->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"ab", termAtt->term());
    EXPECT_TRUE(filter->incrementToken());
    EXPECT_EQ(L"foo", termAtt->term());
    EXPECT_TRUE(!filter->incrementToken());
}
