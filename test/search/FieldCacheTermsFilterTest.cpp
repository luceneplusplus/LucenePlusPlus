/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "KeywordAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "MatchAllDocsQuery.h"
#include "FieldCacheTermsFilter.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(FieldCacheTermsFilterTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testMissingTerms)
{
    String fieldName = L"field1";
    MockRAMDirectoryPtr rd = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(rd, newLucene<KeywordAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    for (int32_t i = 0; i < 100; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        int32_t term = i * 10; // terms are units of 10
        doc->add(newLucene<Field>(fieldName, StringUtils::toString(term), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        w->addDocument(doc);
    }
    w->close();

    IndexReaderPtr reader = IndexReader::open(rd, true);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);
    int32_t numDocs = reader->numDocs();
    MatchAllDocsQueryPtr q = newLucene<MatchAllDocsQuery>();

    Collection<ScoreDocPtr> results = searcher->search(q, newLucene<FieldCacheTermsFilter>(fieldName, newCollection<String>(L"5")), numDocs)->scoreDocs;
    BOOST_CHECK_EQUAL(0, results.size());

    results = searcher->search(q, newLucene<FieldCacheTermsFilter>(fieldName, newCollection<String>(L"10")), numDocs)->scoreDocs;
    BOOST_CHECK_EQUAL(1, results.size());

    results = searcher->search(q, newLucene<FieldCacheTermsFilter>(fieldName, newCollection<String>(L"10", L"20")), numDocs)->scoreDocs;
    BOOST_CHECK_EQUAL(2, results.size());

    reader->close();
    rd->close();
}

BOOST_AUTO_TEST_SUITE_END()
