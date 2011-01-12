/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "CharFilter.h"
#include "StringReader.h"
#include "CharReader.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CharFilterTest, LuceneTestFixture)

class CharFilter1 : public CharFilter
{
public:
    CharFilter1(CharStreamPtr in) : CharFilter(in)
    {
    }
    
    virtual ~CharFilter1()
    {
    }

protected:
    virtual int32_t correct(int32_t currentOff)
    {
        return currentOff + 1;
    }
};

class CharFilter2 : public CharFilter
{
public:
    CharFilter2(CharStreamPtr in) : CharFilter(in)
    {
    }
    
    virtual ~CharFilter2()
    {
    }

protected:
    virtual int32_t correct(int32_t currentOff)
    {
        return currentOff + 2;
    }
};

BOOST_AUTO_TEST_CASE(testCharFilter1)
{
    CharStreamPtr cs = newLucene<CharFilter1>(CharReader::get(newLucene<StringReader>(L"")));
    BOOST_CHECK_EQUAL(1, cs->correctOffset(0));
}

BOOST_AUTO_TEST_CASE(testCharFilter2)
{
    CharStreamPtr cs = newLucene<CharFilter2>(CharReader::get(newLucene<StringReader>(L"")));
    BOOST_CHECK_EQUAL(2, cs->correctOffset(0));
}

BOOST_AUTO_TEST_CASE(testCharFilter12)
{
    CharStreamPtr cs = newLucene<CharFilter2>(newLucene<CharFilter1>(CharReader::get(newLucene<StringReader>(L""))));
    BOOST_CHECK_EQUAL(3, cs->correctOffset(0));
}

BOOST_AUTO_TEST_CASE(testCharFilter11)
{
    CharStreamPtr cs = newLucene<CharFilter1>(newLucene<CharFilter1>(CharReader::get(newLucene<StringReader>(L""))));
    BOOST_CHECK_EQUAL(2, cs->correctOffset(0));
}

BOOST_AUTO_TEST_SUITE_END()
