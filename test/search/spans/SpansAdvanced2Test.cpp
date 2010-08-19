/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "IndexSearcher.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "SpanTermQuery.h"
#include "Term.h"
#include "BooleanQuery.h"
#include "QueryUtils.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "IndexReader.h"

using namespace Lucene;

class SpansAdvanced2Fixture : public LuceneTestFixture
{
public:
    SpansAdvanced2Fixture()
    {
        // create test index
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        addDocument(writer, L"1", L"I think it should work.");
        addDocument(writer, L"2", L"I think it should work.");
        addDocument(writer, L"3", L"I think it should work.");
        addDocument(writer, L"4", L"I think it should work.");
        addDocument(writer, L"A", L"Should we, could we, would we?");
        addDocument(writer, L"B", L"It should.  Should it?");
        addDocument(writer, L"C", L"It shouldn't.");
        addDocument(writer, L"D", L"Should we, should we, should we.");
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
        
        // re-open the searcher since we added more docs
        searcher2 = newLucene<IndexSearcher>(directory, true);
    }
    
    virtual ~SpansAdvanced2Fixture()
    {
        searcher->close();
        searcher2->close();
        directory->close();
    }

public:
    static const String FIELD_ID;
    static const String FIELD_TEXT;

protected:
    DirectoryPtr directory;
    IndexSearcherPtr searcher;
    IndexSearcherPtr searcher2;
    
    void addDocument(IndexWriterPtr writer, const String& id, const String& text)
    {
        DocumentPtr document = newLucene<Document>();
        document->add(newLucene<Field>(FIELD_ID, id, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        document->add(newLucene<Field>(FIELD_TEXT, text, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(document);
    }
    
    void checkHits(SearcherPtr s, QueryPtr query, const String& description, Collection<String> expectedIds, Collection<double> expectedScores)
    {
        QueryUtils::check(query, s);

        double tolerance = 1e-5f;

        // hits normalizes and throws things off if one score is greater than 1.0
        TopDocsPtr topdocs = s->search(query, FilterPtr(), 10000);

        // did we get the hits we expected
        BOOST_CHECK_EQUAL(expectedIds.size(), topdocs->totalHits);
        
        for (int32_t i = 0; i < topdocs->totalHits; ++i)
        {
            int32_t id = topdocs->scoreDocs[i]->doc;
            double score = topdocs->scoreDocs[i]->score;
            DocumentPtr doc = s->doc(id);
            BOOST_CHECK_EQUAL(expectedIds[i], doc->get(FIELD_ID));
            bool scoreEq = (std::abs(expectedScores[i] - score) < tolerance);
            if (scoreEq)
            {
                BOOST_CHECK_CLOSE_FRACTION(expectedScores[i], score, tolerance);
                BOOST_CHECK_CLOSE_FRACTION(s->explain(query, id)->getValue(), score, tolerance);
            }
        }
    }
};

const String SpansAdvanced2Fixture::FIELD_ID = L"ID";
const String SpansAdvanced2Fixture::FIELD_TEXT = L"TEXT";

BOOST_FIXTURE_TEST_SUITE(SpansAdvanced2Test, SpansAdvanced2Fixture)

BOOST_AUTO_TEST_CASE(testVerifyIndex)
{
    IndexReaderPtr reader = IndexReader::open(directory, true);
    BOOST_CHECK_EQUAL(8, reader->numDocs());
    reader->close();
}

BOOST_AUTO_TEST_CASE(testSingleSpanQuery)
{
    QueryPtr spanQuery = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"should"));
    Collection<String> expectedIds = newCollection<String>(L"B", L"D", L"1", L"2", L"3", L"4", L"A");
    Collection<double> expectedScores = newCollection<double>(0.625, 0.45927936, 0.35355338, 0.35355338, 0.35355338, 0.35355338, 0.26516503);
    checkHits(searcher2, spanQuery, L"single span query", expectedIds, expectedScores);
}

BOOST_AUTO_TEST_CASE(testMultipleDifferentSpanQueries)
{
    QueryPtr spanQuery1 = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"should"));
    QueryPtr spanQuery2 = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"we"));
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(spanQuery1, BooleanClause::MUST);
    query->add(spanQuery2, BooleanClause::MUST);
    Collection<String> expectedIds = newCollection<String>(L"D", L"A");
    Collection<double> expectedScores = newCollection<double>(1.0191123, 0.93163157);
    checkHits(searcher2, query, L"multiple different span queries", expectedIds, expectedScores);
}

BOOST_AUTO_TEST_CASE(testBooleanQueryWithSpanQueries)
{
    double expectedScore = 0.73500174;
    QueryPtr spanQuery = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"work"));
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(spanQuery, BooleanClause::MUST);
    query->add(spanQuery, BooleanClause::MUST);
    Collection<String> expectedIds = newCollection<String>(L"1", L"2", L"3", L"4");
    Collection<double> expectedScores = newCollection<double>(expectedScore, expectedScore, expectedScore, expectedScore);
    checkHits(searcher2, query, L"two span queries", expectedIds, expectedScores);
}

BOOST_AUTO_TEST_SUITE_END()
