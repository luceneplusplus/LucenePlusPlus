/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "StandardAnalyzer.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MatchAllDocsQuery.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "QueryParser.h"
#include "Document.h"
#include "Field.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(MatchAllDocsQueryTest, LuceneTestFixture)

static void addDoc(const String& text, IndexWriterPtr iw, double boost)
{
    DocumentPtr doc = newLucene<Document>();
    FieldPtr f = newLucene<Field>(L"key", text, Field::STORE_YES, Field::INDEX_ANALYZED);
    f->setBoost(boost);
    doc->add(f);
    iw->addDocument(doc);
}

BOOST_AUTO_TEST_CASE(testQuery)
{
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr iw = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    iw->setMaxBufferedDocs(2); // force multi-segment
    addDoc(L"one", iw, 1);
    addDoc(L"two", iw, 20);
    addDoc(L"three four", iw, 300);
    iw->close();

    IndexReaderPtr ir = IndexReader::open(dir, false);
    IndexSearcherPtr is = newLucene<IndexSearcher>(ir);

    // assert with norms scoring turned off
    Collection<ScoreDocPtr> hits = is->search(newLucene<MatchAllDocsQuery>(), FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(3, hits.size());
    BOOST_CHECK_EQUAL(L"one", ir->document(hits[0]->doc)->get(L"key"));
    BOOST_CHECK_EQUAL(L"two", ir->document(hits[1]->doc)->get(L"key"));
    BOOST_CHECK_EQUAL(L"three four", ir->document(hits[2]->doc)->get(L"key"));

    // assert with norms scoring turned on
    MatchAllDocsQueryPtr normsQuery = newLucene<MatchAllDocsQuery>(L"key");
    hits = is->search(normsQuery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(3, hits.size());

    BOOST_CHECK_EQUAL(L"three four", ir->document(hits[0]->doc)->get(L"key"));    
    BOOST_CHECK_EQUAL(L"two", ir->document(hits[1]->doc)->get(L"key"));
    BOOST_CHECK_EQUAL(L"one", ir->document(hits[2]->doc)->get(L"key"));

    // change norm & retest
    ir->setNorm(0, L"key", 400.0);
    normsQuery = newLucene<MatchAllDocsQuery>(L"key");
    hits = is->search(normsQuery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(3, hits.size());

    BOOST_CHECK_EQUAL(L"one", ir->document(hits[0]->doc)->get(L"key"));
    BOOST_CHECK_EQUAL(L"three four", ir->document(hits[1]->doc)->get(L"key"));    
    BOOST_CHECK_EQUAL(L"two", ir->document(hits[2]->doc)->get(L"key"));

    // some artificial queries to trigger the use of skipTo()
    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    bq->add(newLucene<MatchAllDocsQuery>(), BooleanClause::MUST);
    bq->add(newLucene<MatchAllDocsQuery>(), BooleanClause::MUST);
    hits = is->search(bq, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(3, hits.size());

    bq = newLucene<BooleanQuery>();
    bq->add(newLucene<MatchAllDocsQuery>(), BooleanClause::MUST);
    bq->add(newLucene<TermQuery>(newLucene<Term>(L"key", L"three")), BooleanClause::MUST);
    hits = is->search(bq, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());

    // delete a document
    is->getIndexReader()->deleteDocument(0);
    hits = is->search(newLucene<MatchAllDocsQuery>(), FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(2, hits.size());

    // test parsable toString()
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"key", analyzer);
    hits = is->search(qp->parse(newLucene<MatchAllDocsQuery>()->toString()), FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(2, hits.size());

    // test parsable toString() with non default boost
    QueryPtr maq = newLucene<MatchAllDocsQuery>();
    maq->setBoost(2.3);
    QueryPtr pq = qp->parse(maq->toString());
    hits = is->search(pq, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(2, hits.size());

    is->close();
    ir->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testEquals)
{
    QueryPtr q1 = newLucene<MatchAllDocsQuery>();
    QueryPtr q2 = newLucene<MatchAllDocsQuery>();
    BOOST_CHECK(q1->equals(q2));
    q1->setBoost(1.5);
    BOOST_CHECK(!q1->equals(q2));
}

BOOST_AUTO_TEST_SUITE_END()
