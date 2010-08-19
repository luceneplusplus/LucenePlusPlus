/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseTokenStreamFixture.h"
#include "TestUtils.h"
#include "StringReader.h"
#include "TokenStream.h"
#include "StopFilter.h"
#include "WhitespaceTokenizer.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(StopFilterTest, BaseTokenStreamFixture)

static void doTestStopPositons(StopFilterPtr stpf, bool enableIcrements)
{
    stpf->setEnablePositionIncrements(enableIcrements);
    TermAttributePtr termAtt = stpf->getAttribute<TermAttribute>();
    PositionIncrementAttributePtr posIncrAtt = stpf->getAttribute<PositionIncrementAttribute>();
    for (int32_t i = 0; i < 20; i += 3)
    {
        BOOST_CHECK(stpf->incrementToken());
        String w = intToEnglish(i);
        BOOST_CHECK_EQUAL(w, termAtt->term());
        BOOST_CHECK_EQUAL(enableIcrements ? (i == 0 ? 1 : 3) : 1, posIncrAtt->getPositionIncrement());
    }
    BOOST_CHECK(!stpf->incrementToken());
}

BOOST_AUTO_TEST_CASE(testExactCase)
{
    StringReaderPtr reader = newLucene<StringReader>(L"Now is The Time");
    HashSet<String> stopWords = HashSet<String>::newInstance();
    stopWords.add(L"is");
    stopWords.add(L"the");
    stopWords.add(L"Time");
    TokenStreamPtr stream = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopWords, false);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    BOOST_CHECK(stream->incrementToken());
    BOOST_CHECK_EQUAL(L"Now", termAtt->term());
    BOOST_CHECK(stream->incrementToken());
    BOOST_CHECK_EQUAL(L"The", termAtt->term());
    BOOST_CHECK(!stream->incrementToken());
}

BOOST_AUTO_TEST_CASE(testIgnoreCase)
{
    StringReaderPtr reader = newLucene<StringReader>(L"Now is The Time");
    HashSet<String> stopWords = HashSet<String>::newInstance();
    stopWords.add(L"is");
    stopWords.add(L"the");
    stopWords.add(L"Time");
    TokenStreamPtr stream = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopWords, true);
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    BOOST_CHECK(stream->incrementToken());
    BOOST_CHECK_EQUAL(L"Now", termAtt->term());
    BOOST_CHECK(!stream->incrementToken());
}

BOOST_AUTO_TEST_CASE(testStopPositons)
{
    StringStream buf;
    Collection<String> stopWords = Collection<String>::newInstance();
    for (int32_t i = 0; i < 20; ++i)
    {
        String w = intToEnglish(i);
        buf << w << L" ";
        if (i % 3 != 0)
            stopWords.add(w);
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
    for (int32_t i = 0; i < stopWords.size(); ++i)
    {
        if (i % 2 == 0)
            stopWords0.add(stopWords[i]);
        else
            stopWords1.add(stopWords[i]);
    }
    HashSet<String> stopSet0 = HashSet<String>::newInstance(stopWords0.begin(), stopWords0.end());
    HashSet<String> stopSet1 = HashSet<String>::newInstance(stopWords1.begin(), stopWords1.end());
    reader = newLucene<StringReader>(buf.str());
    StopFilterPtr stpf0 = newLucene<StopFilter>(false, newLucene<WhitespaceTokenizer>(reader), stopSet0); // first part of the set
    stpf0->setEnablePositionIncrements(true);
    StopFilterPtr stpf01 = newLucene<StopFilter>(false, stpf0, stopSet1); // two stop filters concatenated!
    doTestStopPositons(stpf01, true);
}

BOOST_AUTO_TEST_SUITE_END()
