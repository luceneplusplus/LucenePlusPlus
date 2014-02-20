/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

class SpansTest : public LuceneTestFixture {
public:
    SpansTest() {
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
        for (int32_t i = 0; i < docFields.size(); ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(field, docFields[i], Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }

    virtual ~SpansTest() {
        searcher->close();
    }

protected:
    IndexSearcherPtr searcher;
    Collection<String> docFields;

public:
    static const String field;

public:
    SpanTermQueryPtr makeSpanTermQuery(const String& text) {
        return newLucene<SpanTermQuery>(newLucene<Term>(field, text));
    }

    void checkHits(const QueryPtr& query, Collection<int32_t> results) {
        CheckHits::checkHits(query, field, searcher, results);
    }

    void orderedSlopTest3SQ(const SpanQueryPtr& q1, const SpanQueryPtr& q2, const SpanQueryPtr& q3, int32_t slop, Collection<int32_t> expectedDocs) {
        bool ordered = true;
        SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, q2, q3), slop, ordered);
        checkHits(snq, expectedDocs);
    }

    void orderedSlopTest3(int32_t slop, Collection<int32_t> expectedDocs) {
        orderedSlopTest3SQ(makeSpanTermQuery(L"w1"), makeSpanTermQuery(L"w2"), makeSpanTermQuery(L"w3"), slop, expectedDocs);
    }

    void orderedSlopTest3Equal(int32_t slop, Collection<int32_t> expectedDocs) {
        orderedSlopTest3SQ(makeSpanTermQuery(L"w1"), makeSpanTermQuery(L"w3"), makeSpanTermQuery(L"w3"), slop, expectedDocs);
    }

    void orderedSlopTest1Equal(int32_t slop, Collection<int32_t> expectedDocs) {
        orderedSlopTest3SQ(makeSpanTermQuery(L"u2"), makeSpanTermQuery(L"u2"), makeSpanTermQuery(L"u1"), slop, expectedDocs);
    }

    SpansPtr orSpans(Collection<String> terms) {
        Collection<SpanQueryPtr> sqa = Collection<SpanQueryPtr>::newInstance(terms.size());
        for (int32_t i = 0; i < terms.size(); ++i) {
            sqa[i] = makeSpanTermQuery(terms[i]);
        }
        return newLucene<SpanOrQuery>(sqa)->getSpans(searcher->getIndexReader());
    }

    void checkNextSpans(const SpansPtr& spans, int32_t doc, int32_t start, int32_t end) {
        EXPECT_TRUE(spans->next());
        EXPECT_EQ(doc, spans->doc());
        EXPECT_EQ(start, spans->start());
        EXPECT_EQ(end, spans->end());
    }

    void addDoc(const IndexWriterPtr& writer, const String& id, const String& text) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", id, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"text", text, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }

    int32_t hitCount(const SearcherPtr& searcher, const String& word) {
        return searcher->search(newLucene<TermQuery>(newLucene<Term>(L"text", word)), 10)->totalHits;
    }

    SpanQueryPtr createSpan(const String& value) {
        return newLucene<SpanTermQuery>(newLucene<Term>(L"text", value));
    }

    SpanQueryPtr createSpan(int32_t slop, bool ordered, Collection<SpanQueryPtr> clauses) {
        return newLucene<SpanNearQuery>(clauses, slop, ordered);
    }

    SpanQueryPtr createSpan(int32_t slop, bool ordered, const String& term1, const String& term2) {
        return createSpan(slop, ordered, newCollection<SpanQueryPtr>(createSpan(term1), createSpan(term2)));
    }
};

const String SpansTest::field = L"field";

TEST_F(SpansTest, testSpanNearOrdered01) {
    orderedSlopTest3(0, newCollection<int32_t>(0));
}

TEST_F(SpansTest, testSpanNearOrdered02) {
    orderedSlopTest3(1, newCollection<int32_t>(0, 1));
}

TEST_F(SpansTest, testSpanNearOrdered03) {
    orderedSlopTest3(2, newCollection<int32_t>(0, 1, 2));
}

TEST_F(SpansTest, testSpanNearOrdered04) {
    orderedSlopTest3(3, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpansTest, testSpanNearOrdered05) {
    orderedSlopTest3(4, newCollection<int32_t>(0, 1, 2, 3));
}

TEST_F(SpansTest, testSpanNearOrderedEqual01) {
    orderedSlopTest3Equal(0, Collection<int32_t>::newInstance());
}

TEST_F(SpansTest, testSpanNearOrderedEqual02) {
    orderedSlopTest3Equal(1, newCollection<int32_t>(1));
}

TEST_F(SpansTest, testSpanNearOrderedEqual03) {
    orderedSlopTest3Equal(2, newCollection<int32_t>(1));
}

TEST_F(SpansTest, testSpanNearOrderedEqual04) {
    orderedSlopTest3Equal(3, newCollection<int32_t>(1, 3));
}

TEST_F(SpansTest, testSpanNearOrderedEqual11) {
    orderedSlopTest1Equal(0, newCollection<int32_t>(4));
}

TEST_F(SpansTest, testSpanNearOrderedEqual12) {
    orderedSlopTest1Equal(0, newCollection<int32_t>(4));
}

TEST_F(SpansTest, testSpanNearOrderedEqual13) {
    orderedSlopTest1Equal(1, newCollection<int32_t>(4, 5, 6));
}

TEST_F(SpansTest, testSpanNearOrderedEqual14) {
    orderedSlopTest1Equal(2, newCollection<int32_t>(4, 5, 6, 7));
}

TEST_F(SpansTest, testSpanNearOrderedEqual15) {
    orderedSlopTest1Equal(3, newCollection<int32_t>(4, 5, 6, 7));
}

TEST_F(SpansTest, testSpanNearOrderedOverlap) {
    bool ordered = true;
    int32_t slop = 1;
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"t1"), makeSpanTermQuery(L"t2"), makeSpanTermQuery(L"t3")), slop, ordered);
    SpansPtr spans = snq->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(11, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(11, spans->doc());
    EXPECT_EQ(2, spans->start());
    EXPECT_EQ(6, spans->end());

    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanNearUnOrdered) {
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"u1"), makeSpanTermQuery(L"u2")), 0, false);
    SpansPtr spans = snq->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(4, spans->doc());
    EXPECT_EQ(1, spans->start());
    EXPECT_EQ(3, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(5, spans->doc());
    EXPECT_EQ(2, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(8, spans->doc());
    EXPECT_EQ(2, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(9, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(2, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(10, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(2, spans->end());
    EXPECT_TRUE(!spans->next());

    SpanNearQueryPtr u1u2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(makeSpanTermQuery(L"u1"), makeSpanTermQuery(L"u2")), 0, false);
    snq = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(u1u2, makeSpanTermQuery(L"u2")), 1, false);
    spans = snq->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(4, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(3, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(4, spans->doc());
    EXPECT_EQ(1, spans->start());
    EXPECT_EQ(3, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(5, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(5, spans->doc());
    EXPECT_EQ(2, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(8, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(8, spans->doc());
    EXPECT_EQ(2, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(9, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(2, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(9, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(4, spans->end());

    EXPECT_TRUE(spans->next());
    EXPECT_EQ(10, spans->doc());
    EXPECT_EQ(0, spans->start());
    EXPECT_EQ(2, spans->end());

    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanOrEmpty) {
    SpansPtr spans = orSpans(Collection<String>::newInstance());
    EXPECT_TRUE(!spans->next());

    SpanOrQueryPtr a = newLucene<SpanOrQuery>(Collection<SpanQueryPtr>::newInstance());
    SpanOrQueryPtr b = newLucene<SpanOrQuery>(Collection<SpanQueryPtr>::newInstance());
    EXPECT_TRUE(a->equals(b));
}

TEST_F(SpansTest, testSpanOrSingle) {
    SpansPtr spans = orSpans(newCollection<String>(L"w5"));
    checkNextSpans(spans, 0, 4, 5);
    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanOrMovesForward) {
    SpansPtr spans = orSpans(newCollection<String>(L"w1", L"xx"));

    EXPECT_TRUE(spans->next());
    int32_t doc = spans->doc();
    EXPECT_EQ(0, doc);

    EXPECT_TRUE(spans->skipTo(0));
    doc = spans->doc();

    EXPECT_EQ(1, doc);
}

TEST_F(SpansTest, testSpanOrDouble) {
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"yy"));
    checkNextSpans(spans, 0, 4, 5);
    checkNextSpans(spans, 2, 3, 4);
    checkNextSpans(spans, 3, 4, 5);
    checkNextSpans(spans, 7, 3, 4);
    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanOrDoubleSkip) {
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"yy"));
    EXPECT_TRUE(spans->skipTo(3));
    EXPECT_EQ(3, spans->doc());
    EXPECT_EQ(4, spans->start());
    EXPECT_EQ(5, spans->end());
    checkNextSpans(spans, 7, 3, 4);
    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanOrUnused) {
    SpansPtr spans = orSpans(newCollection<String>(L"w5", L"unusedTerm", L"yy"));
    checkNextSpans(spans, 0, 4, 5);
    checkNextSpans(spans, 2, 3, 4);
    checkNextSpans(spans, 3, 4, 5);
    checkNextSpans(spans, 7, 3, 4);
    EXPECT_TRUE(!spans->next());
}

TEST_F(SpansTest, testSpanOrTripleSameDoc) {
    SpansPtr spans = orSpans(newCollection<String>(L"t1", L"t2", L"t3"));
    checkNextSpans(spans, 11, 0, 1);
    checkNextSpans(spans, 11, 1, 2);
    checkNextSpans(spans, 11, 2, 3);
    checkNextSpans(spans, 11, 3, 4);
    checkNextSpans(spans, 11, 4, 5);
    checkNextSpans(spans, 11, 5, 6);
    EXPECT_TRUE(!spans->next());
}

namespace TestSpanScorerZeroSloppyFreq {

class SloppyFreqSimilarity : public DefaultSimilarity {
public:
    virtual ~SloppyFreqSimilarity() {
    }

public:
    virtual double sloppyFreq(int32_t distance) {
        return 0.0;
    }
};

class SloppyFreqSpanNearQuery : public SpanNearQuery {
public:
    SloppyFreqSpanNearQuery(const SimilarityPtr& sim, Collection<SpanQueryPtr> clauses, int32_t slop, bool inOrder) : SpanNearQuery(clauses, slop, inOrder) {
        this->sim = sim;
    }

    virtual ~SloppyFreqSpanNearQuery() {
    }

protected:
    SimilarityPtr sim;

public:
    virtual SimilarityPtr getSimilarity(const SearcherPtr& searcher) {
        return sim;
    }
};

}

TEST_F(SpansTest, testSpanScorerZeroSloppyFreq) {
    bool ordered = true;
    int32_t slop = 1;

    SimilarityPtr sim = newLucene<TestSpanScorerZeroSloppyFreq::SloppyFreqSimilarity>();
    SpanNearQueryPtr snq = newLucene<TestSpanScorerZeroSloppyFreq::SloppyFreqSpanNearQuery>(sim, newCollection<SpanQueryPtr>(makeSpanTermQuery(L"t1"), makeSpanTermQuery(L"t2")), slop, ordered);
    ScorerPtr spanScorer = snq->weight(searcher)->scorer(searcher->getIndexReader(), true, false);

    EXPECT_NE(spanScorer->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
    EXPECT_EQ(spanScorer->docID(), 11);
    double score = spanScorer->score();
    EXPECT_EQ(score, 0.0);
    EXPECT_EQ(spanScorer->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
}

TEST_F(SpansTest, testNPESpanQuery) {
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
    EXPECT_EQ(2, hitCount(searcher, L"the"));
    EXPECT_EQ(1, hitCount(searcher, L"cat"));
    EXPECT_EQ(1, hitCount(searcher, L"dogs"));
    EXPECT_EQ(0, hitCount(searcher, L"rabbit"));

    // This throws exception (it shouldn't)
    EXPECT_EQ(1, searcher->search(createSpan(0, true, newCollection<SpanQueryPtr>(createSpan(4, false, L"chased", L"cat"), createSpan(L"ate"))), 10)->totalHits);
    reader->close();
    dir->close();
}
