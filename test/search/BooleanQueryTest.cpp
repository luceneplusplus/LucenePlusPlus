/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "PhraseQuery.h"
#include "DisjunctionMaxQuery.h"
#include "TopDocs.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(BooleanQueryTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testEquality)
{
    BooleanQueryPtr bq1 = newLucene<BooleanQuery>();
    bq1->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"value1")), BooleanClause::SHOULD);
    bq1->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"value2")), BooleanClause::SHOULD);
    BooleanQueryPtr nested1 = newLucene<BooleanQuery>();
    nested1->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"nestedvalue1")), BooleanClause::SHOULD);
    nested1->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"nestedvalue2")), BooleanClause::SHOULD);
    bq1->add(nested1, BooleanClause::SHOULD);

    BooleanQueryPtr bq2 = newLucene<BooleanQuery>();
    bq2->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"value1")), BooleanClause::SHOULD);
    bq2->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"value2")), BooleanClause::SHOULD);
    BooleanQueryPtr nested2 = newLucene<BooleanQuery>();
    nested2->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"nestedvalue1")), BooleanClause::SHOULD);
    nested2->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"nestedvalue2")), BooleanClause::SHOULD);
    bq2->add(nested2, BooleanClause::SHOULD);

    BOOST_CHECK(bq1->equals(bq2));
}

BOOST_AUTO_TEST_CASE(testException)
{
    BOOST_CHECK_EXCEPTION(BooleanQuery::setMaxClauseCount(0), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
}

BOOST_AUTO_TEST_CASE(testNullOrSubScorer)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c d", Field::STORE_NO, Field::INDEX_ANALYZED));
    w->addDocument(doc);
    IndexReaderPtr r = w->getReader();
    IndexSearcherPtr s = newLucene<IndexSearcher>(r);
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"a")), BooleanClause::SHOULD);

    // PhraseQuery with no terms added returns a null scorer
    PhraseQueryPtr pq = newLucene<PhraseQuery>();
    q->add(pq, BooleanClause::SHOULD);
    BOOST_CHECK_EQUAL(1, s->search(q, 10)->totalHits);

    // A required clause which returns null scorer should return null scorer to IndexSearcher.
    q = newLucene<BooleanQuery>();
    pq = newLucene<PhraseQuery>();
    q->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"a")), BooleanClause::SHOULD);
    q->add(pq, BooleanClause::MUST);
    BOOST_CHECK_EQUAL(0, s->search(q, 10)->totalHits);

    DisjunctionMaxQueryPtr dmq = newLucene<DisjunctionMaxQuery>(1.0);
    dmq->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"a")));
    dmq->add(pq);
    BOOST_CHECK_EQUAL(1, s->search(dmq, 10)->totalHits);

    r->close();
    w->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
