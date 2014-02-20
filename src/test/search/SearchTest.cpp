/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef LuceneTestFixture SearchTest;

static void doTestSearch(StringStream& out, bool useCompoundFile) {
    DirectoryPtr directory = newLucene<RAMDirectory>();
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

    writer->setUseCompoundFile(useCompoundFile);
    Collection<String> docs = newCollection<String>(
                                  L"a b c d e",
                                  L"a b c d e a b c d e",
                                  L"a b c d e f g h i j",
                                  L"a c e",
                                  L"e c a",
                                  L"a c e a c e",
                                  L"a c e a b c"
                              );

    for (int32_t j = 0; j < docs.size(); ++j) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"contents", docs[j], Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    Collection<String> queries = newCollection<String>(
                                     L"a b",
                                     L"\"a b\"",
                                     L"\"a b c\"",
                                     L"a c",
                                     L"\"a c\"",
                                     L"\"a c e\""
                                 );

    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);
    parser->setPhraseSlop(4);
    for (int32_t j = 0; j < queries.size(); ++j) {
        QueryPtr query = parser->parse(queries[j]);
        out << L"Query: " << query->toString(L"contents") << L"\n";

        Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;

        out << hits.size() << L" total results\n";
        for (int32_t i = 0; i < hits.size() && i < 10; ++i) {
            DocumentPtr doc = searcher->doc(hits[i]->doc);
            out << i << L" " << hits[i]->score << L" " + doc->get(L"contents") << L"\n";
        }
    }
    searcher->close();
}

TEST_F(SearchTest, testSearch) {
    StringStream multiFileOutput;
    doTestSearch(multiFileOutput, false);

    StringStream singleFileOutput;
    doTestSearch(singleFileOutput, true);

    EXPECT_EQ(multiFileOutput.str(), singleFileOutput.str());
}
