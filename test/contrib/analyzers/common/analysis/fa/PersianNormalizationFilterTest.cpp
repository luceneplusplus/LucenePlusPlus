/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "ArabicLetterTokenizer.h"
#include "PersianNormalizationFilter.h"
#include "StringReader.h"

using namespace Lucene;

class PersianNormalizationFilterFixture : public BaseTokenStreamFixture
{
public:
	virtual ~PersianNormalizationFilterFixture()
	{
	}

public:
	void check(const String& input, const String& expected)
    {
        ArabicLetterTokenizerPtr tokenStream  = newLucene<ArabicLetterTokenizer>(newLucene<StringReader>(input));
        PersianNormalizationFilterPtr filter = newLucene<PersianNormalizationFilter>(tokenStream);
        checkTokenStreamContents(filter, newCollection<String>(expected));
    }
};

BOOST_FIXTURE_TEST_SUITE(PersianNormalizationFilterTest, PersianNormalizationFilterFixture)

BOOST_AUTO_TEST_CASE(testFarsiYeh)
{
    const uint8_t first[] = {0xd9, 0x87, 0xd8, 0xa7, 0xdb, 0x8c};
    const uint8_t second[] = {0xd9, 0x87, 0xd8, 0xa7, 0xd9, 0x8a};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_CASE(testYehBarree)
{
    const uint8_t first[] = {0xd9, 0x87, 0xd8, 0xa7, 0xdb, 0x92};
    const uint8_t second[] = {0xd9, 0x87, 0xd8, 0xa7, 0xd9, 0x8a};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_CASE(testKeheh)
{
    const uint8_t first[] = {0xda, 0xa9, 0xd8, 0xb4, 0xd8, 0xa7, 0xd9, 0x86, 0xd8, 0xaf, 0xd9, 0x86};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xb4, 0xd8, 0xa7, 0xd9, 0x86, 0xd8, 0xaf, 0xd9, 0x86};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_CASE(testHehYeh)
{
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8, 0xdb, 0x80};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8, 0xd9, 0x87};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_CASE(testHehHamzaAbove)
{
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8, 0xd9, 0x87, 0xd9, 0x94};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8, 0xd9, 0x87};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_CASE(testHehGoal)
{
    const uint8_t first[] = {0xd8, 0xb2, 0xd8, 0xa7, 0xd8, 0xaf, 0xdb, 0x81};
    const uint8_t second[] = {0xd8, 0xb2, 0xd8, 0xa7, 0xd8, 0xaf, 0xd9, 0x87};
    check(UTF8_TO_STRING(first), UTF8_TO_STRING(second));
}

BOOST_AUTO_TEST_SUITE_END()
