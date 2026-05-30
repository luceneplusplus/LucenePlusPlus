/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "EdgeNGramAnalyzer.h"
#include "EdgeNGramTokenizer.h"
#include "StringReader.h"

using namespace Lucene;

typedef BaseTokenStreamFixture EdgeNGramTokenizerTest;

TEST_F(EdgeNGramTokenizerTest, testDefaultGrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"abc"));
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a"),
                             newCollection<int32_t>(0),
                             newCollection<int32_t>(1),
                             newCollection<int32_t>(1),
                             3);
}

TEST_F(EdgeNGramTokenizerTest, testMinMaxGrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"abcde"), 1, 3);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a", L"ab", L"abc"),
                             newCollection<int32_t>(0, 0, 0),
                             newCollection<int32_t>(1, 2, 3),
                             newCollection<int32_t>(1, 1, 1),
                             5);
}

TEST_F(EdgeNGramTokenizerTest, testOversizedNgrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"abcde"), 6, 6);
    checkTokenStreamContents(stream, Collection<String>::newInstance(), Collection<int32_t>::newInstance(), Collection<int32_t>::newInstance(),
                             Collection<int32_t>::newInstance(), 5);
}

TEST_F(EdgeNGramTokenizerTest, testPreservesCase) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"AbC"), 2, 2);
    checkTokenStreamContents(stream, newCollection<String>(L"Ab"));
}

TEST_F(EdgeNGramTokenizerTest, testAnalyzerReuse) {
    AnalyzerPtr analyzer = newLucene<EdgeNGramAnalyzer>(1, 3);
    checkAnalyzesToReuse(analyzer, L"abcde", newCollection<String>(L"a", L"ab", L"abc"),
                         newCollection<int32_t>(0, 0, 0), newCollection<int32_t>(1, 2, 3),
                         newCollection<int32_t>(1, 1, 1));
    checkAnalyzesToReuse(analyzer, L"xy", newCollection<String>(L"x", L"xy"),
                         newCollection<int32_t>(0, 0), newCollection<int32_t>(1, 2),
                         newCollection<int32_t>(1, 1));
}

TEST_F(EdgeNGramTokenizerTest, testInvalidArguments) {
    EXPECT_THROW(newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"abc"), 0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<EdgeNGramTokenizer>(newLucene<StringReader>(L"abc"), 3, 2), IllegalArgumentException);
    EXPECT_THROW(newLucene<EdgeNGramAnalyzer>(0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<EdgeNGramAnalyzer>(3, 2), IllegalArgumentException);
}
