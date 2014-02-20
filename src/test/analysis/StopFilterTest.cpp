/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "TestUtils.h"
#include "StringReader.h"
#include "TokenStream.h"
#include "StopFilter.h"
#include "WhitespaceTokenizer.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"

using namespace Lucene;

typedef BaseTokenStreamFixture StopFilterTest;

static void doTestStopPositons(const StopFilterPtr& stpf, bool enableIcrements) {
    stpf->setEnablePositionIncrements(enableIcrements);
    TermAttributePtr termAtt = stpf->getAttribute<TermAttribute>();
    PositionIncrementAttributePtr posIncrAtt = stpf->getAttribute<PositionIncrementAttribute>();
    for (int32_t i = 0; i < 20; i += 3) {
        EXPECT_TRUE(stpf->incrementToken());
        String w = intToEnglish(i);
        EXPECT_EQ(w, termAtt->term());
        EXPECT_EQ(enableIcrements ? (i == 0 ? 1 : 3) : 1, posIncrAtt->getPositionIncrement());
    }
    EXPECT_TRUE(!stpf->incrementToken());
}

TEST_F(StopFilterTest, testExactCase) {
    StringReaderPtr reader = newLucene<StringReader>(L"Now is The Time");
    HashSet<String> stopWords = HashSet<String>::newInstance();
    stopWords.add(L"is");
    stopWords.add(L"the");
    stopWords.add(L"Time");
    TokenStreamPtr stream = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopWords, false);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    EXPECT_TRUE(stream->incrementToken());
    EXPECT_EQ(L"Now", termAtt->term());
    EXPECT_TRUE(stream->incrementToken());
    EXPECT_EQ(L"The", termAtt->term());
    EXPECT_TRUE(!stream->incrementToken());
}

TEST_F(StopFilterTest, testIgnoreCase) {
    StringReaderPtr reader = newLucene<StringReader>(L"Now is The Time");
    HashSet<String> stopWords = HashSet<String>::newInstance();
    stopWords.add(L"is");
    stopWords.add(L"the");
    stopWords.add(L"Time");
    TokenStreamPtr stream = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopWords, true);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    EXPECT_TRUE(stream->incrementToken());
    EXPECT_EQ(L"Now", termAtt->term());
    EXPECT_TRUE(!stream->incrementToken());
}

TEST_F(StopFilterTest, testStopPositons) {
    StringStream buf;
    Collection<String> stopWords = Collection<String>::newInstance();
    for (int32_t i = 0; i < 20; ++i) {
        String w = intToEnglish(i);
        buf << w << L" ";
        if (i % 3 != 0) {
            stopWords.add(w);
        }
    }
    HashSet<String> stopSet = HashSet<String>::newInstance(stopWords.begin(), stopWords.end());
    // with increments
    StringReaderPtr reader = newLucene<StringReader>(buf.str());
    StopFilterPtr stpf = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopSet);
    doTestStopPositons(stpf, true);
    // without increments
    reader = newLucene<StringReader>(buf.str());
    stpf = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopSet);
    doTestStopPositons(stpf, false);
    // with increments, concatenating two stop filters
    Collection<String> stopWords0 = Collection<String>::newInstance();
    Collection<String> stopWords1 = Collection<String>::newInstance();
    for (int32_t i = 0; i < stopWords.size(); ++i) {
        if (i % 2 == 0) {
            stopWords0.add(stopWords[i]);
        } else {
            stopWords1.add(stopWords[i]);
        }
    }
    HashSet<String> stopSet0 = HashSet<String>::newInstance(stopWords0.begin(), stopWords0.end());
    HashSet<String> stopSet1 = HashSet<String>::newInstance(stopWords1.begin(), stopWords1.end());
    reader = newLucene<StringReader>(buf.str());
    StopFilterPtr stpf0 = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopSet0); // first part of the set
    stpf0->setEnablePositionIncrements(true);
    StopFilterPtr stpf01 = newLucene<StopFilter>(false, stpf0, stopSet1); // two stop filters concatenated!
    doTestStopPositons(stpf01, true);
}
