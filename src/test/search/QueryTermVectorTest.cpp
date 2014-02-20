/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "QueryTermVector.h"
#include "WhitespaceAnalyzer.h"

using namespace Lucene;

typedef LuceneTestFixture QueryTermVectorTest;

static void checkGold(Collection<String> terms, Collection<String> gold, Collection<int32_t> freq, Collection<int32_t> goldFreqs) {
    for (int32_t i = 0; i < terms.size(); ++i) {
        EXPECT_EQ(terms[i], gold[i]);
        EXPECT_EQ(freq[i], goldFreqs[i]);
    }
}

TEST_F(QueryTermVectorTest, testConstructor) {
    Collection<String> queryTerm = newCollection<String>(L"foo", L"bar", L"foo", L"again", L"foo", L"bar", L"go", L"go", L"go");

    // Items are sorted lexicographically
    Collection<String> gold = newCollection<String>(L"again", L"bar", L"foo", L"go");
    Collection<int32_t> goldFreqs = newCollection<int32_t>(1, 2, 3, 3);
    QueryTermVectorPtr result = newLucene<QueryTermVector>(queryTerm);
    EXPECT_TRUE(result);
    Collection<String> terms = result->getTerms();
    EXPECT_EQ(terms.size(), 4);
    Collection<int32_t> freq = result->getTermFrequencies();
    EXPECT_EQ(freq.size(), 4);
    checkGold(terms, gold, freq, goldFreqs);
    result = newLucene<QueryTermVector>(Collection<String>());
    EXPECT_EQ(result->getTerms().size(), 0);

    result = newLucene<QueryTermVector>(L"foo bar foo again foo bar go go go", newLucene<WhitespaceAnalyzer>());
    EXPECT_TRUE(result);
    terms = result->getTerms();
    EXPECT_EQ(terms.size(), 4);
    freq = result->getTermFrequencies();
    EXPECT_EQ(freq.size(), 4);
    checkGold(terms, gold, freq, goldFreqs);
}
