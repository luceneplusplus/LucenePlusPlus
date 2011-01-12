/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "ExplanationsFixture.h"
#include "MatchAllDocsQuery.h"
#include "FieldCacheTermsFilter.h"
#include "QueryParser.h"
#include "FilteredQuery.h"
#include "ConstantScoreQuery.h"
#include "DisjunctionMaxQuery.h"
#include "MultiPhraseQuery.h"
#include "BooleanQuery.h"
#include "MockRAMDirectory.h"
#include "Document.h"
#include "Field.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "MultiSearcher.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "SpanNearQuery.h"
#include "SpanQuery.h"
#include "SpanTermQuery.h"
#include "Term.h"

using namespace Lucene;

class ItemizedFilter : public FieldCacheTermsFilter
{
public:
    ItemizedFilter(const String& field, Collection<int32_t> terms) : FieldCacheTermsFilter(field, int2str(terms))
    {
    }
    
    ItemizedFilter(Collection<int32_t> terms) : FieldCacheTermsFilter(L"KEY", int2str(terms))
    {
    }
    
    virtual ~ItemizedFilter()
    {
    }

public:
    Collection<String> int2str(Collection<int32_t> terms)
    {
        Collection<String> out = Collection<String>::newInstance(terms.size());
        for (int32_t i = 0; i < terms.size(); ++i)
            out[i] = StringUtils::toString(terms[i]);
        return out;
    }
};

/// TestExplanations subclass focusing on basic query types
class SimpleExplanationsFixture : public ExplanationsFixture
{
public:
    SimpleExplanationsFixture()
    {
    }
    
    virtual ~SimpleExplanationsFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(SimpleExplanationsTest, SimpleExplanationsFixture)

BOOST_AUTO_TEST_CASE(testT1)
{
    qtest(L"w1", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testT2)
{
    qtest(L"w1^1000", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testMA1)
{
    qtest(newLucene<MatchAllDocsQuery>(), newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testMA2)
{
    QueryPtr q = newLucene<MatchAllDocsQuery>();
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testP1)
{
    qtest(L"\"w1 w2\"", newCollection<int32_t>(0));
}

BOOST_AUTO_TEST_CASE(testP2)
{
    qtest(L"\"w1 w3\"", newCollection<int32_t>(1, 3));
}

BOOST_AUTO_TEST_CASE(testP3)
{
    qtest(L"\"w1 w2\"~1", newCollection<int32_t>(0, 1, 2));
}

BOOST_AUTO_TEST_CASE(testP4)
{
    qtest(L"\"w2 w3\"~1", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testP5)
{
    qtest(L"\"w3 w2\"~1", newCollection<int32_t>(1, 3));
}

BOOST_AUTO_TEST_CASE(testP6)
{
    qtest(L"\"w3 w2\"~2", newCollection<int32_t>(0, 1, 3));
}

BOOST_AUTO_TEST_CASE(testP7)
{
    qtest(L"\"w3 w2\"~3", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testFQ1)
{
    qtest(newLucene<FilteredQuery>(qp->parse(L"w1"), newLucene<ItemizedFilter>(newCollection<int32_t>(0, 1, 2, 3))), newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testFQ2)
{
    qtest(newLucene<FilteredQuery>(qp->parse(L"w1"), newLucene<ItemizedFilter>(newCollection<int32_t>(0, 2, 3))), newCollection<int32_t>(0, 2, 3));
}

BOOST_AUTO_TEST_CASE(testFQ3)
{
    qtest(newLucene<FilteredQuery>(qp->parse(L"xx"), newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3))), newCollection<int32_t>(3));
}

BOOST_AUTO_TEST_CASE(testFQ4)
{
    qtest(newLucene<FilteredQuery>(qp->parse(L"xx^1000"), newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3))), newCollection<int32_t>(3));
}

BOOST_AUTO_TEST_CASE(testFQ6)
{
    QueryPtr q = newLucene<FilteredQuery>(qp->parse(L"xx"), newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3)));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(3));
}

BOOST_AUTO_TEST_CASE(testCSQ1)
{
    QueryPtr q = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(0, 1, 2, 3)));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testCSQ2)
{
    QueryPtr q = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(1, 3)));
    qtest(q, newCollection<int32_t>(1, 3));
}

BOOST_AUTO_TEST_CASE(testCSQ3)
{
    QueryPtr q = newLucene<ConstantScoreQuery>(newLucene<ItemizedFilter>(newCollection<int32_t>(0, 2)));
    q->setBoost(1000);
    qtest(q, newCollection<int32_t>(0, 2));
}

BOOST_AUTO_TEST_CASE(testDMQ1)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.0);
    q->add(qp->parse(L"w1"));
    q->add(qp->parse(L"w5"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ2)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"w1"));
    q->add(qp->parse(L"w5"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ3)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"QQ"));
    q->add(qp->parse(L"w5"));
    qtest(q, newCollection<int32_t>(0));
}

BOOST_AUTO_TEST_CASE(testDMQ4)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"QQ"));
    q->add(qp->parse(L"xx"));
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ5)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"yy -QQ"));
    q->add(qp->parse(L"xx"));
    qtest(q, newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ6)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"-yy w3"));
    q->add(qp->parse(L"xx"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ7)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"-yy w3"));
    q->add(qp->parse(L"w2"));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ8)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"yy w5^100"));
    q->add(qp->parse(L"xx^100000"));
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

BOOST_AUTO_TEST_CASE(testDMQ9)
{
    DisjunctionMaxQueryPtr q = newLucene<DisjunctionMaxQuery>(0.5);
    q->add(qp->parse(L"yy w5^100"));
    q->add(qp->parse(L"xx^0"));
    qtest(q, newCollection<int32_t>(0, 2, 3));
}

BOOST_AUTO_TEST_CASE(testMPQ1)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1")));
    q->add(ta(newCollection<String>(L"w2", L"w3", L"xx")));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testMPQ2)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1")));
    q->add(ta(newCollection<String>(L"w2", L"w3")));
    qtest(q, newCollection<int32_t>(0, 1, 3));
}

BOOST_AUTO_TEST_CASE(testMPQ3)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1", L"xx")));
    q->add(ta(newCollection<String>(L"w2", L"w3")));
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testMPQ4)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1")));
    q->add(ta(newCollection<String>(L"w2")));
    qtest(q, newCollection<int32_t>(0));
}

BOOST_AUTO_TEST_CASE(testMPQ5)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1")));
    q->add(ta(newCollection<String>(L"w2")));
    q->setSlop(1);
    qtest(q, newCollection<int32_t>(0, 1, 2));
}

BOOST_AUTO_TEST_CASE(testMPQ6)
{
    MultiPhraseQueryPtr q = newLucene<MultiPhraseQuery>();
    q->add(ta(newCollection<String>(L"w1", L"w3")));
    q->add(ta(newCollection<String>(L"w2")));
    q->setSlop(1);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ1)
{
    qtest(L"+w1 +w2", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ2)
{
    qtest(L"+yy +w3", newCollection<int32_t>(2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ3)
{
    qtest(L"yy +w3", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ4)
{
    qtest(L"w1 (-xx w2)", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ5)
{
    qtest(L"w1 (+qq w2)", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ6)
{
    qtest(L"w1 -(-qq w5)", newCollection<int32_t>(1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ7)
{
    qtest(L"+w1 +(qq (xx -w2) (+w3 +w4))", newCollection<int32_t>(0));
}

BOOST_AUTO_TEST_CASE(testBQ8)
{
    qtest(L"+w1 (qq (xx -w2) (+w3 +w4))", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ9)
{
    qtest(L"+w1 (qq (-xx w2) -(+w3 +w4))", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ10)
{
    qtest(L"+w1 +(qq (-xx w2) -(+w3 +w4))", newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(testBQ11)
{
    qtest(L"w1 w2^1000.0", newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ14)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>(true);
    q->add(qp->parse(L"QQQQQ"), BooleanClause::SHOULD);
    q->add(qp->parse(L"w1"), BooleanClause::SHOULD);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ15)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>(true);
    q->add(qp->parse(L"QQQQQ"), BooleanClause::MUST_NOT);
    q->add(qp->parse(L"w1"), BooleanClause::SHOULD);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ16)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>(true);
    q->add(qp->parse(L"QQQQQ"), BooleanClause::SHOULD);
    q->add(qp->parse(L"w1 -xx"), BooleanClause::SHOULD);
    qtest(q, newCollection<int32_t>(0, 1));
}

BOOST_AUTO_TEST_CASE(testBQ17)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>(true);
    q->add(qp->parse(L"w2"), BooleanClause::SHOULD);
    q->add(qp->parse(L"w1 -xx"), BooleanClause::SHOULD);
    qtest(q, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testBQ19)
{
    qtest(L"-yy w3", newCollection<int32_t>(0, 1));
}

BOOST_AUTO_TEST_CASE(testBQ20)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->setMinimumNumberShouldMatch(2);
    q->add(qp->parse(L"QQQQQ"), BooleanClause::SHOULD);
    q->add(qp->parse(L"yy"), BooleanClause::SHOULD);
    q->add(qp->parse(L"zz"), BooleanClause::SHOULD);
    q->add(qp->parse(L"w5"), BooleanClause::SHOULD);
    q->add(qp->parse(L"w4"), BooleanClause::SHOULD);
    qtest(q, newCollection<int32_t>(0, 3));
}

BOOST_AUTO_TEST_CASE(testTermQueryMultiSearcherExplain)
{
    // creating two directories for indices
    DirectoryPtr indexStoreA = newLucene<MockRAMDirectory>();
    DirectoryPtr indexStoreB = newLucene<MockRAMDirectory>();

    DocumentPtr lDoc = newLucene<Document>();
    lDoc->add(newLucene<Field>(L"handle", L"1 2", Field::STORE_YES, Field::INDEX_ANALYZED));
    DocumentPtr lDoc2 = newLucene<Document>();
    lDoc2->add(newLucene<Field>(L"handle", L"1 2", Field::STORE_YES, Field::INDEX_ANALYZED));
    DocumentPtr lDoc3 = newLucene<Document>();
    lDoc3->add(newLucene<Field>(L"handle", L"1 2", Field::STORE_YES, Field::INDEX_ANALYZED));

    IndexWriterPtr writerA = newLucene<IndexWriter>(indexStoreA, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    IndexWriterPtr writerB = newLucene<IndexWriter>(indexStoreB, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

    writerA->addDocument(lDoc);
    writerA->addDocument(lDoc2);
    writerA->optimize();
    writerA->close();

    writerB->addDocument(lDoc3);
    writerB->close();

    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"fulltext", newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    QueryPtr query = parser->parse(L"handle:1");

    Collection<SearchablePtr> searchers = newCollection<SearchablePtr>(
        newLucene<IndexSearcher>(indexStoreB, true),
        newLucene<IndexSearcher>(indexStoreA, true)
    );
    SearcherPtr mSearcher = newLucene<MultiSearcher>(searchers);
    Collection<ScoreDocPtr> hits = mSearcher->search(query, FilterPtr(), 1000)->scoreDocs;

    BOOST_CHECK_EQUAL(3, hits.size());

    ExplanationPtr explain = mSearcher->explain(query, hits[0]->doc);
    String exp = explain->toString();
    BOOST_CHECK(exp.find(L"maxDocs=3") != String::npos);
    BOOST_CHECK(exp.find(L"docFreq=3") != String::npos);

    query = parser->parse(L"handle:\"1 2\"");
    hits = mSearcher->search(query, FilterPtr(), 1000)->scoreDocs;

    BOOST_CHECK_EQUAL(3, hits.size());

    explain = mSearcher->explain(query, hits[0]->doc);
    exp = explain->toString();
    BOOST_CHECK(exp.find(L"1=3") != String::npos);
    BOOST_CHECK(exp.find(L"2=3") != String::npos);

    query = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<SpanTermQuery>(newLucene<Term>(L"handle", L"1")), newLucene<SpanTermQuery>(newLucene<Term>(L"handle", L"2"))), 0, true);
    hits = mSearcher->search(query, FilterPtr(), 1000)->scoreDocs;

    BOOST_CHECK_EQUAL(3, hits.size());

    explain = mSearcher->explain(query, hits[0]->doc);
    exp = explain->toString();
    BOOST_CHECK(exp.find(L"1=3") != String::npos);
    BOOST_CHECK(exp.find(L"2=3") != String::npos);
    mSearcher->close();
}

BOOST_AUTO_TEST_SUITE_END()
