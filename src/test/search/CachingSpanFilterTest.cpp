/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "CachingSpanFilter.h"
#include "CachingWrapperFilter.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "Field.h"
#include "Document.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "MatchAllDocsQuery.h"
#include "ConstantScoreQuery.h"
#include "SpanFilter.h"
#include "SpanTermQuery.h"
#include "SpanQueryFilter.h"
#include "TermQuery.h"
#include "Term.h"

using namespace Lucene;

typedef LuceneTestFixture CachingSpanFilterTest;

static IndexReaderPtr refreshReader(const IndexReaderPtr& reader) {
    IndexReaderPtr _reader(reader);
    IndexReaderPtr oldReader = _reader;
    _reader = _reader->reopen();
    if (_reader != oldReader) {
        oldReader->close();
    }
    return _reader;
}

TEST_F(CachingSpanFilterTest, testEnforceDeletions) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    IndexReaderPtr reader = writer->getReader();
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);

    // add a doc, refresh the reader, and check that its there
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"id", L"1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    TopDocsPtr docs = searcher->search(newLucene<MatchAllDocsQuery>(), 1);
    EXPECT_EQ(1, docs->totalHits);

    SpanFilterPtr startFilter = newLucene<SpanQueryFilter>(newLucene<SpanTermQuery>(newLucene<Term>(L"id", L"1")));

    // ignore deletions
    CachingSpanFilterPtr filter = newLucene<CachingSpanFilter>(startFilter, CachingWrapperFilter::DELETES_IGNORE);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    EXPECT_EQ(1, docs->totalHits);
    ConstantScoreQueryPtr constantScore = newLucene<ConstantScoreQuery>(filter);
    docs = searcher->search(constantScore, 1);
    EXPECT_EQ(1, docs->totalHits);

    // now delete the doc, refresh the reader, and see that it's not there
    writer->deleteDocuments(newLucene<Term>(L"id", L"1"));

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    EXPECT_EQ(0, docs->totalHits);

    docs = searcher->search(constantScore, 1);
    EXPECT_EQ(1, docs->totalHits);

    // force cache to regenerate
    filter = newLucene<CachingSpanFilter>(startFilter, CachingWrapperFilter::DELETES_RECACHE);

    writer->addDocument(doc);
    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    EXPECT_EQ(1, docs->totalHits);

    constantScore = newLucene<ConstantScoreQuery>(filter);
    docs = searcher->search(constantScore, 1);
    EXPECT_EQ(1, docs->totalHits);

    // make sure we get a cache hit when we reopen readers that had no new deletions
    IndexReaderPtr newReader = refreshReader(reader);
    EXPECT_NE(reader, newReader);
    reader = newReader;
    searcher = newLucene<IndexSearcher>(reader);
    int32_t missCount = filter->missCount;
    docs = searcher->search(constantScore, 1);
    EXPECT_EQ(1, docs->totalHits);
    EXPECT_EQ(missCount, filter->missCount);

    // now delete the doc, refresh the reader, and see that it's not there
    writer->deleteDocuments(newLucene<Term>(L"id", L"1"));

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    EXPECT_EQ(0, docs->totalHits);

    docs = searcher->search(constantScore, 1);
    EXPECT_EQ(0, docs->totalHits);
}
