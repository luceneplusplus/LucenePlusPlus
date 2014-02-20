/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "CJKTokenizer.h"
#include "CJKAnalyzer.h"

using namespace Lucene;

class CJKTokenizerTest : public BaseTokenStreamFixture {
public:
    virtual ~CJKTokenizerTest() {
    }

public:
    struct TestToken {
        TestToken(const String& termText = L"", int32_t start = 0, int32_t end = 0, int32_t type = 0) {
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

    void checkCJKToken(const String& str, Collection<TestToken> out_tokens) {
        AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        Collection<String> terms = Collection<String>::newInstance(out_tokens.size());
        Collection<int32_t> startOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<int32_t> endOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<String> types = Collection<String>::newInstance(out_tokens.size());
        for (int32_t i = 0; i < out_tokens.size(); ++i) {
            terms[i] = out_tokens[i].termText;
            startOffsets[i] = out_tokens[i].start;
            endOffsets[i] = out_tokens[i].end;
            types[i] = out_tokens[i].type;
        }
        checkAnalyzesTo(analyzer, str, terms, startOffsets, endOffsets, types, Collection<int32_t>());
    }

    void checkCJKTokenReusable(const AnalyzerPtr& analyzer, const String& str, Collection<TestToken> out_tokens) {
        Collection<String> terms = Collection<String>::newInstance(out_tokens.size());
        Collection<int32_t> startOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<int32_t> endOffsets = Collection<int32_t>::newInstance(out_tokens.size());
        Collection<String> types = Collection<String>::newInstance(out_tokens.size());
        for (int32_t i = 0; i < out_tokens.size(); ++i) {
            terms[i] = out_tokens[i].termText;
            startOffsets[i] = out_tokens[i].start;
            endOffsets[i] = out_tokens[i].end;
            types[i] = out_tokens[i].type;
        }
        checkAnalyzesToReuse(analyzer, str, terms, startOffsets, endOffsets, types, Collection<int32_t>());
    }
};

TEST_F(CJKTokenizerTest, testJa1) {
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0xe4, 0xba, 0x8c, 0xe4, 0xb8, 0x89, 0xe5, 0x9b, 0x9b,
                           0xe4, 0xba, 0x94, 0xe5, 0x85, 0xad, 0xe4, 0xb8, 0x83, 0xe5, 0x85, 0xab,
                           0xe4, 0xb9, 0x9d, 0xe5, 0x8d, 0x81
                          };

    const uint8_t token1[] = {0xe4, 0xb8, 0x80, 0xe4, 0xba, 0x8c};
    const uint8_t token2[] = {0xe4, 0xba, 0x8c, 0xe4, 0xb8, 0x89};
    const uint8_t token3[] = {0xe4, 0xb8, 0x89, 0xe5, 0x9b, 0x9b};
    const uint8_t token4[] = {0xe5, 0x9b, 0x9b, 0xe4, 0xba, 0x94};
    const uint8_t token5[] = {0xe4, 0xba, 0x94, 0xe5, 0x85, 0xad};
    const uint8_t token6[] = {0xe5, 0x85, 0xad, 0xe4, 0xb8, 0x83};
    const uint8_t token7[] = {0xe4, 0xb8, 0x83, 0xe5, 0x85, 0xab};
    const uint8_t token8[] = {0xe5, 0x85, 0xab, 0xe4, 0xb9, 0x9d};
    const uint8_t token9[] = {0xe4, 0xb9, 0x9d, 0xe5, 0x8d, 0x81};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token5), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token7), 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 7, 9, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token9), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

TEST_F(CJKTokenizerTest, testJa2) {
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0x20, 0xe4, 0xba, 0x8c, 0xe4, 0xb8, 0x89, 0xe5,
                           0x9b, 0x9b, 0x20, 0xe4, 0xba, 0x94, 0xe5, 0x85, 0xad, 0xe4, 0xb8,
                           0x83, 0xe5, 0x85, 0xab, 0xe4, 0xb9, 0x9d, 0x20, 0xe5, 0x8d, 0x81
                          };

    const uint8_t token1[] = {0xe4, 0xb8, 0x80};
    const uint8_t token2[] = {0xe4, 0xba, 0x8c, 0xe4, 0xb8, 0x89};
    const uint8_t token3[] = {0xe4, 0xb8, 0x89, 0xe5, 0x9b, 0x9b};
    const uint8_t token4[] = {0xe4, 0xba, 0x94, 0xe5, 0x85, 0xad};
    const uint8_t token5[] = {0xe5, 0x85, 0xad, 0xe4, 0xb8, 0x83};
    const uint8_t token6[] = {0xe4, 0xb8, 0x83, 0xe5, 0x85, 0xab};
    const uint8_t token7[] = {0xe5, 0x85, 0xab, 0xe4, 0xb9, 0x9d};
    const uint8_t token8[] = {0xe5, 0x8d, 0x81};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token5), 7, 9, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token7), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 12, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

TEST_F(CJKTokenizerTest, testC) {
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

TEST_F(CJKTokenizerTest, testMix) {
    const uint8_t str[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86, 0xe3, 0x81,
                           0x88, 0xe3, 0x81, 0x8a, 0x61, 0x62, 0x63, 0xe3, 0x81, 0x8b, 0xe3,
                           0x81, 0x8d, 0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91, 0xe3, 0x81, 0x93
                          };

    const uint8_t token1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    const uint8_t token2[] = {0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86};
    const uint8_t token3[] = {0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88};
    const uint8_t token4[] = {0xe3, 0x81, 0x88, 0xe3, 0x81, 0x8a};
    const uint8_t token6[] = {0xe3, 0x81, 0x8b, 0xe3, 0x81, 0x8d};
    const uint8_t token7[] = {0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f};
    const uint8_t token8[] = {0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91};
    const uint8_t token9[] = {0xe3, 0x81, 0x91, 0xe3, 0x81, 0x93};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(L"abc", 5, 8, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token7), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token9), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

TEST_F(CJKTokenizerTest, testMix2) {
    const uint8_t str[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86, 0xe3, 0x81,
                           0x88, 0xe3, 0x81, 0x8a, 0x61, 0x62, 0xe3, 0x82, 0x93, 0x63, 0xe3,
                           0x81, 0x8b, 0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91
                          };

    const uint8_t token1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    const uint8_t token2[] = {0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86};
    const uint8_t token3[] = {0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88};
    const uint8_t token4[] = {0xe3, 0x81, 0x88, 0xe3, 0x81, 0x8a};
    const uint8_t token6[] = {0xe3, 0x82, 0x93};
    const uint8_t token8[] = {0xe3, 0x81, 0x8b, 0xe3, 0x81, 0x8d};
    const uint8_t token9[] = {0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f};
    const uint8_t token10[] = {0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(L"ab", 5, 7, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 7, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(L"c", 8, 9, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token9), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token10), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

TEST_F(CJKTokenizerTest, testSingleChar) {
    const uint8_t str[] = {0xe4, 0xb8, 0x80};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(str), 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

/// Full-width text is normalized to half-width
TEST_F(CJKTokenizerTest, testFullWidth) {
    const uint8_t str[] = {0xef, 0xbc, 0xb4, 0xef, 0xbd, 0x85, 0xef, 0xbd, 0x93, 0xef, 0xbd, 0x94,
                           0x20, 0xef, 0xbc, 0x91, 0xef, 0xbc, 0x92, 0xef, 0xbc, 0x93, 0xef, 0xbc, 0x94
                          };

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                           TestToken(L"1234", 5, 9, CJKTokenizer::SINGLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

/// Non-english text (not just CJK) is treated the same as CJK: C1C2 C2C3
TEST_F(CJKTokenizerTest, testNonIdeographic) {
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0x20, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8, 0xd8, 0xb1,
                           0xd8, 0xaa, 0x20, 0xd9, 0x85, 0xd9, 0x88, 0xd9, 0x8a, 0xd8, 0xb1
                          };

    const uint8_t token1[] = {0xe4, 0xb8, 0x80};
    const uint8_t token2[] = {0xd8, 0xb1, 0xd9, 0x88};
    const uint8_t token3[] = {0xd9, 0x88, 0xd8, 0xa8};
    const uint8_t token4[] = {0xd8, 0xa8, 0xd8, 0xb1};
    const uint8_t token5[] = {0xd8, 0xb1, 0xd8, 0xaa};
    const uint8_t token6[] = {0xd9, 0x85, 0xd9, 0x88};
    const uint8_t token7[] = {0xd9, 0x88, 0xd9, 0x8a};
    const uint8_t token8[] = {0xd9, 0x8a, 0xd8, 0xb1};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token5), 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token7), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

/// Non-english text with non-letters (non-spacing marks,etc) is treated as C1C2 C2C3,
/// except for words are split around non-letters.
TEST_F(CJKTokenizerTest, testNonIdeographicNonLetter) {
    const uint8_t str[] = {0xe4, 0xb8, 0x80, 0x20, 0xd8, 0xb1, 0xd9, 0x8f, 0xd9, 0x88, 0xd8, 0xa8,
                           0xd8, 0xb1, 0xd8, 0xaa, 0x20, 0xd9, 0x85, 0xd9, 0x88, 0xd9, 0x8a, 0xd8, 0xb1
                          };

    const uint8_t token1[] = {0xe4, 0xb8, 0x80};
    const uint8_t token2[] = {0xd8, 0xb1};
    const uint8_t token3[] = {0xd9, 0x88, 0xd8, 0xa8};
    const uint8_t token4[] = {0xd8, 0xa8, 0xd8, 0xb1};
    const uint8_t token5[] = {0xd8, 0xb1, 0xd8, 0xaa};
    const uint8_t token6[] = {0xd9, 0x85, 0xd9, 0x88};
    const uint8_t token7[] = {0xd9, 0x88, 0xd9, 0x8a};
    const uint8_t token8[] = {0xd9, 0x8a, 0xd8, 0xb1};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(token1), 0, 1, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token2), 2, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token3), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token4), 5, 7, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token5), 6, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token6), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token7), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(token8), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKToken(UTF8_TO_STRING(str), out_tokens);
}

TEST_F(CJKTokenizerTest, testTokenStream) {
    AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    const uint8_t token1[] = {0xe4, 0xb8, 0x80, 0xe4, 0xb8, 0x81, 0xe4, 0xb8, 0x82};
    const uint8_t token2[] = {0xe4, 0xb8, 0x80, 0xe4, 0xb8, 0x81};
    const uint8_t token3[] = {0xe4, 0xb8, 0x81, 0xe4, 0xb8, 0x82};

    checkAnalyzesTo(analyzer, UTF8_TO_STRING(token1), newCollection<String>(UTF8_TO_STRING(token2), UTF8_TO_STRING(token3)));
}

TEST_F(CJKTokenizerTest, testReusableTokenStream) {
    AnalyzerPtr analyzer = newLucene<CJKAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    const uint8_t first[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88,
                             0xe3, 0x81, 0x8a, 0x61, 0x62, 0x63, 0xe3, 0x81, 0x8b, 0xe3, 0x81, 0x8d,
                             0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91, 0xe3, 0x81, 0x93
                            };

    const uint8_t firstToken1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    const uint8_t firstToken2[] = {0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86};
    const uint8_t firstToken3[] = {0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88};
    const uint8_t firstToken4[] = {0xe3, 0x81, 0x88, 0xe3, 0x81, 0x8a};
    const uint8_t firstToken6[] = {0xe3, 0x81, 0x8b, 0xe3, 0x81, 0x8d};
    const uint8_t firstToken7[] = {0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f};
    const uint8_t firstToken8[] = {0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91};
    const uint8_t firstToken9[] = {0xe3, 0x81, 0x91, 0xe3, 0x81, 0x93};

    Collection<TestToken> out_tokens = newCollection<TestToken>(
                                           TestToken(UTF8_TO_STRING(firstToken1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken2), 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken3), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken4), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(L"abc", 5, 8, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken6), 8, 10, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken7), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken8), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                           TestToken(UTF8_TO_STRING(firstToken9), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                       );
    checkCJKTokenReusable(analyzer, UTF8_TO_STRING(first), out_tokens);

    const uint8_t second[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88,
                              0xe3, 0x81, 0x8a, 0x61, 0x62, 0xe3, 0x82, 0x93, 0x63, 0xe3, 0x81, 0x8b,
                              0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91
                             };

    const uint8_t secondToken1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    const uint8_t secondToken2[] = {0xe3, 0x81, 0x84, 0xe3, 0x81, 0x86};
    const uint8_t secondToken3[] = {0xe3, 0x81, 0x86, 0xe3, 0x81, 0x88};
    const uint8_t secondToken4[] = {0xe3, 0x81, 0x88, 0xe3, 0x81, 0x8a};
    const uint8_t secondToken6[] = {0xe3, 0x82, 0x93};
    const uint8_t secondToken8[] = {0xe3, 0x81, 0x8b, 0xe3, 0x81, 0x8d};
    const uint8_t secondToken9[] = {0xe3, 0x81, 0x8d, 0xe3, 0x81, 0x8f};
    const uint8_t secondToken10[] = {0xe3, 0x81, 0x8f, 0xe3, 0x81, 0x91};

    Collection<TestToken> out_tokens2 = newCollection<TestToken>(
                                            TestToken(UTF8_TO_STRING(secondToken1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken2), 1, 3, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken3), 2, 4, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken4), 3, 5, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(L"ab", 5, 7, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken6), 7, 8, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(L"c", 8, 9, CJKTokenizer::SINGLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken8), 9, 11, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken9), 10, 12, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                                            TestToken(UTF8_TO_STRING(secondToken10), 11, 13, CJKTokenizer::DOUBLE_TOKEN_TYPE)
                                        );
    checkCJKTokenReusable(analyzer, UTF8_TO_STRING(second), out_tokens2);
}

TEST_F(CJKTokenizerTest, testFinalOffset) {
    const uint8_t token1[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84};
    checkCJKToken(UTF8_TO_STRING(token1),
                  newCollection<TestToken>(TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
    const uint8_t token2[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x20, 0x20, 0x20};
    checkCJKToken(UTF8_TO_STRING(token2),
                  newCollection<TestToken>(TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
    checkCJKToken(L"test", newCollection<TestToken>(TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    checkCJKToken(L"test   ", newCollection<TestToken>(TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    const uint8_t token3[] = {0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x74, 0x65, 0x73, 0x74};
    checkCJKToken(UTF8_TO_STRING(token3),
                  newCollection<TestToken>(
                      TestToken(UTF8_TO_STRING(token1), 0, 2, CJKTokenizer::DOUBLE_TOKEN_TYPE),
                      TestToken(L"test", 2, 6, CJKTokenizer::SINGLE_TOKEN_TYPE)));
    const uint8_t token4[] = {0x74, 0x65, 0x73, 0x74, 0xe3, 0x81, 0x82, 0xe3, 0x81, 0x84, 0x20, 0x20, 0x20, 0x20};
    checkCJKToken(UTF8_TO_STRING(token4),
                  newCollection<TestToken>(
                      TestToken(L"test", 0, 4, CJKTokenizer::SINGLE_TOKEN_TYPE),
                      TestToken(UTF8_TO_STRING(token1), 4, 6, CJKTokenizer::DOUBLE_TOKEN_TYPE)));
}
