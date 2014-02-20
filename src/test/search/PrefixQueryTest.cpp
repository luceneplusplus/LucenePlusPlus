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
#include "PrefixQuery.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture PrefixQueryTest;

TEST_F(PrefixQueryTest, testPrefixQuery) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();

    Collection<String> categories = newCollection<String>(
                                        L"/Computers/Linux",
                                        L"/Computers/Mac",
                                        L"/Computers/Windows"
                                    );
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < categories.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"category", categories[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }

    writer->close();

    PrefixQueryPtr query = newLucene<PrefixQuery>(newLucene<Term>(L"category", L"/Computers"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());

    query = newLucene<PrefixQuery>(newLucene<Term>(L"category", L"/Computers/Mac"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
}
