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
#include "IndexSearcher.h"
#include "MultiPhraseQuery.h"
#include "Term.h"
#include "IndexReader.h"
#include "TermEnum.h"
#include "Document.h"
#include "Field.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TermQuery.h"
#include "BooleanQuery.h"
#include "StandardAnalyzer.h"

using namespace Lucene;

typedef LuceneTestFixture MultiPhraseQueryTest;

static void add(const String& s, const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"body", s, Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void add(const String& s, const String& type, const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"body", s, Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"type", type, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);
}

TEST_F(MultiPhraseQueryTest, testPhrasePrefix) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    add(L"blueberry pie", writer);
    add(L"blueberry strudel", writer);
    add(L"blueberry pizza", writer);
    add(L"blueberry chewing gum", writer);
    add(L"bluebird pizza", writer);
    add(L"bluebird foobar pizza", writer);
    add(L"piccadilly circus", writer);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    // search for "blueberry pi*"
    MultiPhraseQueryPtr query1 = newLucene<MultiPhraseQuery>();
    // search for "strawberry pi*"
    MultiPhraseQueryPtr query2 = newLucene<MultiPhraseQuery>();
    query1->add(newLucene<Term>(L"body", L"blueberry"));
    query2->add(newLucene<Term>(L"body", L"strawberry"));

    Collection<TermPtr> termsWithPrefix = Collection<TermPtr>::newInstance();
    IndexReaderPtr ir = IndexReader::open(indexStore, true);

    // this TermEnum gives "piccadilly", "pie" and "pizza".
    String prefix = L"pi";
    TermEnumPtr te = ir->terms(newLucene<Term>(L"body", prefix));
    do {
        if (boost::starts_with(te->term()->text(), prefix)) {
            termsWithPrefix.add(te->term());
        }
    } while (te->next());

    query1->add(termsWithPrefix);
    EXPECT_EQ(L"body:\"blueberry (piccadilly pie pizza)\"", query1->toString());
    query2->add(termsWithPrefix);
    EXPECT_EQ(L"body:\"strawberry (piccadilly pie pizza)\"", query2->toString());

    Collection<ScoreDocPtr> result = searcher->search(query1, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, result.size());
    result = searcher->search(query2, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, result.size());

    // search for "blue* pizza"
    MultiPhraseQueryPtr query3 = newLucene<MultiPhraseQuery>();
    termsWithPrefix.clear();
    prefix = L"blue";
    te = ir->terms(newLucene<Term>(L"body", prefix));
    do {
        if (boost::starts_with(te->term()->text(), prefix)) {
            termsWithPrefix.add(te->term());
        }
    } while (te->next());

    query3->add(termsWithPrefix);
    query3->add(newLucene<Term>(L"body", L"pizza"));

    result = searcher->search(query3, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, result.size()); // blueberry pizza, bluebird pizza
    EXPECT_EQ(L"body:\"(blueberry bluebird) pizza\"", query3->toString());

    // test slop
    query3->setSlop(1);
    result = searcher->search(query3, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, result.size()); // blueberry pizza, bluebird pizza, bluebird foobar pizza

    MultiPhraseQueryPtr query4 = newLucene<MultiPhraseQuery>();
    query4->add(newLucene<Term>(L"field1", L"foo"));
    try {
        query4->add(newLucene<Term>(L"field2", L"foobar"));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }

    searcher->close();
    indexStore->close();
}

TEST_F(MultiPhraseQueryTest, testBooleanQueryContainingSingleTermPrefixQuery) {
    // In order to cause the bug, the outer query must have more than one term and all terms required.
    // The contained PhraseMultiQuery must contain exactly one term array.

    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    add(L"blueberry pie", writer);
    add(L"blueberry chewing gum", writer);
    add(L"blue raspberry pie", writer);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);
    // This query will be equivalent to +body:pie +body:"blue*"
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"body", L"pie")), BooleanClause::MUST);

    MultiPhraseQueryPtr mpq = newLucene<MultiPhraseQuery>();
    mpq->add(newCollection<TermPtr>(newLucene<Term>(L"body", L"blueberry"), newLucene<Term>(L"body", L"blue")));
    q->add(mpq, BooleanClause::MUST);

    Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(2, hits.size());
    searcher->close();
}

TEST_F(MultiPhraseQueryTest, testPhrasePrefixWithBooleanQuery) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT, HashSet<String>::newInstance()), true, IndexWriter::MaxFieldLengthLIMITED);
    add(L"This is a test", L"object", writer);
    add(L"a note", L"note", writer);
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    // This query will be equivalent to +type:note +body:"a t*"
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"type", L"note")), BooleanClause::MUST);

    MultiPhraseQueryPtr mpq = newLucene<MultiPhraseQuery>();
    mpq->add(newLucene<Term>(L"body", L"a"));
    mpq->add(newCollection<TermPtr>(newLucene<Term>(L"body", L"test"), newLucene<Term>(L"body", L"this")));
    q->add(mpq, BooleanClause::MUST);

    Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(0, hits.size());
    searcher->close();
}

TEST_F(MultiPhraseQueryTest, testHashCodeAndEquals) {
    MultiPhraseQueryPtr query1 = newLucene<MultiPhraseQuery>();
    MultiPhraseQueryPtr query2 = newLucene<MultiPhraseQuery>();

    EXPECT_EQ(query1->hashCode(), query2->hashCode());
    EXPECT_TRUE(query1->equals(query2));

    TermPtr term1 = newLucene<Term>(L"someField", L"someText");

    query1->add(term1);
    query2->add(term1);

    EXPECT_EQ(query1->hashCode(), query2->hashCode());
    EXPECT_TRUE(query1->equals(query2));

    TermPtr term2 = newLucene<Term>(L"someField", L"someMoreText");

    query1->add(term2);

    EXPECT_NE(query1->hashCode(), query2->hashCode());
    EXPECT_TRUE(!query1->equals(query2));

    query2->add(term2);

    EXPECT_EQ(query1->hashCode(), query2->hashCode());
    EXPECT_TRUE(query1->equals(query2));
}
