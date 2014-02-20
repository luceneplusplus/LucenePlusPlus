/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "PrefixFilter.h"
#include "Term.h"
#include "ConstantScoreQuery.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture PrefixFilterTest;

TEST_F(PrefixFilterTest, testPrefixFilter) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();

    Collection<String> categories = newCollection<String>(
                                        L"/Computers/Linux",
                                        L"/Computers/Mac/One",
                                        L"/Computers/Mac/Two",
                                        L"/Computers/Windows"
                                    );
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < categories.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"category", categories[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }

    writer->close();

    // PrefixFilter combined with ConstantScoreQuery
    PrefixFilterPtr filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers"));
    QueryPtr query = newLucene<ConstantScoreQuery>(filter);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(4, hits.size());

    // test middle of values
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/Mac"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());

    // test start of values
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/Linux"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // test end of values
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/Windows"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // test non-existent
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/ObsoleteOS"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // test non-existent, before values
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/AAA"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // test non-existent, after values
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L"/Computers/ZZZ"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // test zero length prefix
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"category", L""));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(4, hits.size());

    // test non existent field
    filter = newLucene<PrefixFilter>(newLucene<Term>(L"nonexistentfield", L"/Computers"));
    query = newLucene<ConstantScoreQuery>(filter);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
}
