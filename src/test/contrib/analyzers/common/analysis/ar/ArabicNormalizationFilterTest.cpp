/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "ArabicLetterTokenizer.h"
#include "ArabicNormalizationFilter.h"
#include "StringReader.h"

using namespace Lucene;

class ArabicNormalizationFilterTest : public BaseTokenStreamFixture {
public:
    virtual ~ArabicNormalizationFilterTest() {
    }

public:
    void check(const String& input, const String& expected) {
        ArabicLetterTokenizerPtr tokenStream  = newLucene<ArabicLetterTokenizer>(newLucene<StringReader>(input));
        ArabicNormalizationFilterPtr filter = newLucene<ArabicNormalizationFilter>(tokenStream);
        checkTokenStreamContents(filter, newCollection<String>(expected));
    }
};

TEST_F(ArabicNormalizationFilterTest, testAlifMadda) {
    const uint8_t first[] = {0xd8, 0xa2, 0xd8, 0xac, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xa7, 0xd8, 0xac, 0xd9, 0x86};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testAlifHamzaAbove) {
    const uint8_t first[] = {0xd8, 0xa3, 0xd8, 0xad, 0xd9, 0x85, 0xd8, 0xaf};
    const uint8_t second[] = {0xd8, 0xa7, 0xd8, 0xad, 0xd9, 0x85, 0xd8, 0xaf};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testAlifHamzaBelow) {
    const uint8_t first[] = {0xd8, 0xa5, 0xd8, 0xb9, 0xd8, 0xa7, 0xd8, 0xb0};
    const uint8_t second[] = {0xd8, 0xa7, 0xd8, 0xb9, 0xd8, 0xa7, 0xd8, 0xb0};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testAlifMaksura) {
    const uint8_t first[] = {0xd8, 0xa8, 0xd9, 0x86, 0xd9, 0x89};
    const uint8_t second[] = {0xd8, 0xa8, 0xd9, 0x86, 0xd9, 0x8a};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testTehMarbuta) {
    const uint8_t first[] = {0xd9, 0x81, 0xd8, 0xa7, 0xd8, 0xb7, 0xd9, 0x85, 0xd8, 0xa9};
    const uint8_t second[] = {0xd9, 0x81, 0xd8, 0xa7, 0xd8, 0xb7, 0xd9, 0x85, 0xd9, 0x87};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testTatweel) {
    const uint8_t first[] = {0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8, 0xd8, 0xb1, 0xd9, 0x80, 0xd9, 0x80, 0xd9, 0x80, 0xd9, 0x80, 0xd9, 0x80, 0xd8, 0xaa};
    const uint8_t second[] = {0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8, 0xd8, 0xb1, 0xd8, 0xaa};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testFatha) {
    const uint8_t first[] = {0xd9, 0x85, 0xd9, 0x8e, 0xd8, 0xa8, 0xd9, 0x86, 0xd8, 0xa7};
    const uint8_t second[] = {0xd9, 0x85, 0xd8, 0xa8, 0xd9, 0x86, 0xd8, 0xa7};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testKasra) {
    const uint8_t first[] = {0xd8, 0xb9, 0xd9, 0x84, 0xd9, 0x90, 0xd9, 0x8a};
    const uint8_t second[] = {0xd8, 0xb9, 0xd9, 0x84, 0xd9, 0x8a};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testDamma) {
    const uint8_t first[] = {0xd8, 0xa8, 0xd9, 0x8f, 0xd9, 0x88, 0xd8, 0xa7, 0xd8, 0xaa};
    const uint8_t second[] = {0xd8, 0xa8, 0xd9, 0x88, 0xd8, 0xa7, 0xd8, 0xaa};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testFathatan) {
    const uint8_t first[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf, 0xd8, 0xa7, 0xd9, 0x8b};
    const uint8_t second[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf, 0xd8, 0xa7};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testKasratan) {
    const uint8_t first[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf, 0xd9, 0x8d};
    const uint8_t second[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testDammatan) {
    const uint8_t first[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf, 0xd9, 0x8c};
    const uint8_t second[] = {0xd9, 0x88, 0xd9, 0x84, 0xd8, 0xaf};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testSukun) {
    const uint8_t first[] = {0xd9, 0x86, 0xd9, 0x84, 0xd9, 0x92, 0xd8, 0xb3, 0xd9, 0x88, 0xd9, 0x86};
    const uint8_t second[] = {0xd9, 0x86, 0xd9, 0x84, 0xd8, 0xb3, 0xd9, 0x88, 0xd9, 0x86};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

TEST_F(ArabicNormalizationFilterTest, testShaddah) {
    const uint8_t first[] = {0xd9, 0x87, 0xd8, 0xaa, 0xd9, 0x85, 0xd9, 0x8a, 0xd9, 0x91};
    const uint8_t second[] = {0xd9, 0x87, 0xd8, 0xaa, 0xd9, 0x85, 0xd9, 0x8a};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}
