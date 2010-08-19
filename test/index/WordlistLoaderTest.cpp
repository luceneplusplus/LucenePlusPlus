/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "WordlistLoader.h"
#include "StringReader.h"
#include "BufferedReader.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(WordlistLoaderTest, LuceneTestFixture)

static void checkSet(HashSet<String> wordset)
{
    BOOST_CHECK_EQUAL(3, wordset.size());
    BOOST_CHECK(wordset.contains(L"ONE")); // case is not modified
    BOOST_CHECK(!wordset.contains(L"one")); // case is not modified
    BOOST_CHECK(wordset.contains(L"two")); // surrounding whitespace is removed
    BOOST_CHECK(wordset.contains(L"three"));
    BOOST_CHECK(!wordset.contains(L"four"));
}

BOOST_AUTO_TEST_CASE(testWordlistLoading)
{
    String s = L"ONE\n  two \nthree";
    HashSet<String> wordSet1 = WordlistLoader::getWordSet(newLucene<StringReader>(s));
    checkSet(wordSet1);
    HashSet<String> wordSet2 = WordlistLoader::getWordSet(newLucene<BufferedReader>(newLucene<StringReader>(s)));
    checkSet(wordSet2);
}

BOOST_AUTO_TEST_CASE(testComments)
{
    String s = L"ONE\n  two \nthree\n#comment";
    HashSet<String> wordSet1 = WordlistLoader::getWordSet(newLucene<StringReader>(s), L"#");
    checkSet(wordSet1);
    BOOST_CHECK(!wordSet1.contains(L"#comment"));
    BOOST_CHECK(!wordSet1.contains(L"comment"));
}

BOOST_AUTO_TEST_SUITE_END()
