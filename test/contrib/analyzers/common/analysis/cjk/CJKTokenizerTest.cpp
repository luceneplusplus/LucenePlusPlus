/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseTokenStreamFixture.h"
#include "CJKTokenizer.h"
#include "CJKAnalyzer.h"

using namespace Lucene;

class CJKTokenizerFixture : public BaseTokenStreamFixture
{
public:
	virtual ~CJKTokenizerFixture()
	{
	}

public:
    struct TestToken
	{
		TestToken(const String& termText = L"", int32_t start = 0, int32_t end = 0, int32_t type = 0)
		{
			this->termText = termText;
			this->start = start;
			this->end = end;
			this->type = CJKTokenizer::TOKEN_TYPE_NAMES[type];
		}
		
        String termText;
        int32_t start;
        int32_t end;
        String type;
	};
    
    void checkCJKToken(const String& str, Collection<TestToken> out_tokens)
    {
        AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        Collection<String> terms = Collection<String>::newInstance(out_tokens.size());
        Collection<int32_t> startOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<int32_t> endOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<String> types = Collection<String>::newInstance(out_tokens.size());
        for (int32_t i = 0; i < out_tokens.size(); ++i)
        {
            terms[i] = out_tokens[i].termText;
            startOffsets[i] = out_tokens[i].start;
            endOffsets[i] = out_tokens[i].end;
            types[i] = out_tokens[i].type;
        }
        checkAnalyzesTo(analyzer, str, terms, startOffsets, endOffsets, types, Collection<int32_t>());
    }
    
    void checkCJKTokenReusable(AnalyzerPtr analyzer, const String& str, Collection<TestToken> out_tokens)
    {
        Collection<String> terms = Collection<String>::newInstance(out_tokens.size());
        Collection<int32_t> startOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<int32_t> endOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<String> types = Collection<String>::newInstance(out_tokens.size());
        for (int32_t i = 0; i < out_tokens.size(); ++i)
        {
            terms[i] = out_tokens[i].termText;
            startOffsets[i] = out_tokens[i].start;
            endOffsets[i] = out_tokens[i].end;
            types[i] = out_tokens[i].type;
        }
        checkAnalyzesToReuse(analyzer, str, terms, startOffsets, endOffsets, types, Collection<int32_t>());
    }
};

BOOST_FIXTURE_TEST_SUITE(CJKTokenizerTest, CJKTokenizerFixture)

BOOST_AUTO_TEST_CASE(testJa1)
{
    String str = L"\u4e00\u4e8c\u4e09\u56db\u4e94\u516d\u4e03\u516b\u4e5d\u5341";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u4e00\u4e8c", 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e8c\u4e09", 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e09\u56db", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u56db\u4e94", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u4e94\u516d", 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u516d\u4e03", 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e03\u516b", 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u516b\u4e5d", 7, 9, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e5d\u5341", 8,10, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

BOOST_AUTO_TEST_CASE(testJa2)
{
    String str = L"\u4e00 \u4e8c\u4e09\u56db \u4e94\u516d\u4e03\u516b\u4e5d \u5341";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u4e00", 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u4e8c\u4e09", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e09\u56db", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e94\u516d", 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u516d\u4e03", 7, 9, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u4e03\u516b", 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u516b\u4e5d", 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u5341", 12,13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

BOOST_AUTO_TEST_CASE(testC)
{
    String str = L"abc defgh ijklmn opqrstu vwxy z";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"abc", 0, 3, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"defgh", 4, 9, CJKTokenizer::SINGLE_TOKEN_TYPE),
        TestToken(L"ijklmn", 10, 16, CJKTokenizer::SINGLE_TOKEN_TYPE),
        TestToken(L"opqrstu", 17, 24, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"vwxy", 25, 29, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"z", 30, 31, CJKTokenizer::SINGLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

BOOST_AUTO_TEST_CASE(testMix)
{
    String str = L"\u3042\u3044\u3046\u3048\u304aabc\u304b\u304d\u304f\u3051\u3053";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u3042\u3044", 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u3044\u3046", 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3046\u3048", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3048\u304a", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"abc", 5, 8, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u304b\u304d", 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304d\u304f", 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304f\u3051", 10,12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3051\u3053", 11,13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

BOOST_AUTO_TEST_CASE(testMix2)
{
    String str = L"\u3042\u3044\u3046\u3048\u304aab\u3093c\u304b\u304d\u304f\u3051";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u3042\u3044", 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u3044\u3046", 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3046\u3048", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3048\u304a", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"ab", 5, 7, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u3093", 7, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"c", 8, 9, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u304b\u304d", 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304d\u304f", 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304f\u3051", 11,13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

BOOST_AUTO_TEST_CASE(testSingleChar)
{
    String str = L"\u4e00";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u4e00", 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(str, out_tokens);
}

/// Full-width text is normalized to half-width 
BOOST_AUTO_TEST_CASE(testFullWidth)
{
    const uint8_t str[] = {0xef, 0xbc, 0xb4, 0xef, 0xbd, 0x85, 0xef, 0xbd, 0x93, 0xef, 0xbd, 0x94,
                           0x20, 0xef, 0xbc, 0x91, 0xef, 0xbc, 0x92, 0xef, 0xbc, 0x93, 0xef, 0xbc, 0x94};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"1234", 5, 9, CJKTokenizer::SINGLE_TOKEN_TYPE)
    );
    checkCJKToken(StringUtils::toUnicode(str, sizeof(str) / sizeof(str[0])), out_tokens);
}

/// Non-english text (not just CJK) is treated the same as CJK: C1C2 C2C3 
BOOST_AUTO_TEST_CASE(testNonIdeographic)
{
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0x20, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8, 0xd8, 0xb1,
                           0xd8, 0xaa, 0x20, 0xd9, 0x85, 0xd9, 0x88, 0xd9, 0x8a, 0xd8, 0xb1};
    
    const uint8_t token1[] = {0xd8, 0xb1, 0xd9, 0x88};
    const uint8_t token2[] = {0xd9, 0x88, 0xd8, 0xa8};
    const uint8_t token3[] = {0xd8, 0xa8, 0xd8, 0xb1};
    const uint8_t token4[] = {0xd8, 0xb1, 0xd8, 0xaa};
    const uint8_t token5[] = {0xd9, 0x85, 0xd9, 0x88};
    const uint8_t token6[] = {0xd9, 0x88, 0xd9, 0x8a};
    const uint8_t token7[] = {0xd9, 0x8a, 0xd8, 0xb1};
    
    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u4e00", 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token2, sizeof(token2) / sizeof(token2[0])), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token3, sizeof(token3) / sizeof(token3[0])), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token4, sizeof(token4) / sizeof(token4[0])), 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token5, sizeof(token5) / sizeof(token5[0])), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token6, sizeof(token6) / sizeof(token6[0])), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token7, sizeof(token7) / sizeof(token7[0])), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(StringUtils::toUnicode(str, sizeof(str) / sizeof(str[0])), out_tokens);
}

/// Non-english text with non-letters (non-spacing marks,etc) is treated as C1C2 C2C3, 
/// except for words are split around non-letters.
BOOST_AUTO_TEST_CASE(testNonIdeographicNonLetter)
{
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0x20, 0xd8, 0xb1, 0xd9, 0x8f, 0xd9, 0x88, 0xd8, 0xa8,
                           0xd8, 0xb1, 0xd8, 0xaa, 0x20, 0xd9, 0x85, 0xd9, 0x88, 0xd9, 0x8a, 0xd8, 0xb1};
    
    const uint8_t token1[] = {0xd8, 0xb1};
    const uint8_t token2[] = {0xd9, 0x88, 0xd8, 0xa8};
    const uint8_t token3[] = {0xd8, 0xa8, 0xd8, 0xb1};
    const uint8_t token4[] = {0xd8, 0xb1, 0xd8, 0xaa};
    const uint8_t token5[] = {0xd9, 0x85, 0xd9, 0x88};
    const uint8_t token6[] = {0xd9, 0x88, 0xd9, 0x8a};
    const uint8_t token7[] = {0xd9, 0x8a, 0xd8, 0xb1};
    
    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u4e00", 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 2, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token2, sizeof(token2) / sizeof(token2[0])), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token3, sizeof(token3) / sizeof(token3[0])), 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token4, sizeof(token4) / sizeof(token4[0])), 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token5, sizeof(token5) / sizeof(token5[0])), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token6, sizeof(token6) / sizeof(token6[0])), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(StringUtils::toUnicode(token7, sizeof(token7) / sizeof(token7[0])), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKToken(StringUtils::toUnicode(str, sizeof(str) / sizeof(str[0])), out_tokens);
}

BOOST_AUTO_TEST_CASE(testTokenStream)
{
    AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(analyzer, L"\u4e00\u4e01\u4e02", newCollection<String>(L"\u4e00\u4e01", L"\u4e01\u4e02"));
}

BOOST_AUTO_TEST_CASE(testReusableTokenStream)
{
    AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    String str = L"\u3042\u3044\u3046\u3048\u304aabc\u304b\u304d\u304f\u3051\u3053";

    Collection<TestToken> out_tokens = newCollection<TestToken>(
        TestToken(L"\u3042\u3044", 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u3044\u3046", 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3046\u3048", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3048\u304a", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"abc", 5, 8, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u304b\u304d", 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304d\u304f", 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304f\u3051", 10,12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3051\u3053", 11,13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKTokenReusable(analyzer, str, out_tokens);
    
    str = L"\u3042\u3044\u3046\u3048\u304aab\u3093c\u304b\u304d\u304f\u3051";

    Collection<TestToken> out_tokens2 = newCollection<TestToken>(
        TestToken(L"\u3042\u3044", 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"\u3044\u3046", 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3046\u3048", 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u3048\u304a", 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"ab", 5, 7, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u3093", 7, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE), 
        TestToken(L"c", 8, 9, CJKTokenizer::SINGLE_TOKEN_TYPE), 
        TestToken(L"\u304b\u304d", 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304d\u304f", 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
        TestToken(L"\u304f\u3051", 11,13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
    );
    checkCJKTokenReusable(analyzer, str, out_tokens2);
}

BOOST_AUTO_TEST_CASE(testFinalOffset)
{
    const uint8_t token1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    checkCJKToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 
                  newCollection<TestToken>(TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
    const uint8_t token2[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x20, 0x20, 0x20};
    checkCJKToken(StringUtils::toUnicode(token2, sizeof(token2) / sizeof(token2[0])), 
                  newCollection<TestToken>(TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
    checkCJKToken(L"test", newCollection<TestToken>(TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    checkCJKToken(L"test   ", newCollection<TestToken>(TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    const uint8_t token3[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x74, 0x65, 0x73, 0x74};
    checkCJKToken(StringUtils::toUnicode(token3, sizeof(token3) / sizeof(token3[0])), 
                  newCollection<TestToken>(
                  TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                  TestToken(L"test", 2, 6, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    const uint8_t token4[] = {0x74, 0x65, 0x73, 0x74, 0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x20, 0x20, 0x20, 0x20};
    checkCJKToken(StringUtils::toUnicode(token4, sizeof(token4) / sizeof(token4[0])), 
                  newCollection<TestToken>(
                  TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE),
                  TestToken(StringUtils::toUnicode(token1, sizeof(token1) / sizeof(token1[0])), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
}

BOOST_AUTO_TEST_SUITE_END()
