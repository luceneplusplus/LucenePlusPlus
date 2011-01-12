/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "SimpleAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "QueryParser.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "IndexSearcher.h"
#include "Query.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(SearchForDuplicatesTest, LuceneTestFixture)

static const String PRIORITY_FIELD = L"priority";
static const String ID_FIELD = L"id";
static const String HIGH_PRIORITY = L"high";
static const String MED_PRIORITY = L"medium";
static const String LOW_PRIORITY = L"low";

static void printHits(StringStream& out, Collection<ScoreDocPtr> hits, SearcherPtr searcher)
{
    out << hits.size() << L" total results\n";
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        if (i < 10 || (i > 94 && i < 105))
        {
            DocumentPtr doc = searcher->doc(hits[i]->doc);
            out << i << L" " << doc->get(ID_FIELD) << L"\n";
        }
    }
}

static void checkHits(Collection<ScoreDocPtr> hits, int32_t expectedCount, SearcherPtr searcher)
{
    BOOST_CHECK_EQUAL(expectedCount, hits.size());
    for (int32_t i = 0; i < hits.size(); ++i)
    {
        if (i < 10 || (i > 94 && i < 105))
        {
            DocumentPtr doc = searcher->doc(hits[i]->doc);
            BOOST_CHECK_EQUAL(StringUtils::toString(i), doc->get(ID_FIELD));
        }
    }
}

static void doTest(StringStream& out, bool useCompoundFile)
{
    DirectoryPtr directory = newLucene<RAMDirectory>();
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    writer->setUseCompoundFile(useCompoundFile);
    
    int32_t MAX_DOCS = 225;

    for (int32_t j = 0; j < MAX_DOCS; ++j)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(PRIORITY_FIELD, HIGH_PRIORITY, Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(ID_FIELD, StringUtils::toString(j), Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    // try a search without OR
    SearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, PRIORITY_FIELD, analyzer);

    QueryPtr query = parser->parse(HIGH_PRIORITY);
    out << L"Query: " << query->toString(PRIORITY_FIELD) << L"\n";

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), MAX_DOCS)->scoreDocs;
    printHits(out, hits, searcher);
    checkHits(hits, MAX_DOCS, searcher);

    searcher->close();

    // try a new search with OR
    searcher = newLucene<IndexSearcher>(directory, true);
    
    parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, PRIORITY_FIELD, analyzer);

    query = parser->parse(HIGH_PRIORITY + L" OR " + MED_PRIORITY);
    out << L"Query: " << query->toString(PRIORITY_FIELD) << L"\n";

    hits = searcher->search(query, FilterPtr(), MAX_DOCS)->scoreDocs;
    printHits(out, hits, searcher);
    checkHits(hits, MAX_DOCS, searcher);

    searcher->close();
}

BOOST_AUTO_TEST_CASE(testRun)
{
    StringStream multiFileOutput;
    doTest(multiFileOutput, false);
    
    StringStream singleFileOutput;
    doTest(singleFileOutput, true);
    
    BOOST_CHECK_EQUAL(multiFileOutput.str(), singleFileOutput.str());
}

BOOST_AUTO_TEST_SUITE_END()
