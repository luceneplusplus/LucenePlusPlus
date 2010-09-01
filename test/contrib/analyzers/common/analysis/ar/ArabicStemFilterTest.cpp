/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseTokenStreamFixture.h"
#include "ArabicStemFilter.h"
#include "ArabicLetterTokenizer.h"
#include "StringReader.h"

using namespace Lucene;

class ArabicStemFilterFixture : public BaseTokenStreamFixture
{
public:
	virtual ~ArabicStemFilterFixture()
	{
	}

public:
	void check(const String& input, const String& expected)
    {
        ArabicLetterTokenizerPtr tokenStream  = newLucene<ArabicLetterTokenizer>(newLucene<StringReader>(input));
        ArabicStemFilterPtr filter = newLucene<ArabicStemFilter>(tokenStream);
        checkTokenStreamContents(filter, newCollection<String>(expected));
    }
};

BOOST_FIXTURE_TEST_SUITE(ArabicStemFilterTest, ArabicStemFilterFixture)

BOOST_AUTO_TEST_CASE(testAlPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testWalPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x88, 0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testBalPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xa8, 0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testKalPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testFalPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x81, 0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testLlPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x84, 0xd9, 0x84, 0xd8, 0xa7, 0xd8, 0xae, 0xd8, 0xb1};
    const uint8_t second[] = {0xd8, 0xa7, 0xd8, 0xae, 0xd8, 0xb1};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testWaPrefix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x88, 0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xad, 0xd8, 0xb3, 0xd9, 0x86};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testAhSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb2, 0xd9, 0x88, 0xd8, 0xac, 0xd9, 0x87, 0xd8, 0xa7};
    const uint8_t second[] = {0xd8, 0xb2, 0xd9, 0x88, 0xd8, 0xac};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testAnSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd8, 0xa7, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testAtSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd8, 0xa7, 0xd8, 0xaa};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testWnSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x88, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testYnSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x8a, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testYhSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x8a, 0xd9, 0x87};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testYpSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x8a, 0xd8, 0xa9};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testHSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x87};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testPSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd8, 0xa9};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testYSuffix)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x8a};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testComboPrefSuf)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd9, 0x88, 0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x88, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testComboSuf)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf, 0xd9, 0x87, 0xd8, 0xa7, 0xd8, 0xaa};
    const uint8_t second[] = {0xd8, 0xb3, 0xd8, 0xa7, 0xd9, 0x87, 0xd8, 0xaf};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testShouldntStem)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const uint8_t first[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x88};
    const uint8_t second[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x88};
    check(StringUtils::toUnicode(first, sizeof(first) / sizeof(first[0])), StringUtils::toUnicode(second, sizeof(second) / sizeof(second[0])));
    #endif
}

BOOST_AUTO_TEST_CASE(testNonArabic)
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    check(L"English", L"English");
    #endif
}

BOOST_AUTO_TEST_SUITE_END()
