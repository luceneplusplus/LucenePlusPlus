/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "ChineseTokenizer.h"
#include "ChineseFilter.h"
#include "ChineseAnalyzer.h"
#include "StringReader.h"
#include "OffsetAttribute.h"
#include "WhitespaceTokenizer.h"

using namespace Lucene;

/// Analyzer that just uses ChineseTokenizer, not ChineseFilter.
/// Convenience to show the behaviour of the tokenizer
class JustChineseTokenizerAnalyzer : public Analyzer {
public:
    virtual ~JustChineseTokenizerAnalyzer() {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<ChineseTokenizer>(reader);
    }
};

/// Analyzer that just uses ChineseFilter, not ChineseTokenizer.
/// Convenience to show the behavior of the filter.
class JustChineseFilterAnalyzer : public Analyzer {
public:
    virtual ~JustChineseFilterAnalyzer() {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<ChineseFilter>(newLucene<WhitespaceTokenizer>(reader));
    }
};

typedef BaseTokenStreamFixture ChineseTokenizerTest;

TEST_F(ChineseTokenizerTest, testOtherLetterOffset) {
    const uint8_t token[] = {0x61, 0xe5, 0xa4, 0xa9, 0x62};
    ChineseTokenizerPtr tokenizer = newLucene<ChineseTokenizer>(newLucene<StringReader>(UTF8_TO_STRING(token)));

    int32_t correctStartOffset = 0;
    int32_t correctEndOffset = 1;
    OffsetAttributePtr offsetAtt = tokenizer->getAttribute<OffsetAttribute>();
    while (tokenizer->incrementToken()) {
        EXPECT_EQ(correctStartOffset, offsetAtt->startOffset());
        EXPECT_EQ(correctEndOffset, offsetAtt->endOffset());
        ++correctStartOffset;
        ++correctEndOffset;
    }
}

TEST_F(ChineseTokenizerTest, testReusableTokenStream1) {
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();

    const uint8_t input[] = {0xe4, 0xb8, 0xad, 0xe5, 0x8d, 0x8e, 0xe4, 0xba, 0xba, 0xe6, 0xb0,
                             0x91, 0xe5, 0x85, 0xb1, 0xe5, 0x92, 0x8c, 0xe5, 0x9b, 0xbd
                            };

    const uint8_t token1[] = {0xe4, 0xb8, 0xad};
    const uint8_t token2[] = {0xe5, 0x8d, 0x8e};
    const uint8_t token3[] = {0xe4, 0xba, 0xba};
    const uint8_t token4[] = {0xe6, 0xb0, 0x91};
    const uint8_t token5[] = {0xe5, 0x85, 0xb1};
    const uint8_t token6[] = {0xe5, 0x92, 0x8c};
    const uint8_t token7[] = {0xe5, 0x9b, 0xbd};

    checkAnalyzesToReuse(a, UTF8_TO_STRING(input),
                         newCollection<String>(
                             UTF8_TO_STRING(token1),
                             UTF8_TO_STRING(token2),
                             UTF8_TO_STRING(token3),
                             UTF8_TO_STRING(token4),
                             UTF8_TO_STRING(token5),
                             UTF8_TO_STRING(token6),
                             UTF8_TO_STRING(token7)
                         ),
                         newCollection<int32_t>(0, 1, 2, 3, 4, 5, 6),
                         newCollection<int32_t>(1, 2, 3, 4, 5, 6, 7));
}

TEST_F(ChineseTokenizerTest, testReusableTokenStream2) {
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();

    const uint8_t input[] = {0xe5, 0x8c, 0x97, 0xe4, 0xba, 0xac, 0xe5, 0xb8, 0x82};

    const uint8_t token1[] = {0xe5, 0x8c, 0x97};
    const uint8_t token2[] = {0xe4, 0xba, 0xac};
    const uint8_t token3[] = {0xe5, 0xb8, 0x82};

    checkAnalyzesToReuse(a, UTF8_TO_STRING(input),
                         newCollection<String>(
                             UTF8_TO_STRING(token1),
                             UTF8_TO_STRING(token2),
                             UTF8_TO_STRING(token3)
                         ),
                         newCollection<int32_t>(0, 1, 2),
                         newCollection<int32_t>(1, 2, 3));
}

/// ChineseTokenizer tokenizes numbers as one token, but they are filtered by ChineseFilter
TEST_F(ChineseTokenizerTest, testNumerics) {
    AnalyzerPtr justTokenizer = newLucene<JustChineseTokenizerAnalyzer>();

    const uint8_t input[] = {0xe4, 0xb8, 0xad, 0x31, 0x32, 0x33, 0x34};
    const uint8_t token1[] = {0xe4, 0xb8, 0xad};

    checkAnalyzesTo(justTokenizer, UTF8_TO_STRING(input), newCollection<String>(UTF8_TO_STRING(token1), L"1234"));

    // in this case the ChineseAnalyzer (which applies ChineseFilter) will remove the numeric token.
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();
    checkAnalyzesTo(a, UTF8_TO_STRING(input), newCollection<String>(UTF8_TO_STRING(token1)));
}

/// ChineseTokenizer tokenizes english similar to SimpleAnalyzer.
/// It will lowercase terms automatically.
///
/// ChineseFilter has an english stopword list, it also removes any single character tokens.
/// The stopword list is case-sensitive.
TEST_F(ChineseTokenizerTest, testEnglish) {
    AnalyzerPtr chinese = newLucene<ChineseAnalyzer>();
    checkAnalyzesTo(chinese, L"This is a Test. b c d", newCollection<String>(L"test"));

    AnalyzerPtr justTokenizer = newLucene<JustChineseTokenizerAnalyzer>();
    checkAnalyzesTo(justTokenizer, L"This is a Test. b c d", newCollection<String>(L"this", L"is", L"a", L"test", L"b", L"c", L"d"));

    AnalyzerPtr justFilter = newLucene<JustChineseFilterAnalyzer>();
    checkAnalyzesTo(justFilter, L"This is a Test. b c d", newCollection<String>(L"This", L"Test."));
}
