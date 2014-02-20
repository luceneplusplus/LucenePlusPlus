/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "MultiPhraseQuery.h"
#include "Term.h"
#include "TermEnum.h"
#include "IndexReader.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture PhrasePrefixQueryTest;

TEST_F(PhrasePrefixQueryTest, testPhrasePrefix) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc1 = newLucene<Document>();
    DocumentPtr doc2 = newLucene<Document>();
    DocumentPtr doc3 = newLucene<Document>();
    DocumentPtr doc4 = newLucene<Document>();
    DocumentPtr doc5 = newLucene<Document>();
    doc1->add(newLucene<Field>(L"body", L"blueberry pie", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc2->add(newLucene<Field>(L"body", L"blueberry strudel", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc3->add(newLucene<Field>(L"body", L"blueberry pizza", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc4->add(newLucene<Field>(L"body", L"blueberry chewing gum", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc5->add(newLucene<Field>(L"body", L"piccadilly circus", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc1);
    writer->addDocument(doc2);
    writer->addDocument(doc3);
    writer->addDocument(doc4);
    writer->addDocument(doc5);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    MultiPhraseQueryPtr query1 = newLucene<MultiPhraseQuery>();
    MultiPhraseQueryPtr query2 = newLucene<MultiPhraseQuery>();
    query1->add(newLucene<Term>(L"body", L"blueberry"));
    query2->add(newLucene<Term>(L"body", L"strawberry"));

    Collection<TermPtr> termsWithPrefix = Collection<TermPtr>::newInstance();
    IndexReaderPtr ir = IndexReader::open(indexStore, true);

    // this TermEnum gives "piccadilly", "pie" and "pizza".
    String prefix = L"pi";
    TermEnumPtr te = ir->terms(newLucene<Term>(L"body", prefix + L"*"));
    do {
        if (boost::starts_with(te->term()->text(), prefix)) {
            termsWithPrefix.add(te->term());
        }
    } while (te->next());

    query1->add(termsWithPrefix);
    query2->add(termsWithPrefix);

    Collection<ScoreDocPtr> result = searcher->search(query1, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, result.size());

    result = searcher->search(query2, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());
}
