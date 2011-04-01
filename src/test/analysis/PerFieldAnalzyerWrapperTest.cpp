/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "PerFieldAnalyzerWrapper.h"
#include "WhitespaceAnalyzer.h"
#include "SimpleAnalyzer.h"
#include "TokenStream.h"
#include "StringReader.h"
#include "TermAttribute.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(PerFieldAnalzyerWrapperTest, BaseTokenStreamFixture)

BOOST_AUTO_TEST_CASE(testPerField)
{
    String text = L"Qwerty";
    PerFieldAnalyzerWrapperPtr analyzer = newLucene<PerFieldAnalyzerWrapper>(newLucene<WhitespaceAnalyzer>());
    analyzer->addAnalyzer(L"special", newLucene<SimpleAnalyzer>());

    TokenStreamPtr tokenStream = analyzer->tokenStream(L"field", newLucene<StringReader>(text));
    TermAttributePtr termAtt = tokenStream->getAttribute<TermAttribute>();

    BOOST_CHECK(tokenStream->incrementToken());
    BOOST_CHECK_EQUAL(L"Qwerty", termAtt->term());

    tokenStream = analyzer->tokenStream(L"special", newLucene<StringReader>(text));
    termAtt = tokenStream->getAttribute<TermAttribute>();
    BOOST_CHECK(tokenStream->incrementToken());
    BOOST_CHECK_EQUAL(L"qwerty", termAtt->term());
}

BOOST_AUTO_TEST_SUITE_END()
