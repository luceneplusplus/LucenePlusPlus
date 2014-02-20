/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Collector.h"
#include "Scorer.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"

using namespace Lucene;

typedef LuceneTestFixture DocBoostTest;

namespace TestDocBoost {

class BoostCollector : public Collector {
public:
    BoostCollector(Collection<double> scores) {
        this->scores = scores;
        this->base = 0;
    }

    virtual ~BoostCollector() {
    }

public:
    Collection<double> scores;
    int32_t base;
    ScorerPtr scorer;

public:
    virtual void setScorer(const ScorerPtr& scorer) {
        this->scorer = scorer;
    }

    virtual void collect(int32_t doc) {
        scores[doc + base] = scorer->score();
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        base = docBase;
    }

    virtual bool acceptsDocsOutOfOrder() {
        return true;
    }
};

}

TEST_F(DocBoostTest, testDocBoost) {
    RAMDirectoryPtr store = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(store, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    FieldablePtr f1 = newLucene<Field>(L"field", L"word", Field::STORE_YES, Field::INDEX_ANALYZED);
    FieldablePtr f2 = newLucene<Field>(L"field", L"word", Field::STORE_YES, Field::INDEX_ANALYZED);
    f2->setBoost(2.0);

    DocumentPtr d1 = newLucene<Document>();
    DocumentPtr d2 = newLucene<Document>();
    DocumentPtr d3 = newLucene<Document>();
    DocumentPtr d4 = newLucene<Document>();
    d3->setBoost(3.0);
    d4->setBoost(2.0);

    d1->add(f1); // boost = 1
    d2->add(f2); // boost = 2
    d3->add(f1); // boost = 3
    d4->add(f2); // boost = 4

    writer->addDocument(d1);
    writer->addDocument(d2);
    writer->addDocument(d3);
    writer->addDocument(d4);
    writer->optimize();
    writer->close();

    Collection<double> scores = Collection<double>::newInstance(4);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(store, true);
    searcher->search(newLucene<TermQuery>(newLucene<Term>(L"field", L"word")), newLucene<TestDocBoost::BoostCollector>(scores));

    double lastScore = 0.0;
    for (int32_t i = 0; i < 4; ++i) {
        EXPECT_TRUE(scores[i] > lastScore);
        lastScore = scores[i];
    }
}
