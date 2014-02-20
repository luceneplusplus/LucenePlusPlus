/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef BaseTokenStreamFixture PerFieldAnalzyerWrapperTest;

TEST_F(PerFieldAnalzyerWrapperTest, testPerField) {
    String text = L"Qwerty";
    PerFieldAnalyzerWrapperPtr analyzer = newLucene<PerFieldAnalyzerWrapper>(newLucene<WhitespaceAnalyzer>());
    analyzer->addAnalyzer(L"special", newLucene<SimpleAnalyzer>());

    TokenStreamPtr tokenStream = analyzer->tokenStream(L"field", newLucene<StringReader>(text));
    TermAttributePtr termAtt = tokenStream->getAttribute<TermAttribute>();

    EXPECT_TRUE(tokenStream->incrementToken());
    EXPECT_EQ(L"Qwerty", termAtt->term());

    tokenStream = analyzer->tokenStream(L"special", newLucene<StringReader>(text));
    termAtt = tokenStream->getAttribute<TermAttribute>();
    EXPECT_TRUE(tokenStream->incrementToken());
    EXPECT_EQ(L"qwerty", termAtt->term());
}
