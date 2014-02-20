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
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "Similarity.h"
#include "BooleanScorer.h"

using namespace Lucene;

typedef LuceneTestFixture BooleanScorerTest;

TEST_F(BooleanScorerTest, testMethod) {
    static const String FIELD = L"category";

    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    Collection<String> values = newCollection<String>(L"1", L"2", L"3", L"4");

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < values.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD, values[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    BooleanQueryPtr booleanQuery1 = newLucene<BooleanQuery>();
    booleanQuery1->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"1")), BooleanClause::SHOULD);
    booleanQuery1->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"2")), BooleanClause::SHOULD);

    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(booleanQuery1, BooleanClause::MUST);
    query->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"9")), BooleanClause::MUST_NOT);

    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    Collection<ScoreDocPtr> hits = indexSearcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
}

namespace TestEmptyBucketWithMoreDocs {

class EmptyScorer : public Scorer {
public:
    EmptyScorer(const SimilarityPtr& similarity) : Scorer(similarity) {
        doc = -1;
    }

    virtual ~EmptyScorer() {
    }

protected:
    int32_t doc;

public:
    virtual double score() {
        return 0.0;
    }

    virtual int32_t docID() {
        return doc;
    }

    virtual int32_t nextDoc() {
        doc = doc == -1 ? 3000 : NO_MORE_DOCS;
        return doc;
    }

    virtual int32_t advance(int32_t target) {
        doc = target <= 3000 ? 3000 : NO_MORE_DOCS;
        return doc;
    }
};

}

TEST_F(BooleanScorerTest, testEmptyBucketWithMoreDocs) {
    // This test checks the logic of nextDoc() when all sub scorers have docs beyond the first bucket
    // (for example). Currently, the code relies on the 'more' variable to work properly, and this
    // test ensures that if the logic changes, we have a test to back it up.
    SimilarityPtr sim = Similarity::getDefault();
    Collection<ScorerPtr> scorers = newCollection<ScorerPtr>(newLucene<TestEmptyBucketWithMoreDocs::EmptyScorer>(sim));

    BooleanScorerPtr bs = newLucene<BooleanScorer>(sim, 1, scorers, Collection<ScorerPtr>());

    EXPECT_EQ(3000, bs->nextDoc());
    EXPECT_EQ(DocIdSetIterator::NO_MORE_DOCS, bs->nextDoc());
}
