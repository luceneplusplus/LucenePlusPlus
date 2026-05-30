/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "EdgeNGramTokenFilter.h"
#include "PositionIncrementAttribute.h"
#include "StringReader.h"
#include "TokenFilter.h"
#include "TermAttribute.h"
#include "WhitespaceTokenizer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture EdgeNGramTokenFilterTest;

namespace {

class ZeroPositionIncrementTokenFilter : public TokenFilter {
public:
    ZeroPositionIncrementTokenFilter(const TokenStreamPtr& input) : TokenFilter(input) {
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        started = false;
    }

    virtual ~ZeroPositionIncrementTokenFilter() {
    }

    LUCENE_CLASS(ZeroPositionIncrementTokenFilter);

protected:
    PositionIncrementAttributePtr posIncrAtt;
    bool started;

public:
    virtual bool incrementToken() {
        if (!input->incrementToken()) {
            return false;
        }

        if (started) {
            posIncrAtt->setPositionIncrement(0);
        } else {
            started = true;
        }
        return true;
    }

    virtual void reset() {
        TokenFilter::reset();
        started = false;
    }
};

}

TEST_F(EdgeNGramTokenFilterTest, testMinMaxGrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde")), 2, 3);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"ab", L"abc"),
                             newCollection<int32_t>(0, 0),
                             newCollection<int32_t>(5, 5),
                             newCollection<int32_t>(1, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testFrontUnigram) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde")), 1, 1);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a"),
                             newCollection<int32_t>(0),
                             newCollection<int32_t>(5),
                             newCollection<int32_t>(1));
}

TEST_F(EdgeNGramTokenFilterTest, testOversizedNgrams) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde")), 6, 6);
    checkTokenStreamContents(stream,
                             Collection<String>::newInstance(),
                             Collection<int32_t>::newInstance(),
                             Collection<int32_t>::newInstance());
}

TEST_F(EdgeNGramTokenFilterTest, testOversizedNgramsPreserveOriginal) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde")), 6, 6, true);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"abcde"),
                             newCollection<int32_t>(0),
                             newCollection<int32_t>(5),
                             newCollection<int32_t>(1));
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
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"a bcd efghi jk")), 2, 3, true);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a", L"bc", L"bcd", L"ef", L"efg", L"efghi", L"jk"),
                             newCollection<int32_t>(0, 2, 2, 6, 6, 6, 12),
                             newCollection<int32_t>(1, 5, 5, 11, 11, 11, 14),
                             newCollection<int32_t>(1, 1, 0, 1, 0, 0, 1));
}

TEST_F(EdgeNGramTokenFilterTest, testPreserveOriginalDisabled) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"a bcd efghi jk")), 2, 3, false);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"bc", L"bcd", L"ef", L"efg", L"jk"),
                             newCollection<int32_t>(2, 2, 6, 6, 12),
                             newCollection<int32_t>(5, 5, 11, 11, 14),
                             newCollection<int32_t>(2, 0, 1, 0, 1));
}

TEST_F(EdgeNGramTokenFilterTest, testFilterPositions) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde vwxyz")), 1, 3);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"a", L"ab", L"abc", L"v", L"vw", L"vwx"),
                             newCollection<int32_t>(0, 0, 0, 6, 6, 6),
                             newCollection<int32_t>(5, 5, 5, 11, 11, 11),
                             newCollection<int32_t>(1, 0, 0, 1, 0, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testFirstTokenPositionIncrement) {
    TokenStreamPtr stream = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"a abc"));
    stream = newLucene<ZeroPositionIncrementTokenFilter>(stream);
    stream = newLucene<EdgeNGramTokenFilter>(stream, 2, 3, false);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"ab", L"abc"),
                             newCollection<int32_t>(2, 2),
                             newCollection<int32_t>(5, 5),
                             newCollection<int32_t>(1, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testSmallTokenInStream) {
    TokenStreamPtr stream = newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc de fgh")), 3, 3, false);
    checkTokenStreamContents(stream,
                             newCollection<String>(L"abc", L"fgh"),
                             newCollection<int32_t>(0, 7),
                             newCollection<int32_t>(3, 10),
                             newCollection<int32_t>(1, 2));
}

TEST_F(EdgeNGramTokenFilterTest, testReset) {
    TokenizerPtr tokenizer = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abcde"));
    TokenStreamPtr filter = newLucene<EdgeNGramTokenFilter>(tokenizer, 1, 3, false);
    checkTokenStreamContents(filter,
                             newCollection<String>(L"a", L"ab", L"abc"),
                             newCollection<int32_t>(0, 0, 0),
                             newCollection<int32_t>(5, 5, 5),
                             newCollection<int32_t>(1, 0, 0));

    tokenizer->reset(newLucene<StringReader>(L"abcde"));
    checkTokenStreamContents(filter,
                             newCollection<String>(L"a", L"ab", L"abc"),
                             newCollection<int32_t>(0, 0, 0),
                             newCollection<int32_t>(5, 5, 5),
                             newCollection<int32_t>(1, 0, 0));
}

TEST_F(EdgeNGramTokenFilterTest, testInvalidArguments) {
    EXPECT_THROW(newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc")), 0, 1), IllegalArgumentException);
    EXPECT_THROW(newLucene<EdgeNGramTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(L"abc")), 3, 2), IllegalArgumentException);
}
