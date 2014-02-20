/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <float.h>
#include "BaseTestRangeFilterFixture.h"
#include "RAMDirectory.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "FieldCacheRangeFilter.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "DocIdSet.h"

using namespace Lucene;

/// A basic 'positive' Unit test class for the FieldCacheRangeFilter class.
typedef BaseTestRangeFilterFixture FieldCacheRangeFilterTest;

TEST_F(FieldCacheRangeFilterTest, testRangeFilterId) {
    IndexReaderPtr reader = IndexReader::open((DirectoryPtr)signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t medId = ((maxId - minId) / 2);

    String minIP = pad(minId);
    String maxIP = pad(maxId);
    String medIP = pad(medId);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test id, bounded on both ends

    FieldCacheRangeFilterPtr fcrf = FieldCacheRangeFilter::newStringRange(L"id", minIP, maxIP, true, true);
    Collection<ScoreDocPtr> result = search->search(q, fcrf, numDocs)->scoreDocs;
    EXPECT_TRUE(fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, maxIP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, maxIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", medIP, maxIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, medIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", L"", L"", true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, L"", true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", L"", maxIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, L"", false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", L"", maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", medIP, maxIP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, medIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, minIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", medIP, medIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", maxIP, maxIP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", minIP, minIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", L"", minIP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", maxIP, maxIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", maxIP, L"", true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"id", medIP, medIP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(FieldCacheRangeFilterTest, testFieldCacheRangeFilterRand) {
    IndexReaderPtr reader = IndexReader::open((DirectoryPtr)signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t medId = ((maxId - minId) / 2);

    String minRP = pad(signedIndex->minR);
    String maxRP = pad(signedIndex->maxR);

    int32_t numDocs = reader->numDocs();

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test extremes, bounded on both ends

    Collection<ScoreDocPtr> result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, maxRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, maxRP, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, maxRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    // unbounded

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, L"", true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", L"", maxRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, L"", false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", L"", maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    // very small sets

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, minRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", maxRP, maxRP, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", minRP, minRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", L"", minRP, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", maxRP, maxRP, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newStringRange(L"rand", maxRP, L"", true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(FieldCacheRangeFilterTest, testFieldCacheRangeFilterInts) {
    IndexReaderPtr reader = IndexReader::open((DirectoryPtr)signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t numDocs = reader->numDocs();
    int32_t medId = ((maxId - minId) / 2);

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test id, bounded on both ends

    FieldCacheRangeFilterPtr fcrf = FieldCacheRangeFilter::newIntRange(L"id", minId, maxId, true, true);
    Collection<ScoreDocPtr> result = search->search(q, fcrf, numDocs)->scoreDocs;
    EXPECT_TRUE(fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(numDocs, result.size());

    // test extremes, bounded on both ends

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, maxId, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, maxId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", medId, maxId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, medId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", INT_MIN, INT_MAX, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, INT_MAX, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", INT_MIN, maxId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, INT_MAX, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", INT_MIN, maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", medId, maxId, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, medId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, minId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", medId, medId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", maxId, maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", minId, minId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", INT_MIN, minId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", maxId, maxId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", maxId, INT_MAX, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newIntRange(L"id", medId, medId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(FieldCacheRangeFilterTest, testFieldCacheRangeFilterLongs) {
    IndexReaderPtr reader = IndexReader::open((DirectoryPtr)signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t numDocs = reader->numDocs();
    int64_t medId = ((maxId - minId) / 2);

    EXPECT_EQ(numDocs, 1 + maxId - minId);

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    // test id, bounded on both ends

    FieldCacheRangeFilterPtr fcrf = FieldCacheRangeFilter::newLongRange(L"id", minId, maxId, true, true);
    Collection<ScoreDocPtr> result = search->search(q, fcrf, numDocs)->scoreDocs;
    EXPECT_TRUE(fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(numDocs, result.size());

    // test extremes, bounded on both ends

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, (int64_t)maxId, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, (int64_t)maxId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, (int64_t)maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 2, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", medId, (int64_t)maxId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, medId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1 + medId - minId, result.size());

    // unbounded id

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", LLONG_MIN, LLONG_MAX, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, LLONG_MAX, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", LLONG_MIN, (int64_t)maxId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, LLONG_MAX, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", LLONG_MIN, (int64_t)maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs - 1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", medId, (int64_t)maxId, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(maxId - medId, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, medId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(medId - minId, result.size());

    // very small sets

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, (int64_t)minId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", medId, medId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)maxId, (int64_t)maxId, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)minId, (int64_t)minId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", LLONG_MIN, (int64_t)minId, false, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)maxId, (int64_t)maxId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", (int64_t)maxId, LLONG_MAX, true, false), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = search->search(q, FieldCacheRangeFilter::newLongRange(L"id", medId, medId, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(1, result.size());
}

TEST_F(FieldCacheRangeFilterTest, testFieldCacheRangeFilterDoubles) {
    IndexReaderPtr reader = IndexReader::open((DirectoryPtr)signedIndex->index, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);

    int32_t numDocs = reader->numDocs();
    double minIdO = (double)minId + 0.5;
    double medIdO = minIdO + (double)(maxId - minId) / 2.0;

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    Collection<ScoreDocPtr> result = search->search(q, FieldCacheRangeFilter::newDoubleRange(L"id", minIdO, medIdO, true, true), numDocs)->scoreDocs;
    EXPECT_EQ(numDocs / 2, result.size());

    int32_t count = 0;
    result = search->search(q, FieldCacheRangeFilter::newDoubleRange(L"id", -DBL_MAX, medIdO, false, true), numDocs)->scoreDocs;
    count += result.size();
    result = search->search(q, FieldCacheRangeFilter::newDoubleRange(L"id", medIdO, DBL_MAX, false, false), numDocs)->scoreDocs;
    count += result.size();
    EXPECT_EQ(numDocs, count);
    result = search->search(q, FieldCacheRangeFilter::newDoubleRange(L"id", std::numeric_limits<double>::infinity(), DBL_MAX, false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
    result = search->search(q, FieldCacheRangeFilter::newDoubleRange(L"id", -DBL_MAX, -std::numeric_limits<double>::infinity(), false, false), numDocs)->scoreDocs;
    EXPECT_EQ(0, result.size());
}

TEST_F(FieldCacheRangeFilterTest, testSparseIndex) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    for (int32_t d = -20; d <= 20; ++d) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", StringUtils::toString(d), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"body", L"body", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }

    writer->optimize();
    writer->deleteDocuments(newLucene<Term>(L"id", L"0"));
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    IndexSearcherPtr search = newLucene<IndexSearcher>(reader);
    EXPECT_TRUE(reader->hasDeletions());

    QueryPtr q = newLucene<TermQuery>(newLucene<Term>(L"body", L"body"));

    FieldCacheRangeFilterPtr fcrf = FieldCacheRangeFilter::newIntRange(L"id", -20, 20, true, true);
    Collection<ScoreDocPtr> result = search->search(q, fcrf, 100)->scoreDocs;
    EXPECT_TRUE(!fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(40, result.size());

    fcrf = FieldCacheRangeFilter::newIntRange(L"id", 0, 20, true, true);
    result = search->search(q, fcrf, 100)->scoreDocs;
    EXPECT_TRUE(!fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(20, result.size());

    fcrf = FieldCacheRangeFilter::newIntRange(L"id", -20, 0, true, true);
    result = search->search(q, fcrf, 100)->scoreDocs;
    EXPECT_TRUE(!fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(20, result.size());

    fcrf = FieldCacheRangeFilter::newIntRange(L"id", 10, 20, true, true);
    result = search->search(q, fcrf, 100)->scoreDocs;
    EXPECT_TRUE(fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(11, result.size());

    fcrf = FieldCacheRangeFilter::newIntRange(L"id", -20, -10, true, true);
    result = search->search(q, fcrf, 100)->scoreDocs;
    EXPECT_TRUE(fcrf->getDocIdSet(reader->getSequentialSubReaders()[0])->isCacheable());
    EXPECT_EQ(11, result.size());
}
