/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "StopAnalyzer.h"
#include "StringReader.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"

using namespace Lucene;

class StopAnalyzerTest : public BaseTokenStreamFixture {
public:
    StopAnalyzerTest() {
        stop = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        inValidTokens = HashSet<String>::newInstance();
        for (HashSet<String>::iterator word = StopAnalyzer::ENGLISH_STOP_WORDS_SET().begin(); word != StopAnalyzer::ENGLISH_STOP_WORDS_SET().end(); ++word) {
            inValidTokens.add(*word);
        }
    }

    virtual ~StopAnalyzerTest() {
    }

protected:
    StopAnalyzerPtr stop;
    HashSet<String> inValidTokens;
};

TEST_F(StopAnalyzerTest, testDefaults) {
    EXPECT_TRUE(stop);
    StringReaderPtr reader = newLucene<StringReader>(L"This is a test of the english stop analyzer");
    TokenStreamPtr stream = stop->tokenStream(L"test", reader);
    EXPECT_TRUE(stream);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();

    while (stream->incrementToken()) {
        EXPECT_TRUE(!inValidTokens.contains(termAtt->term()));
    }
}

TEST_F(StopAnalyzerTest, testStopList) {
    HashSet<String> stopWordsSet = HashSet<String>::newInstance();
    stopWordsSet.add(L"good");
    stopWordsSet.add(L"test");
    stopWordsSet.add(L"analyzer");
    StopAnalyzerPtr newStop = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_24, stopWordsSet);
    StringReaderPtr reader = newLucene<StringReader>(L"This is a good test of the english stop analyzer");
    TokenStreamPtr stream = newStop->tokenStream(L"test", reader);
    EXPECT_TRUE(stream);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    PositionIncrementAttributePtr posIncrAtt = stream->addAttribute<PositionIncrementAttribute>();

    while (stream->incrementToken()) {
        String text = termAtt->term();
        EXPECT_TRUE(!stopWordsSet.contains(text));
        EXPECT_EQ(1, posIncrAtt->getPositionIncrement()); // in 2.4 stop tokenizer does not apply increments.
    }
}

TEST_F(StopAnalyzerTest, testStopListPositions) {
    HashSet<String> stopWordsSet = HashSet<String>::newInstance();
    stopWordsSet.add(L"good");
    stopWordsSet.add(L"test");
    stopWordsSet.add(L"analyzer");
    StopAnalyzerPtr newStop = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT, stopWordsSet);
    StringReaderPtr reader = newLucene<StringReader>(L"This is a good test of the english stop analyzer with positions");
    Collection<int32_t> expectedIncr = newCollection<int32_t>(1, 1, 1, 3, 1, 1, 1, 2, 1);
    TokenStreamPtr stream = newStop->tokenStream(L"test", reader);
    EXPECT_TRUE(stream);
    int32_t i = 0;
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    PositionIncrementAttributePtr posIncrAtt = stream->addAttribute<PositionIncrementAttribute>();

    while (stream->incrementToken()) {
        String text = termAtt->term();
        EXPECT_TRUE(!stopWordsSet.contains(text));
        EXPECT_EQ(expectedIncr[i++], posIncrAtt->getPositionIncrement());
    }
}
