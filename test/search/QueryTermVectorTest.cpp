/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "QueryTermVector.h"
#include "WhitespaceAnalyzer.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(QueryTermVectorTest, LuceneTestFixture)

static void checkGold(Collection<String> terms, Collection<String> gold, Collection<int32_t> freq, Collection<int32_t> goldFreqs)
{
    for (int32_t i = 0; i < terms.size(); ++i)
    {
        BOOST_CHECK_EQUAL(terms[i], gold[i]);
        BOOST_CHECK_EQUAL(freq[i], goldFreqs[i]);
    }
}

BOOST_AUTO_TEST_CASE(testConstructor)
{
    Collection<String> queryTerm = newCollection<String>(L"foo", L"bar", L"foo", L"again", L"foo", L"bar", L"go", L"go", L"go");
    
    // Items are sorted lexicographically
    Collection<String> gold = newCollection<String>(L"again", L"bar", L"foo", L"go");
    Collection<int32_t> goldFreqs = newCollection<int32_t>(1, 2, 3, 3);
    QueryTermVectorPtr result = newLucene<QueryTermVector>(queryTerm);
    BOOST_CHECK(result);
    Collection<String> terms = result->getTerms();
    BOOST_CHECK_EQUAL(terms.size(), 4);
    Collection<int32_t> freq = result->getTermFrequencies();
    BOOST_CHECK_EQUAL(freq.size(), 4);
    checkGold(terms, gold, freq, goldFreqs);
    result = newLucene<QueryTermVector>(Collection<String>());
    BOOST_CHECK_EQUAL(result->getTerms().size(), 0);

    result = newLucene<QueryTermVector>(L"foo bar foo again foo bar go go go", newLucene<WhitespaceAnalyzer>());
    BOOST_CHECK(result);
    terms = result->getTerms();
    BOOST_CHECK_EQUAL(terms.size(), 4);
    freq = result->getTermFrequencies();
    BOOST_CHECK_EQUAL(freq.size(), 4);
    checkGold(terms, gold, freq, goldFreqs);
}

BOOST_AUTO_TEST_SUITE_END()
