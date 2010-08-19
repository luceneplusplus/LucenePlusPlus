/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "Filter.h"
#include "BitSet.h"
#include "DocIdBitSet.h"
#include "FilteredQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "QueryUtils.h"
#include "Sort.h"
#include "SortField.h"
#include "BooleanQuery.h"
#include "TermRangeQuery.h"
#include "TopFieldDocs.h"
#include "MatchAllDocsQuery.h"

using namespace Lucene;

class StaticFilterA : public Filter
{
public:
    virtual ~StaticFilterA()
    {
    }

public:
    virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
    {
        BitSetPtr bitset = newLucene<BitSet>(5);
        bitset->set((uint32_t)0, (uint32_t)5);
        return newLucene<DocIdBitSet>(bitset);
    }
};

class StaticFilterB : public Filter
{
public:
    virtual ~StaticFilterB()
    {
    }

public:
    virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
    {
        BitSetPtr bitset = newLucene<BitSet>(5);
        bitset->set(1);
        bitset->set(3);
        return newLucene<DocIdBitSet>(bitset);
    }
};

class SingleDocTestFilter : public Filter
{
public:
    SingleDocTestFilter(int32_t doc)
    {
        this->doc = doc;
    }
    
    virtual ~SingleDocTestFilter()
    {
    }

protected:
    int32_t doc;

public:
    virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
    {
        BitSetPtr bits = newLucene<BitSet>(reader->maxDoc());
        bits->set(doc);
        return newLucene<DocIdBitSet>(bits);
    }
};

class FilteredQueryFixture : public LuceneTestFixture
{
public:
    FilteredQueryFixture()
    {
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"one two three four five", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"sorter", L"b", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"one two three four", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"sorter", L"d", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"one two three y", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"sorter", L"a", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"one two x", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"sorter", L"c", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        writer->optimize();
        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
        query = newLucene<TermQuery>(newLucene<Term>(L"field", L"three"));
        filter = newStaticFilterB();
    }
    
    virtual ~FilteredQueryFixture()
    {
        searcher->close();
        directory->close();
    }

protected:
    IndexSearcherPtr searcher;
    RAMDirectoryPtr directory;
    QueryPtr query;
    FilterPtr filter;

public:
    FilterPtr newStaticFilterA()
    {
        return newLucene<StaticFilterA>();
    }
    
    FilterPtr newStaticFilterB()
    {
        return newLucene<StaticFilterB>();
    }
    
    void checkScoreEquals(QueryPtr q1, QueryPtr q2)
    {
        Collection<ScoreDocPtr> hits1 = searcher->search(q1, FilterPtr(), 1000)->scoreDocs;
        Collection<ScoreDocPtr> hits2 = searcher->search (q2, FilterPtr(), 1000)->scoreDocs;

        BOOST_CHECK_EQUAL(hits1.size(), hits2.size());
        
        for (int32_t i = 0; i < hits1.size(); ++i)
            BOOST_CHECK_CLOSE_FRACTION(hits1[i]->score, hits2[i]->score, 0.0000001);
    }
};

BOOST_FIXTURE_TEST_SUITE(FilteredQueryTest, FilteredQueryFixture)

BOOST_AUTO_TEST_CASE(testFilteredQuery)
{
    QueryPtr filteredquery = newLucene<FilteredQuery>(query, filter);
    Collection<ScoreDocPtr> hits = searcher->search(filteredquery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
    BOOST_CHECK_EQUAL(1, hits[0]->doc);
    QueryUtils::check(filteredquery, searcher);

    hits = searcher->search(filteredquery, FilterPtr(), 1000, newLucene<Sort>(newLucene<SortField>(L"sorter", SortField::STRING)))->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
    BOOST_CHECK_EQUAL(1, hits[0]->doc);

    filteredquery = newLucene<FilteredQuery>(newLucene<TermQuery>(newLucene<Term>(L"field", L"one")), filter);
    hits = searcher->search(filteredquery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(2, hits.size());
    QueryUtils::check(filteredquery, searcher);
    
    filteredquery = newLucene<FilteredQuery>(newLucene<TermQuery>(newLucene<Term>(L"field", L"x")), filter);
    hits = searcher->search(filteredquery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
    BOOST_CHECK_EQUAL(3, hits[0]->doc);
    QueryUtils::check(filteredquery, searcher);

    filteredquery = newLucene<FilteredQuery>(newLucene<TermQuery>(newLucene<Term>(L"field", L"y")), filter);
    hits = searcher->search(filteredquery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(0, hits.size());
    QueryUtils::check(filteredquery, searcher);

    // test boost
    FilterPtr f = newStaticFilterA();

    double boost = 2.5;
    BooleanQueryPtr bq1 = newLucene<BooleanQuery>();
    TermQueryPtr tq = newLucene<TermQuery>(newLucene<Term>(L"field", L"one"));
    tq->setBoost(boost);
    bq1->add(tq, BooleanClause::MUST);
    bq1->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"five")), BooleanClause::MUST);

    BooleanQueryPtr bq2 = newLucene<BooleanQuery>();
    tq = newLucene<TermQuery>(newLucene<Term>(L"field", L"one"));
    filteredquery = newLucene<FilteredQuery>(tq, f);
    filteredquery->setBoost(boost);
    bq2->add(filteredquery, BooleanClause::MUST);
    bq2->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"five")), BooleanClause::MUST);
    checkScoreEquals(bq1, bq2);

    BOOST_CHECK_EQUAL(boost, filteredquery->getBoost());
    BOOST_CHECK_EQUAL(1.0, tq->getBoost()); // the boost value of the underlying query shouldn't have changed 
}

BOOST_AUTO_TEST_CASE(testRangeQuery)
{
    TermRangeQueryPtr rq = newLucene<TermRangeQuery>(L"sorter", L"b", L"d", true, true);

    QueryPtr filteredquery = newLucene<FilteredQuery>(rq, filter);
    Collection<ScoreDocPtr> hits = searcher->search(filteredquery, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(2, hits.size());
    QueryUtils::check(filteredquery, searcher);
}

BOOST_AUTO_TEST_CASE(testBoolean)
{
    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    QueryPtr query = newLucene<FilteredQuery>(newLucene<MatchAllDocsQuery>(), newLucene<SingleDocTestFilter>(0));
    bq->add(query, BooleanClause::MUST);
    query = newLucene<FilteredQuery>(newLucene<MatchAllDocsQuery>(), newLucene<SingleDocTestFilter>(1));
    bq->add(query, BooleanClause::MUST);
    Collection<ScoreDocPtr> hits = searcher->search(bq, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(0, hits.size());
    QueryUtils::check(query, searcher);
}

/// Make sure BooleanQuery, which does out-of-order scoring, inside FilteredQuery, works
BOOST_AUTO_TEST_CASE(testBoolean2)
{
    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    QueryPtr query = newLucene<FilteredQuery>(bq, newLucene<SingleDocTestFilter>(0));
    bq->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"one")), BooleanClause::SHOULD);
    bq->add(newLucene<TermQuery>(newLucene<Term>(L"field", L"two")), BooleanClause::SHOULD);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
    QueryUtils::check(query, searcher);
}

BOOST_AUTO_TEST_SUITE_END()
