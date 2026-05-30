/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "EdgeNGramTokenFilter.h"
#include "StringReader.h"
#include "WhitespaceTokenizer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture EdgeNGramTokenFilterTest;

TEST_F(EdgeNGramTokenFilterTest, testMinMaxGrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde")), 2, 3);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"ab", L"abc"),
                             newCollection<int32_t>(0, 0),
                             newCollection<int32_t>(5, 5),
                             newCollection<int32_t>(1, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testMultipleInputTokens) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc xy")), 2, 2);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"ab", L"xy"),
                             newCollection<int32_t>(0, 4),
                             newCollection<int32_t>(3, 6),
                             newCollection<int32_t>(1, 1));
}

TEST_F(EdgeNGramTokenFilterTest, testPreserveOriginal) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"a abcd")), 2, 3, true);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a", L"ab", L"abc", L"abcd"),
                             newCollection<int32_t>(0, 2, 2, 2),
                             newCollection<int32_t>(1, 6, 6, 6),
                             newCollection<int32_t>(1, 1, 0, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testInvalidArguments) {
    EXPECT_THROW(newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc")), 0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc")), 3, 2), IllegalArgumentException);
}
