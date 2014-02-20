/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "TermRangeFilter.h"
#include "TermQuery.h"
#include "Term.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "DateTools.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture DateFilterTest;

TEST_F(DateFilterTest, testBefore) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    int64_t now = MiscUtils::currentTimeMillis();

    DocumentPtr doc = newLucene<Document>();
    // add time that is in the past
    doc->add(newLucene<Field>(L"datefield", DateTools::timeToString(now - 1000, DateTools::RESOLUTION_MILLISECOND), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"body", L"Today is a very sunny day in New York City", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    // filter that should preserve matches
    TermRangeFilterPtr df1 = newLucene<TermRangeFilter>(L"datefield", DateTools::timeToString(now - 2000, DateTools::RESOLUTION_MILLISECOND),
                             DateTools::timeToString(now, DateTools::RESOLUTION_MILLISECOND), false, true);
    // filter that should discard matches
    TermRangeFilterPtr df2 = newLucene<TermRangeFilter>(L"datefield", DateTools::timeToString(0, DateTools::RESOLUTION_MILLISECOND),
                             DateTools::timeToString(now - 2000, DateTools::RESOLUTION_MILLISECOND), true, false);

    // search something that doesn't exist with DateFilter
    QueryPtr query1 = newLucene<TermQuery>(newLucene<Term>(L"body", L"NoMatchForThis"));

    // search for something that does exists
    QueryPtr query2 = newLucene<TermQuery>(newLucene<Term>(L"body", L"sunny"));

    // ensure that queries return expected results without DateFilter first
    Collection<ScoreDocPtr> result = searcher->search(query1, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query2, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, result.size());

    // run queries with DateFilter
    result = searcher->search(query1, df1, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query1, df2, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query2, df1, 1000)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = searcher->search(query2, df2, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());
}

TEST_F(DateFilterTest, testAfter) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    int64_t now = MiscUtils::currentTimeMillis();

    DocumentPtr doc = newLucene<Document>();
    // add time that is in the future
    doc->add(newLucene<Field>(L"datefield", DateTools::timeToString(now + 888888, DateTools::RESOLUTION_MILLISECOND), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"body", L"Today is a very sunny day in New York City", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    // filter that should preserve matches
    TermRangeFilterPtr df1 = newLucene<TermRangeFilter>(L"datefield", DateTools::timeToString(now, DateTools::RESOLUTION_MILLISECOND),
                             DateTools::timeToString(now + 999999, DateTools::RESOLUTION_MILLISECOND), true, false);
    // filter that should discard matches
    TermRangeFilterPtr df2 = newLucene<TermRangeFilter>(L"datefield", DateTools::timeToString(now + 999999, DateTools::RESOLUTION_MILLISECOND),
                             DateTools::timeToString(now + 999999999, DateTools::RESOLUTION_MILLISECOND), false, true);

    // search something that doesn't exist with DateFilter
    QueryPtr query1 = newLucene<TermQuery>(newLucene<Term>(L"body", L"NoMatchForThis"));

    // search for something that does exists
    QueryPtr query2 = newLucene<TermQuery>(newLucene<Term>(L"body", L"sunny"));

    // ensure that queries return expected results without DateFilter first
    Collection<ScoreDocPtr> result = searcher->search(query1, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query2, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, result.size());

    // run queries with DateFilter
    result = searcher->search(query1, df1, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query1, df2, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    result = searcher->search(query2, df1, 1000)->scoreDocs;
    EXPECT_EQ(1, result.size());

    result = searcher->search(query2, df2, 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());
}
