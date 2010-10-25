/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "IndexSearcher.h"
#include "WhitespaceAnalyzer.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "SpanTermQuery.h"
#include "Term.h"
#include "CheckHits.h"
#include "SpanQuery.h"
#include "SpanNearQuery.h"
#include "Spans.h"
#include "SpanOrQuery.h"
#include "DefaultSimilarity.h"
#include "Scorer.h"
#include "DocIdSetIterator.h"
#include "Weight.h"
#include "StandardAnalyzer.h"
#include "TermQuery.h"
#include "TopDocs.h"
#include "IndexReader.h"

using namespace Lucene;

class SpansFixture : public LuceneTestFixture
{
public:
    SpansFixture()
    {
        docFields = Collection<String>::newInstance();
        docFields.add(L"w1 w2 w3 w4 w5");
        docFields.add(L"w1 w3 w2 w3");
        docFields.add(L"w1 xx w2 yy w3");
        docFields.add(L"w1 w3 xx w2 yy w3");
        docFields.add(L"u2 u2 u1");
        docFields.add(L"u2 xx u2 u1");
        docFields.add(L"u2 u2 xx u1");
        docFields.add(L"u2 xx u2 yy u1");
        docFields.add(L"u2 xx u1 u2");
        docFields.add(L"u2 u1 xx u2");
        docFields.add(L"u1 u2 xx u2");
        docFields.add(L"t1 t2 t1 t3 t2 t3");
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer= newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < docFields.size(); ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(field, docFields[i], Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }
    
    virtual ~SpansFixture()
    {
        searcher->close();
    }

protected:
    IndexSearcherPtr searcher;
    Collection<String> docFields;
    
public:
    static const String field;

public:
    SpanTermQueryPtr makeSpanTermQuery(const String& text)
    {
        return newLucene<SpanTermQuery>(newLucene<Term>(field, text));
    }
    
    void checkHits(QueryPtr query, Collection<int32_t> results)
    {
        CheckHits::checkHits(query, field, searcher, results);
    }
    
    void orderedSlopTest3SQ(SpanQueryPtr q1, SpanQueryPtr q2, SpanQueryPtr q3, int32_t slop, Collection<int32_t> expectedDocs)
    {
        bool ordered = true;
        SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, q2, q3), slop, ordered);
        checkHits(snq, expectedDocs);
    }
    
    void orderedSlopTest3(int32_t slop, Collection<int32_t> expectedDocs)
    {
        orderedSlopTest3SQ(makeSpanTermQuery(L"w1"), makeSpanTermQuery(L"w2"), makeSpanTermQuery(L"w3"), slop, expectedDocs);
    }
    
    void orderedSlopTest3Equal(int32_t slop, Collection<int32_t> expectedDocs)
    {
        orderedSlopTest3SQ(makeSpanTermQuery(L"w1"), makeSpanTermQuery(L"w3"), makeSpanTermQuery(L"w3"), slop, expectedDocs);
    }
    
    void orderedSlopTest1Equal(int32_t slop, Collection<int32_t> expectedDocs)
    {
        orderedSlopTest3SQ(makeSpanTermQuery(L"u2"), makeSpanTermQuery(L"u2"), makeSpanTermQuery(L"u1"), slop, expectedDocs);
    }
    
    SpansPtr orSpans(Collection<String> terms)
    {
        Collection<SpanQueryPtr> sqa = Collection<SpanQueryPtr>::newInstance(terms.size());
        for (int32_t i = 0; i < terms.size(); ++i)
            sqa[i] = makeSpanTermQuery(terms[i]);
        return newLucene<SpanOrQuery>(sqa)->getSpans(searcher->getIndexReader());
    }
    
    void checkNextSpans(SpansPtr spans, int32_t doc, int32_t start, int32_t end)
    {
        BOOST_CHECK(spans->next());
        BOOST_CHECK_EQUAL(doc, spans->doc());
        BOOST_CHECK_EQUAL(start, spans->start());
        BOOST_CHECK_EQUAL(end, spans->end());
    }
    
    void addDoc(IndexWriterPtr writer, const String& id, const String& text)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", id, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"text", text, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    
    int32_t hitCount(SearcherPtr searcher, const String& word)
    {
        return searcher->search(newLucene<TermQuery>(newLucene<Term>(L"text", word)), 10)->totalHits;
    }
    
    SpanQueryPtr createSpan(const String& value)
    {
        return newLucene<SpanTermQuery>(newLucene<Term>(L"text", value));
    }
    
    SpanQueryPtr createSpan(int32_t slop, bool ordered, Collection<SpanQueryPtr> clauses)
    {
        return newLucene<SpanNearQuery>(clauses, slop, ordered);
    }
    
    SpanQueryPtr createSpan(int32_t slop, bool ordered, const String& term1, const String& term2)
    {
        return createSpan(slop, ordered, newCollection<SpanQueryPtr>(createSpan(term1), createSpan(term2)));
    }
};

const String SpansFixture::field = L"field";

BOOST_FIXTURE_TEST_SUITE(SpansTest, SpansFixture)

BOOST_AUTO_TEST_CASE(testSpanNearOrdered01)
{
    orderedSlopTest3(0, newCollection<int32_t>(0));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrdered02)
{
    orderedSlopTest3(1, newCollection<int32_t>(0, 1));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrdered03)
{
    orderedSlopTest3(2, newCollection<int32_t>(0, 1, 2));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrdered04)
{
    orderedSlopTest3(3, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrdered05)
{
    orderedSlopTest3(4, newCollection<int32_t>(0, 1, 2, 3));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual01)
{
    orderedSlopTest3Equal(0, Collection<int32_t>::newInstance());
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual02)
{
    orderedSlopTest3Equal(1, newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual03)
{
    orderedSlopTest3Equal(2, newCollection<int32_t>(1));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual04)
{
    orderedSlopTest3Equal(3, newCollection<int32_t>(1, 3));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual11)
{
    orderedSlopTest1Equal(0, newCollection<int32_t>(4));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual12)
{
    orderedSlopTest1Equal(0, newCollection<int32_t>(4));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual13)
{
    orderedSlopTest1Equal(1, newCollection<int32_t>(4, 5, 6));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual14)
{
    orderedSlopTest1Equal(2, newCollection<int32_t>(4, 5, 6, 7));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedEqual15)
{
    orderedSlopTest1Equal(3, newCollection<int32_t>(4, 5, 6, 7));
}

BOOST_AUTO_TEST_CASE(testSpanNearOrderedOverlap)
{
    bool ordered = true;
    int32_t slop = 1;
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"t1"), makeSpanTermQuery(L"t2"), makeSpanTermQuery(L"t3")), slop, ordered);
    SpansPtr spans = snq->getSpans(searcher->getIndexReader());

    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(11, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());

    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(11, spans->doc());
    BOOST_CHECK_EQUAL(2, spans->start());
    BOOST_CHECK_EQUAL(6, spans->end());

    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanNearUnOrdered)
{
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"u1"), makeSpanTermQuery(L"u2")), 0, false);
    SpansPtr spans = snq->getSpans(searcher->getIndexReader());

    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(4, spans->doc());
    BOOST_CHECK_EQUAL(1, spans->start());
    BOOST_CHECK_EQUAL(3, spans->end());

    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(5, spans->doc());
    BOOST_CHECK_EQUAL(2, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(8, spans->doc());
    BOOST_CHECK_EQUAL(2, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(9, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(2, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(10, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(2, spans->end());
    BOOST_CHECK(!spans->next());

    SpanNearQueryPtr u1u2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"u1"), makeSpanTermQuery(L"u2")), 0, false);
    snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(u1u2, makeSpanTermQuery(L"u2")), 1, false);
    spans = snq->getSpans(searcher->getIndexReader());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(4, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(3, spans->end());

    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(4, spans->doc());
    BOOST_CHECK_EQUAL(1, spans->start());
    BOOST_CHECK_EQUAL(3, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(5, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(5, spans->doc());
    BOOST_CHECK_EQUAL(2, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(8, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(8, spans->doc());
    BOOST_CHECK_EQUAL(2, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(9, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(2, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(9, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(4, spans->end());
    
    BOOST_CHECK(spans->next());
    BOOST_CHECK_EQUAL(10, spans->doc());
    BOOST_CHECK_EQUAL(0, spans->start());
    BOOST_CHECK_EQUAL(2, spans->end());
    
    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanOrEmpty)
{
    SpansPtr spans = orSpans(Collection<String>::newInstance());
    BOOST_CHECK(!spans->next());

    SpanOrQueryPtr a = newLucene<SpanOrQuery>(Collection<SpanQueryPtr>::newInstance());
    SpanOrQueryPtr b = newLucene<SpanOrQuery>(Collection<SpanQueryPtr>::newInstance());
    BOOST_CHECK(a->equals(b));
}

BOOST_AUTO_TEST_CASE(testSpanOrSingle)
{
    SpansPtr spans = orSpans(newCollection<String>(L"w5"));
    checkNextSpans(spans, 0, 4, 5);
    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanOrMovesForward)
{
    SpansPtr spans = orSpans(newCollection<String>(L"w1", L"xx"));

    BOOST_CHECK(spans->next());
    int32_t doc = spans->doc();
    BOOST_CHECK_EQUAL(0, doc);

    BOOST_CHECK(spans->skipTo(0));
    doc = spans->doc();

    BOOST_CHECK_EQUAL(1, doc);
}

BOOST_AUTO_TEST_CASE(testSpanOrDouble)
{
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"yy"));
    checkNextSpans(spans, 0, 4, 5);
    checkNextSpans(spans, 2, 3, 4);
    checkNextSpans(spans, 3, 4, 5);
    checkNextSpans(spans, 7, 3, 4);
    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanOrDoubleSkip)
{
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"yy"));
    BOOST_CHECK(spans->skipTo(3));
    BOOST_CHECK_EQUAL(3, spans->doc());
    BOOST_CHECK_EQUAL(4, spans->start());
    BOOST_CHECK_EQUAL(5, spans->end());
    checkNextSpans(spans, 7, 3, 4);
    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanOrUnused)
{
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"unusedTerm", L"yy"));
    checkNextSpans(spans, 0, 4, 5);
    checkNextSpans(spans, 2, 3, 4);
    checkNextSpans(spans, 3, 4, 5);
    checkNextSpans(spans, 7, 3, 4);
    BOOST_CHECK(!spans->next());
}

BOOST_AUTO_TEST_CASE(testSpanOrTripleSameDoc)
{
    SpansPtr spans = orSpans(newCollection<String>(L"t1", L"t2", L"t3"));
    checkNextSpans(spans, 11, 0, 1);
    checkNextSpans(spans, 11, 1, 2);
    checkNextSpans(spans, 11, 2, 3);
    checkNextSpans(spans, 11, 3, 4);
    checkNextSpans(spans, 11, 4, 5);
    checkNextSpans(spans, 11, 5, 6);
    BOOST_CHECK(!spans->next());
}

namespace TestSpanScorerZeroSloppyFreq
{
    class SloppyFreqSimilarity : public DefaultSimilarity
    {
    public:
        virtual ~SloppyFreqSimilarity()
        {
        }
    
    public:
        virtual double sloppyFreq(int32_t distance)
        {
            return 0.0;
        }
    };
    
    class SloppyFreqSpanNearQuery : public SpanNearQuery
    {
    public:
        SloppyFreqSpanNearQuery(SimilarityPtr sim, Collection<SpanQueryPtr> clauses, int32_t slop, bool inOrder) : SpanNearQuery(clauses, slop, inOrder)
        {
            this->sim = sim;
        }
        
        virtual ~SloppyFreqSpanNearQuery()
        {
        }
    
    protected:
        SimilarityPtr sim;
        
    public:
        virtual SimilarityPtr getSimilarity(SearcherPtr searcher)
        {
            return sim;
        }
    };
}

BOOST_AUTO_TEST_CASE(testSpanScorerZeroSloppyFreq)
{
    bool ordered = true;
    int32_t slop = 1;
    
    SimilarityPtr sim = newLucene<TestSpanScorerZeroSloppyFreq::SloppyFreqSimilarity>();
    SpanNearQueryPtr snq = newLucene<TestSpanScorerZeroSloppyFreq::SloppyFreqSpanNearQuery>(sim, newCollection<SpanQueryPtr>(makeSpanTermQuery(L"t1"), makeSpanTermQuery(L"t2")), slop, ordered);
    ScorerPtr spanScorer = snq->weight(searcher)->scorer(searcher->getIndexReader(), true, false);

    BOOST_CHECK_NE(spanScorer->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
    BOOST_CHECK_EQUAL(spanScorer->docID(), 11);
    double score = spanScorer->score();
    BOOST_CHECK_EQUAL(score, 0.0);
    BOOST_CHECK_EQUAL(spanScorer->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
}

BOOST_AUTO_TEST_CASE(testNPESpanQuery)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT, HashSet<String>::newInstance()), IndexWriter::MaxFieldLengthLIMITED);

    // Add documents
    addDoc(writer, L"1", L"the big dogs went running to the market");
    addDoc(writer, L"2", L"the cat chased the mouse, then the cat ate the mouse quickly");

    // Commit
    writer->close();

    // Get searcher
    IndexReaderPtr reader = IndexReader::open(dir, true);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);

    // Control (make sure docs indexed)
    BOOST_CHECK_EQUAL(2, hitCount(searcher, L"the"));
    BOOST_CHECK_EQUAL(1, hitCount(searcher, L"cat"));
    BOOST_CHECK_EQUAL(1, hitCount(searcher, L"dogs"));
    BOOST_CHECK_EQUAL(0, hitCount(searcher, L"rabbit"));

    // This throws exception (it shouldn't)
    BOOST_CHECK_EQUAL(1, searcher->search(createSpan(0, true, newCollection<SpanQueryPtr>(createSpan(4, false, L"chased", L"cat"), createSpan(L"ate"))), 10)->totalHits);
    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
