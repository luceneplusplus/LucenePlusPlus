/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTestRangeFilterFixture.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TermRangeFilter.h"
#include "TermQuery.h"
#include "Term.h"
#include "RAMDirectory.h"
#include "Collator.h"
#include "VariantUtils.h"

using namespace Lucene;

class TermRangeFilterTest : public BaseTestRangeFilterFixture {
public:
    virtual ~TermRangeFilterTest() {
    }
};

TEST_F(TermRangeFilterTest, testRangeFilterId) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t medId = ((maxId - minId) / 2);

    String minIP = pad(minId);
    String maxIP = pad(maxId);
    String medIP = pad(medId);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test id, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, maxIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, medIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, VariantUtils::null(), true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), maxIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, VariantUtils::null(), false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, maxIP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, medIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, minIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, medIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, minIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), minIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, maxIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, VariantUtils::null(), true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, medIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(TermRangeFilterTest, testRangeFilterIdCollating) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    CollatorPtr c = newLucene<Collator>(std::locale());

    int32_t medId = ((maxId - minId) / 2);

    String minIP = pad(minId);
    String maxIP = pad(maxId);
    String medIP = pad(medId);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test id, bounded on both ends

    int32_t numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, true, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, maxIP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 2, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, maxIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1 + maxId - medId, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, medIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1 + medId - minId, numHits);

    // unbounded id

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, VariantUtils::null(), true, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), maxIP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, VariantUtils::null(), false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), maxIP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, maxIP, true, false, c), 1000)->totalHits;
    EXPECT_EQ(maxId - medId, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, medIP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(medId - minId, numHits);

    // very small sets

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, minIP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(0, numHits);
    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, medIP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(0, numHits);
    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, maxIP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(0, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", minIP, minIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", VariantUtils::null(), minIP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, maxIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", maxIP, VariantUtils::null(), true, false, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);

    numHits = search->search(q,  newLucene<TermRangeFilter>(L"id", medIP, medIP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
}

TEST_F(TermRangeFilterTest, testRangeFilterRand) {
    IndexReaderPtr reader = IndexReader::open(signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    String minRP = pad(signedIndex->minR);
    String maxRP = pad(signedIndex->maxR);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test extremes, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    // unbounded

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, VariantUtils::null(), true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), maxRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, VariantUtils::null(), false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    // very small sets

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, minRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, minRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), minRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, maxRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, VariantUtils::null(), true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(TermRangeFilterTest, testRangeFilterRandCollating) {
    // using the unsigned index because collation seems to ignore hyphens
    IndexReaderPtr reader = IndexReader::open(unsignedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    CollatorPtr c = newLucene<Collator>(std::locale());

    String minRP = pad(unsignedIndex->minR);
    String maxRP = pad(unsignedIndex->maxR);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test extremes, bounded on both ends

    int32_t numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, true, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, maxRP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 2, numHits);

    // unbounded

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, VariantUtils::null(), true, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), maxRP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(numDocs, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, VariantUtils::null(), false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), maxRP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(numDocs - 1, numHits);

    // very small sets

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, minRP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(0, numHits);
    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, maxRP, false, false, c), 1000)->totalHits;
    EXPECT_EQ(0, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", minRP, minRP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", VariantUtils::null(), minRP, false, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);

    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, maxRP, true, true, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
    numHits = search->search(q, newLucene<TermRangeFilter>(L"rand", maxRP, VariantUtils::null(), true, false, c), 1000)->totalHits;
    EXPECT_EQ(1, numHits);
}
