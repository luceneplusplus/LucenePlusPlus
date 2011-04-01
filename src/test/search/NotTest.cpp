/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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
#include "QueryParser.h"
#include "Query.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(NotTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testNot)
{
    RAMDirectoryPtr store = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(store, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr d1 = newLucene<Document>();
    d1->add(newLucene<Field>(L"field", L"a b", Field::STORE_YES, Field::INDEX_ANALYZED));

    writer->addDocument(d1);
    writer->optimize();
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(store, true);
    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<SimpleAnalyzer>());
    QueryPtr query = parser->parse(L"a NOT b");
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(0, hits.size());
}

BOOST_AUTO_TEST_SUITE_END()
