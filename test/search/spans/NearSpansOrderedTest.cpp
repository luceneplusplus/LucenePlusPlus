/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "QueryParser.h"
#include "IndexSearcher.h"
#include "WhitespaceAnalyzer.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "SpanNearQuery.h"
#include "CheckHits.h"
#include "SpanTermQuery.h"
#include "Term.h"
#include "Spans.h"
#include "Weight.h"
#include "Scorer.h"
#include "Explanation.h"

using namespace Lucene;

class NearSpansOrderedFixture : public LuceneTestFixture
{
public:
    NearSpansOrderedFixture()
    {
        qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, FIELD, newLucene<WhitespaceAnalyzer>());
        docFields = newCollection<String>(L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3 zz", L"w1 xx w2 yy w3", L"w1 w3 xx w2 yy w3 zz");
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer= newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < docFields.size(); ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, docFields[i], Field::STORE_NO, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }
    
    virtual ~NearSpansOrderedFixture()
    {
        searcher->close();
    }

protected:
    IndexSearcherPtr searcher;
    QueryParserPtr qp;
    Collection<String> docFields;
    
public:
    static const String FIELD;

public:
    SpanNearQueryPtr makeQuery(const String& s1, const String& s2, const String& s3, int32_t slop, bool inOrder)
    {
        return newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(
            newLucene<SpanTermQuery>(newLucene<Term>(FIELD, s1)),
            newLucene<SpanTermQuery>(newLucene<Term>(FIELD, s2)),
            newLucene<SpanTermQuery>(newLucene<Term>(FIELD, s3))
        ), slop, inOrder);
    }
                                    
    SpanNearQueryPtr makeQuery()
    {
        return makeQuery(L"w1", L"w2", L"w3", 1, true);
    }
    
    String str(SpansPtr span)
    {
        return str(span->doc(), span->start(), span->end());
    }
    
    String str(int32_t doc, int32_t start, int32_t end)
    {
        return L"s(" + StringUtils::toString(doc) + L"," + StringUtils::toString(start) + L"," + StringUtils::toString(end) + L")";
    }
};

const String NearSpansOrderedFixture::FIELD = L"field";

BOOST_FIXTURE_TEST_SUITE(NearSpansOrderedTest, NearSpansOrderedFixture)

BOOST_AUTO_TEST_CASE(testSpanNearQuery)
{
    SpanNearQueryPtr q = makeQuery();
    CheckHits::checkHits(q, FIELD, searcher, newCollection<int32_t>(0, 1));
}

BOOST_AUTO_TEST_CASE(testNearSpansNext)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->next());
    BOOST_CHECK_EQUAL(str(0, 0, 3), str(span));
    BOOST_CHECK(span->next());
    BOOST_CHECK_EQUAL(str(1, 0, 4), str(span));
    BOOST_CHECK(!span->next());
}

/// test does not imply that skipTo(doc+1) should work exactly the same as next -- it's only applicable in this case 
/// since we know doc does not contain more than one span
BOOST_AUTO_TEST_CASE(testNearSpansSkipToLikeNext)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->skipTo(0));
    BOOST_CHECK_EQUAL(str(0, 0, 3), str(span));
    BOOST_CHECK(span->skipTo(1));
    BOOST_CHECK_EQUAL(str(1, 0, 4), str(span));
    BOOST_CHECK(!span->skipTo(2));
}

BOOST_AUTO_TEST_CASE(testNearSpansNextThenSkipTo)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->next());
    BOOST_CHECK_EQUAL(str(0, 0, 3), str(span));
    BOOST_CHECK(span->skipTo(1));
    BOOST_CHECK_EQUAL(str(1, 0, 4), str(span));
    BOOST_CHECK(!span->next());
}

BOOST_AUTO_TEST_CASE(testNearSpansNextThenSkipPast)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->next());
    BOOST_CHECK_EQUAL(str(0, 0, 3), str(span));
    BOOST_CHECK(!span->skipTo(2));
}

BOOST_AUTO_TEST_CASE(testNearSpansSkipPast)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(!span->skipTo(2));
}

BOOST_AUTO_TEST_CASE(testNearSpansSkipTo0)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->skipTo(0));
    BOOST_CHECK_EQUAL(str(0, 0, 3), str(span));
}

BOOST_AUTO_TEST_CASE(testNearSpansSkipTo1)
{
    SpanNearQueryPtr q = makeQuery();
    SpansPtr span = q->getSpans(searcher->getIndexReader());
    BOOST_CHECK(span->skipTo(1));
    BOOST_CHECK_EQUAL(str(1, 0, 4), str(span));
}

BOOST_AUTO_TEST_CASE(testSpanNearScorerSkipTo1)
{
    SpanNearQueryPtr q = makeQuery();
    WeightPtr w = q->weight(searcher);
    ScorerPtr s = w->scorer(searcher->getIndexReader(), true, false);
    BOOST_CHECK_EQUAL(1, s->advance(1));
}

BOOST_AUTO_TEST_CASE(testSpanNearScorerExplain)
{
    SpanNearQueryPtr q = makeQuery();
    ExplanationPtr e = q->weight(searcher)->explain(searcher->getIndexReader(), 1);
    BOOST_CHECK(0.0 < e->getValue());
}

BOOST_AUTO_TEST_SUITE_END()
