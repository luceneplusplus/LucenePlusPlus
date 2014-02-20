/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Similarity.h"
#include "Explanation.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "Collector.h"
#include "BooleanQuery.h"
#include "PhraseQuery.h"
#include "Scorer.h"

using namespace Lucene;

typedef LuceneTestFixture SimilarityTest;

namespace TestSimilarity {

class SimpleIDFExplanation : public IDFExplanation {
public:
    virtual ~SimpleIDFExplanation() {
    }

public:
    virtual double getIdf() {
        return 1.0;
    }

    virtual String explain() {
        return L"Inexplicable";
    }
};

class SimpleSimilarity : public Similarity {
public:
    virtual ~SimpleSimilarity() {
    }

public:
    virtual double lengthNorm(const String& fieldName, int32_t numTokens) {
        return 1.0;
    }

    virtual double queryNorm(double sumOfSquaredWeights) {
        return 1.0;
    }

    virtual double tf(double freq) {
        return freq;
    }

    virtual double sloppyFreq(int32_t distance) {
        return 2.0;
    }

    virtual double idf(int32_t docFreq, int32_t numDocs) {
        return 1.0;
    }

    virtual double coord(int32_t overlap, int32_t maxOverlap) {
        return 1.0;
    }

    virtual IDFExplanationPtr idfExplain(Collection<TermPtr> terms, const SearcherPtr& searcher) {
        return newLucene<SimpleIDFExplanation>();
    }
};

class TermQueryCollector : public Collector {
public:
    virtual ~TermQueryCollector() {
    }

protected:
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(1.0, scorer->score());
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

class BooleanQueryCollector : public Collector {
public:
    BooleanQueryCollector() {
        this->base = 0;
    }

    virtual ~BooleanQueryCollector() {
    }

protected:
    int32_t base;
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ((double)doc + (double)base + 1.0, scorer->score());
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

class PhraseQueryCollector : public Collector {
public:
    PhraseQueryCollector(double expectedScore) {
        this->expectedScore = expectedScore;
    }

    virtual ~PhraseQueryCollector() {
    }

protected:
    double expectedScore;
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        EXPECT_EQ(expectedScore, scorer->score());
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

TEST_F(SimilarityTest, testSimilarity) {
    RAMDirectoryPtr store = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(store, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setSimilarity(newLucene<TestSimilarity::SimpleSimilarity>());

    DocumentPtr d1 = newLucene<Document>();
    d1->add(newLucene<Field>(L"field", L"a c", Field::STORE_YES, Field::INDEX_ANALYZED));

    DocumentPtr d2 = newLucene<Document>();
    d2->add(newLucene<Field>(L"field", L"a b c", Field::STORE_YES, Field::INDEX_ANALYZED));

    writer->addDocument(d1);
    writer->addDocument(d2);
    writer->optimize();
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(store, true);
    searcher->setSimilarity(newLucene<TestSimilarity::SimpleSimilarity>());

    TermPtr a = newLucene<Term>(L"field", L"a");
    TermPtr b = newLucene<Term>(L"field", L"b");
    TermPtr c = newLucene<Term>(L"field", L"c");

    searcher->search(newLucene<TermQuery>(b), newLucene<TestSimilarity::TermQueryCollector>());

    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    bq->add(newLucene<TermQuery>(a), BooleanClause::SHOULD);
    bq->add(newLucene<TermQuery>(b), BooleanClause::SHOULD);

    searcher->search(bq, newLucene<TestSimilarity::BooleanQueryCollector>());

    PhraseQueryPtr pq = newLucene<PhraseQuery>();
    pq->add(a);
    pq->add(c);

    searcher->search(pq, newLucene<TestSimilarity::PhraseQueryCollector>(1.0));

    pq->setSlop(2);

    searcher->search(pq, newLucene<TestSimilarity::PhraseQueryCollector>(2.0));
}
