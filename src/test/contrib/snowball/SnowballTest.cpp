/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "SnowballAnalyzer.h"
#include "StopAnalyzer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture SnowballTest;

TEST_F(SnowballTest, testEnglish) {
    AnalyzerPtr a = newLucene<SnowballAnalyzer>(LuceneVersion::LUCENE_CURRENT, L"english");
    checkAnalyzesTo(a, L"he abhorred accents", newCollection<String>(L"he", L"abhor", L"accent"));
}

TEST_F(SnowballTest, testStopwords) {
    AnalyzerPtr a = newLucene<SnowballAnalyzer>(LuceneVersion::LUCENE_CURRENT, L"english", StopAnalyzer::ENGLISH_STOP_WORDS_SET());
    checkAnalyzesTo(a, L"the quick brown fox jumped", newCollection<String>(L"quick", L"brown", L"fox", L"jump"));
}

TEST_F(SnowballTest, testReusableTokenStream) {
    AnalyzerPtr a = newLucene<SnowballAnalyzer>(LuceneVersion::LUCENE_CURRENT, L"english");

    checkAnalyzesToReuse(a, L"he abhorred accents", newCollection<String>(L"he", L"abhor", L"accent"));
    checkAnalyzesToReuse(a, L"she abhorred him", newCollection<String>(L"she", L"abhor", L"him"));
}
