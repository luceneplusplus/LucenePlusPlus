/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Explanation.h"
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

class SpansAdvanced2Test : public LuceneTestFixture {
public:
    SpansAdvanced2Test() {
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

    virtual ~SpansAdvanced2Test() {
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

    void addDocument(const IndexWriterPtr& writer, const String& id, const String& text) {
        DocumentPtr document = newLucene<Document>();
        document->add(newLucene<Field>(FIELD_ID, id, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        document->add(newLucene<Field>(FIELD_TEXT, text, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(document);
    }

    void checkHits(const SearcherPtr& s, const QueryPtr& query, const String& description, Collection<String> expectedIds, Collection<double> expectedScores) {
        QueryUtils::check(query, s);

        double tolerance = 1e-5f;

        // hits normalizes and throws things off if one score is greater than 1.0
        TopDocsPtr topdocs = s->search(query, FilterPtr(), 10000);

        // did we get the hits we expected
        EXPECT_EQ(expectedIds.size(), topdocs->totalHits);

        for (int32_t i = 0; i < topdocs->totalHits; ++i) {
            int32_t id = topdocs->scoreDocs[i]->doc;
            double score = topdocs->scoreDocs[i]->score;
            DocumentPtr doc = s->doc(id);
            EXPECT_EQ(expectedIds[i], doc->get(FIELD_ID));
            bool scoreEq = (std::abs(expectedScores[i] - score) < tolerance);
            if (scoreEq) {
                EXPECT_NEAR(expectedScores[i], score, tolerance);
                EXPECT_NEAR(s->explain(query, id)->getValue(), score, tolerance);
            }
        }
    }
};

const String SpansAdvanced2Test::FIELD_ID = L"ID";
const String SpansAdvanced2Test::FIELD_TEXT = L"TEXT";

TEST_F(SpansAdvanced2Test, testVerifyIndex) {
    IndexReaderPtr reader = IndexReader::open(directory, true);
    EXPECT_EQ(8, reader->numDocs());
    reader->close();
}

TEST_F(SpansAdvanced2Test, testSingleSpanQuery) {
    QueryPtr spanQuery = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"should"));
    Collection<String> expectedIds = newCollection<String>(L"B", L"D", L"1", L"2", L"3", L"4", L"A");
    Collection<double> expectedScores = newCollection<double>(0.625, 0.45927936, 0.35355338, 0.35355338, 0.35355338, 0.35355338, 0.26516503);
    checkHits(searcher2, spanQuery, L"single span query", expectedIds, expectedScores);
}

TEST_F(SpansAdvanced2Test, testMultipleDifferentSpanQueries) {
    QueryPtr spanQuery1 = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"should"));
    QueryPtr spanQuery2 = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"we"));
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(spanQuery1, BooleanClause::MUST);
    query->add(spanQuery2, BooleanClause::MUST);
    Collection<String> expectedIds = newCollection<String>(L"D", L"A");
    Collection<double> expectedScores = newCollection<double>(1.0191123, 0.93163157);
    checkHits(searcher2, query, L"multiple different span queries", expectedIds, expectedScores);
}

TEST_F(SpansAdvanced2Test, testBooleanQueryWithSpanQueries) {
    double expectedScore = 0.73500174;
    QueryPtr spanQuery = newLucene<SpanTermQuery>(newLucene<Term>(FIELD_TEXT, L"work"));
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(spanQuery, BooleanClause::MUST);
    query->add(spanQuery, BooleanClause::MUST);
    Collection<String> expectedIds = newCollection<String>(L"1", L"2", L"3", L"4");
    Collection<double> expectedScores = newCollection<double>(expectedScore, expectedScore, expectedScore, expectedScore);
    checkHits(searcher2, query, L"two span queries", expectedIds, expectedScores);
}
