/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TermQuery.h"
#include "Term.h"
#include "Weight.h"
#include "TermScorer.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "Collector.h"
#include "DocIdSetIterator.h"

using namespace Lucene;

DECLARE_SHARED_PTR(TestHit)

class TestHit : public LuceneObject {
public:
    TestHit(int32_t doc, double score) {
        this->doc = doc;
        this->score = score;
    }

    virtual ~TestHit() {
    }

public:
    int32_t doc;
    double score;

public:
    virtual String toString() {
        return L"TestHit{doc=" + StringUtils::toString(doc) + L", score=" + StringUtils::toString(score) + L"}";
    }
};

class TermScorerTest : public LuceneTestFixture {
public:
    TermScorerTest() {
        values = newCollection<String>(L"all", L"dogs dogs", L"like", L"playing", L"fetch", L"all");
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < values.size(); ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, values[i], Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        indexSearcher = newLucene<IndexSearcher>(directory, false);
        indexReader = indexSearcher->getIndexReader();
    }

    virtual ~TermScorerTest() {
    }

protected:
    static const String FIELD;

    RAMDirectoryPtr directory;
    Collection<String> values;
    IndexSearcherPtr indexSearcher;
    IndexReaderPtr indexReader;
};

const String TermScorerTest::FIELD = L"field";

namespace TestTermScorer {

class TestCollector : public Collector {
public:
    TestCollector(Collection<TestHitPtr> docs) {
        this->docs = docs;
        this->base = 0;
    }

    virtual ~TestCollector() {
    }

protected:
    int32_t base;
    ScorerPtr scorer;
    Collection<TestHitPtr> docs;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        double score = scorer->score();
        doc = doc + base;
        docs.add(newLucene<TestHit>(doc, score));
        EXPECT_TRUE(score > 0);
        EXPECT_TRUE(doc == 0 || doc == 5);
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

TEST_F(TermScorerTest, testTermScorer) {
    TermPtr allTerm = newLucene<Term>(FIELD, L"all");
    TermQueryPtr termQuery = newLucene<TermQuery>(allTerm);

    WeightPtr weight = termQuery->weight(indexSearcher);

    TermScorerPtr ts = newLucene<TermScorer>(weight, indexReader->termDocs(allTerm), indexSearcher->getSimilarity(), indexReader->norms(FIELD));

    // we have 2 documents with the term all in them, one document for all the other values
    Collection<TestHitPtr> docs = Collection<TestHitPtr>::newInstance();

    ts->score(newLucene<TestTermScorer::TestCollector>(docs));

    EXPECT_EQ(docs.size(), 2);
    EXPECT_EQ(docs[0]->score, docs[1]->score);
    EXPECT_NEAR(docs[0]->score, 1.6931472, 0.000001);
}

TEST_F(TermScorerTest, testNext) {
    TermPtr allTerm = newLucene<Term>(FIELD, L"all");
    TermQueryPtr termQuery = newLucene<TermQuery>(allTerm);

    WeightPtr weight = termQuery->weight(indexSearcher);

    TermScorerPtr ts = newLucene<TermScorer>(weight, indexReader->termDocs(allTerm), indexSearcher->getSimilarity(), indexReader->norms(FIELD));
    EXPECT_NE(ts->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
    EXPECT_NEAR(ts->score(), 1.6931472, 0.000001);
    EXPECT_NE(ts->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
    EXPECT_NEAR(ts->score(), 1.6931472, 0.000001);
    EXPECT_EQ(ts->nextDoc(), DocIdSetIterator::NO_MORE_DOCS);
}

TEST_F(TermScorerTest, testSkipTo) {
    TermPtr allTerm = newLucene<Term>(FIELD, L"all");
    TermQueryPtr termQuery = newLucene<TermQuery>(allTerm);

    WeightPtr weight = termQuery->weight(indexSearcher);

    TermScorerPtr ts = newLucene<TermScorer>(weight, indexReader->termDocs(allTerm), indexSearcher->getSimilarity(), indexReader->norms(FIELD));
    EXPECT_NE(ts->advance(3), DocIdSetIterator::NO_MORE_DOCS);
    EXPECT_EQ(ts->docID(), 5);
}
