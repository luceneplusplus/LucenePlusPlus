/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "NormalizeCharMap.h"
#include "CharStream.h"
#include "MappingCharFilter.h"
#include "StringReader.h"
#include "WhitespaceTokenizer.h"
#include "CharReader.h"

using namespace Lucene;

class MappingCharFilterTestFixture : public BaseTokenStreamFixture
{
public:
    MappingCharFilterTestFixture()
    {
        normMap = newLucene<NormalizeCharMap>();

        normMap->add(L"aa", L"a");
        normMap->add(L"bbb", L"b");
        normMap->add(L"cccc", L"cc");

        normMap->add(L"h", L"i");
        normMap->add(L"j", L"jj");
        normMap->add(L"k", L"kkk");
        normMap->add(L"ll", L"llll");

        normMap->add(L"empty", L"");
    }
    
    virtual ~MappingCharFilterTestFixture()
    {
    }

public:
    NormalizeCharMapPtr normMap;
};

BOOST_FIXTURE_TEST_SUITE(MappingCharFilterTest, MappingCharFilterTestFixture)

BOOST_AUTO_TEST_CASE(testReaderReset)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"x"));
    CharArray buf = CharArray::newInstance(10);
    int32_t len = cs->read(buf.get(), 0, 10);
    BOOST_CHECK_EQUAL(1, len);
    BOOST_CHECK_EQUAL(L'x', buf[0]) ;
    len = cs->read(buf.get(), 0, 10);
    BOOST_CHECK_EQUAL(-1, len);

    // rewind
    cs->reset();
    len = cs->read(buf.get(), 0, 10);
    BOOST_CHECK_EQUAL(1, len);
    BOOST_CHECK_EQUAL(L'x', buf[0]) ;
}

BOOST_AUTO_TEST_CASE(testNothingChange)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"x"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"x"), newCollection<int32_t>(0), newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(test1to1)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"h"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"i"), newCollection<int32_t>(0), newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(test1to2)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"j"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"jj"), newCollection<int32_t>(0), newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(test1to3)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"k"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"kkk"), newCollection<int32_t>(0), newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(test2to4)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"ll"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"llll"), newCollection<int32_t>(0), newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(test2to1)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"aa"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"a"), newCollection<int32_t>(0), newCollection<int32_t>(2));
}

BOOST_AUTO_TEST_CASE(test3to1)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"bbb"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"b"), newCollection<int32_t>(0), newCollection<int32_t>(3));
}

BOOST_AUTO_TEST_CASE(test4to2)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"cccc"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"cc"), newCollection<int32_t>(0), newCollection<int32_t>(4));
}

BOOST_AUTO_TEST_CASE(test5to0)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, newLucene<StringReader>(L"empty"));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, Collection<String>::newInstance());
}

//
//                1111111111222
//      01234567890123456789012
//(in)  h i j k ll cccc bbb aa
//
//                1111111111222
//      01234567890123456789012
//(out) i i jj kkk llll cc b a
//
//    h, 0, 1 =>    i, 0, 1
//    i, 2, 3 =>    i, 2, 3
//    j, 4, 5 =>   jj, 4, 5
//    k, 6, 7 =>  kkk, 6, 7
//   ll, 8,10 => llll, 8,10
// cccc,11,15 =>   cc,11,15
//  bbb,16,19 =>    b,16,19
//   aa,20,22 =>    a,20,22
BOOST_AUTO_TEST_CASE(testTokenStream)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, CharReader::get(newLucene<StringReader>(L"h i j k ll cccc bbb aa")));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"i", L"i", L"jj", L"kkk", L"llll", L"cc", L"b", L"a"), 
                             newCollection<int32_t>(0, 2, 4, 6, 8, 11, 16, 20), 
                             newCollection<int32_t>(1, 3, 5, 7, 10, 15, 19, 22));
}

//
//
//        0123456789
//(in)    aaaa ll h
//(out-1) aa llll i
//(out-2) a llllllll i
//
// aaaa,0,4 => a,0,4
//   ll,5,7 => llllllll,5,7
//    h,8,9 => i,8,9
BOOST_AUTO_TEST_CASE(testChained)
{
    CharStreamPtr cs = newLucene<MappingCharFilter>(normMap, (CharStreamPtr)newLucene<MappingCharFilter>(normMap, CharReader::get(newLucene<StringReader>(L"aaaa ll h"))));
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(cs);
    checkTokenStreamContents(ts, newCollection<String>(L"a", L"llllllll", L"i"), newCollection<int32_t>(0, 5, 8), newCollection<int32_t>(4, 7, 9));
}

BOOST_AUTO_TEST_SUITE_END()
