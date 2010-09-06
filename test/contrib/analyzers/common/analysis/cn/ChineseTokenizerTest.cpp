/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
class JustChineseTokenizerAnalyzer : public Analyzer
{
public:
    virtual ~JustChineseTokenizerAnalyzer()
    {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<ChineseTokenizer>(reader);
    }
};

/// Analyzer that just uses ChineseFilter, not ChineseTokenizer.
/// Convenience to show the behavior of the filter.
class JustChineseFilterAnalyzer : public Analyzer
{
public:
    virtual ~JustChineseFilterAnalyzer()
    {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<ChineseFilter>(newLucene<WhitespaceTokenizer>(reader));
    }
};

BOOST_FIXTURE_TEST_SUITE(ChineseTokenizerTest, BaseTokenStreamFixture)

BOOST_AUTO_TEST_CASE(testOtherLetterOffset)
{
    const uint8_t token[] = {0x61, 0xe5, 0xa4, 0xa9, 0x62};
    ChineseTokenizerPtr tokenizer = newLucene<ChineseTokenizer>(newLucene<StringReader>(StringUtils::toUnicode(token, sizeof(token) / sizeof(token[0]))));

    int32_t correctStartOffset = 0;
    int32_t correctEndOffset = 1;
    OffsetAttributePtr offsetAtt = tokenizer->getAttribute<OffsetAttribute>();
    while (tokenizer->incrementToken())
    {
        BOOST_CHECK_EQUAL(correctStartOffset, offsetAtt->startOffset());
        BOOST_CHECK_EQUAL(correctEndOffset, offsetAtt->endOffset());
        ++correctStartOffset;
        ++correctEndOffset;
    }
}

BOOST_AUTO_TEST_CASE(testReusableTokenStream1)
{
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();
    
    const uint8_t input[] = {0xe4, 0xb8, 0xad, 0xe5, 0x8d, 0x8e, 0xe4, 0xba, 0xba, 0xe6, 0xb0,
                             0x91, 0xe5, 0x85, 0xb1, 0xe5, 0x92, 0x8c, 0xe5, 0x9b, 0xbd};
    
    const uint8_t token1[] = {0xe4, 0xb8, 0xad};
    const uint8_t token2[] = {0xe5, 0x8d, 0x8e};
    const uint8_t token3[] = {0xe4, 0xba, 0xba};
    const uint8_t token4[] = {0xe6, 0xb0, 0x91};
    const uint8_t token5[] = {0xe5, 0x85, 0xb1};
    const uint8_t token6[] = {0xe5, 0x92, 0x8c};
    const uint8_t token7[] = {0xe5, 0x9b, 0xbd};
    
    checkAnalyzesToReuse(a, StringUtils::toUnicode(input, sizeof(input) / sizeof(input[0])),
                            newCollection<String>(
                                StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])),
                                StringUtils::toUnicode(token2, sizeof(token2) / sizeof(token2[0])),
                                StringUtils::toUnicode(token3, sizeof(token3) / sizeof(token3[0])),
                                StringUtils::toUnicode(token4, sizeof(token4) / sizeof(token4[0])),
                                StringUtils::toUnicode(token5, sizeof(token5) / sizeof(token5[0])),
                                StringUtils::toUnicode(token6, sizeof(token6) / sizeof(token6[0])),
                                StringUtils::toUnicode(token7, sizeof(token7) / sizeof(token7[0]))
                            ),
                            newCollection<int32_t>(0, 1, 2, 3, 4, 5, 6),
                            newCollection<int32_t>(1, 2, 3, 4, 5, 6, 7));
}

BOOST_AUTO_TEST_CASE(testReusableTokenStream2)
{
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();
    
    const uint8_t input[] = {0xe5, 0x8c, 0x97, 0xe4, 0xba, 0xac, 0xe5, 0xb8, 0x82};

    const uint8_t token1[] = {0xe5, 0x8c, 0x97};
    const uint8_t token2[] = {0xe4, 0xba, 0xac};
    const uint8_t token3[] = {0xe5, 0xb8, 0x82};
    
    checkAnalyzesToReuse(a, StringUtils::toUnicode(input, sizeof(input) / sizeof(input[0])),
                            newCollection<String>(
                                StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])),
                                StringUtils::toUnicode(token2, sizeof(token2) / sizeof(token2[0])),
                                StringUtils::toUnicode(token3, sizeof(token3) / sizeof(token3[0]))
                            ),
                            newCollection<int32_t>(0, 1, 2),
                            newCollection<int32_t>(1, 2, 3));
}

/// ChineseTokenizer tokenizes numbers as one token, but they are filtered by ChineseFilter
BOOST_AUTO_TEST_CASE(testNumerics)
{
    AnalyzerPtr justTokenizer = newLucene<JustChineseTokenizerAnalyzer>();
    
    const uint8_t input[] = {0xe4, 0xb8, 0xad, 0x31, 0x32, 0x33, 0x34};
    const uint8_t token1[] = {0xe4, 0xb8, 0xad};
    
    checkAnalyzesTo(justTokenizer, StringUtils::toUnicode(input, sizeof(input) / sizeof(input[0])),
                    newCollection<String>(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), L"1234"));

    // in this case the ChineseAnalyzer (which applies ChineseFilter) will remove the numeric token.
    AnalyzerPtr a = newLucene<ChineseAnalyzer>();
    checkAnalyzesTo(a, StringUtils::toUnicode(input, sizeof(input) / sizeof(input[0])), newCollection<String>(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0]))));
}

/// ChineseTokenizer tokenizes english similar to SimpleAnalyzer.
/// It will lowercase terms automatically.
///
/// ChineseFilter has an english stopword list, it also removes any single character tokens.
/// The stopword list is case-sensitive.
BOOST_AUTO_TEST_CASE(testEnglish)
{
    AnalyzerPtr chinese = newLucene<ChineseAnalyzer>();
    checkAnalyzesTo(chinese, L"This is a Test. b c d", newCollection<String>(L"test"));

    AnalyzerPtr justTokenizer = newLucene<JustChineseTokenizerAnalyzer>();
    checkAnalyzesTo(justTokenizer, L"This is a Test. b c d", newCollection<String>(L"this", L"is", L"a", L"test", L"b", L"c", L"d"));

    AnalyzerPtr justFilter = newLucene<JustChineseFilterAnalyzer>();
    checkAnalyzesTo(justFilter, L"This is a Test. b c d", newCollection<String>(L"This", L"Test."));
}

BOOST_AUTO_TEST_SUITE_END()
