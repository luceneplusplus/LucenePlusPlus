/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TermQuery.h"
#include "Term.h"
#include "QueryWrapperFilter.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MatchAllDocsQuery.h"
#include "BooleanQuery.h"
#include "FuzzyQuery.h"
#include "CachingWrapperFilter.h"

using namespace Lucene;

typedef LuceneTestFixture QueryWrapperFilterTest;

TEST_F(QueryWrapperFilterTest, testBasic) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"value", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    TermQueryPtr termQuery = newLucene<TermQuery>(newLucene<Term>(L"field", L"value"));

    // should not throw exception with primitive query
    QueryWrapperFilterPtr qwf = newLucene<QueryWrapperFilter>(termQuery);

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    TopDocsPtr hits = searcher->search(newLucene<MatchAllDocsQuery>(), qwf, 10);
    EXPECT_EQ(1, hits->totalHits);
    hits = searcher->search(newLucene<MatchAllDocsQuery>(), newLucene<CachingWrapperFilter>(qwf), 10);
    EXPECT_EQ(1, hits->totalHits);

    // should not throw exception with complex primitive query
    BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(termQuery, BooleanClause::MUST);
    booleanQuery->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"missing")), BooleanClause::MUST_NOT);
    qwf = newLucene<QueryWrapperFilter>(termQuery);

    hits = searcher->search(newLucene<MatchAllDocsQuery>(), qwf, 10);
    EXPECT_EQ(1, hits->totalHits);
    hits = searcher->search(newLucene<MatchAllDocsQuery>(), newLucene<CachingWrapperFilter>(qwf), 10);
    EXPECT_EQ(1, hits->totalHits);

    // should not throw exception with non primitive Query (doesn't implement Query#createWeight)
    qwf = newLucene<QueryWrapperFilter>(newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"valu")));

    hits = searcher->search(newLucene<MatchAllDocsQuery>(), qwf, 10);
    EXPECT_EQ(1, hits->totalHits);
    hits = searcher->search(newLucene<MatchAllDocsQuery>(), newLucene<CachingWrapperFilter>(qwf), 10);
    EXPECT_EQ(1, hits->totalHits);

    // test a query with no hits
    termQuery = newLucene<TermQuery>(newLucene<Term>(L"field", L"not_exist"));
    qwf = newLucene<QueryWrapperFilter>(termQuery);
    hits = searcher->search(newLucene<MatchAllDocsQuery>(), qwf, 10);
    EXPECT_EQ(0, hits->totalHits);
    hits = searcher->search(newLucene<MatchAllDocsQuery>(), newLucene<CachingWrapperFilter>(qwf), 10);
    EXPECT_EQ(0, hits->totalHits);
}
