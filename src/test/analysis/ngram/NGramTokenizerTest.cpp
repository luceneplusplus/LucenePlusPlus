/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "NGramAnalyzer.h"
#include "NGramTokenizer.h"
#include "StringReader.h"

using namespace Lucene;

typedef BaseTokenStreamFixture NGramTokenizerTest;

TEST_F(NGramTokenizerTest, testDefaultGrams) {
    TokenStreamPtr stream = newLucene<NGramTokenizer>(newLucene<StringReader>(L"abc"));
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a", L"ab", L"b", L"bc", L"c"),
                             newCollection<int32_t>(0, 0, 1, 1, 2),
                             newCollection<int32_t>(1, 2, 2, 3, 3),
                             newCollection<int32_t>(1, 1, 1, 1, 1),
                             3);
}

TEST_F(NGramTokenizerTest, testMinMaxGrams) {
    TokenStreamPtr stream = newLucene<NGramTokenizer>(newLucene<StringReader>(L"abcde"), 2, 3);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"ab", L"abc", L"bc", L"bcd", L"cd", L"cde", L"de"),
                             newCollection<int32_t>(0, 0, 1, 1, 2, 2, 3),
                             newCollection<int32_t>(2, 3, 3, 4, 4, 5, 5),
                             newCollection<int32_t>(1, 1, 1, 1, 1, 1, 1),
                             5);
}

TEST_F(NGramTokenizerTest, testPreservesCase) {
    TokenStreamPtr stream = newLucene<NGramTokenizer>(newLucene<StringReader>(L"AbC"), 2, 2);
    checkTokenStreamContents(stream, newCollection<String>(L"Ab", L"bC"));
}

TEST_F(NGramTokenizerTest, testAnalyzerReuse) {
    AnalyzerPtr analyzer = newLucene<NGramAnalyzer>(2, 3);
    checkAnalyzesToReuse(analyzer, L"abc", newCollection<String>(L"ab", L"abc", L"bc"),
                         newCollection<int32_t>(0, 0, 1), newCollection<int32_t>(2, 3, 3),
                         newCollection<int32_t>(1, 1, 1));
    checkAnalyzesToReuse(analyzer, L"xy", newCollection<String>(L"xy"),
                         newCollection<int32_t>(0), newCollection<int32_t>(2),
                         newCollection<int32_t>(1));
}

TEST_F(NGramTokenizerTest, testInvalidArguments) {
    EXPECT_THROW(newLucene<NGramTokenizer>(newLucene<StringReader>(L"abc"), 0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<NGramTokenizer>(newLucene<StringReader>(L"abc"), 3, 2), IllegalArgumentException);
    EXPECT_THROW(newLucene<NGramAnalyzer>(0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<NGramAnalyzer>(3, 2), IllegalArgumentException);
}
