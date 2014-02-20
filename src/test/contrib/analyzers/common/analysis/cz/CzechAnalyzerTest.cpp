/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "CzechAnalyzer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture CzechAnalyzerTest;

TEST_F(CzechAnalyzerTest, testStopWord) {
    CzechAnalyzerPtr analyzer = newLucene<CzechAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(analyzer, L"Pokud mluvime o volnem", newCollection<String>(L"mluvime", L"volnem"));
}

TEST_F(CzechAnalyzerTest, testReusableTokenStream1) {
    CzechAnalyzerPtr analyzer = newLucene<CzechAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesToReuse(analyzer, L"Pokud mluvime o volnem", newCollection<String>(L"mluvime", L"volnem"));
}

TEST_F(CzechAnalyzerTest, testReusableTokenStream2) {
    CzechAnalyzerPtr analyzer = newLucene<CzechAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t input[] = {0xc4, 0x8c, 0x65, 0x73, 0x6b, 0xc3, 0xa1, 0x20, 0x52,
                             0x65, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x6b, 0x61
                            };
    const uint8_t token1[] = {0xc4, 0x8d, 0x65, 0x73, 0x6b, 0xc3, 0xa1};
    const uint8_t token2[] = {0x72, 0x65, 0x70, 0x75, 0x62, 0x6c, 0x69, 0x6b, 0x61};
    checkAnalyzesToReuse(analyzer, UTF8_TO_STRING(input), newCollection<String>(UTF8_TO_STRING(token1), UTF8_TO_STRING(token2)));
}
