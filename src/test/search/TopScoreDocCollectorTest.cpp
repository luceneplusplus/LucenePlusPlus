/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "BooleanQuery.h"
#include "MatchAllDocsQuery.h"
#include "IndexSearcher.h"
#include "TopDocsCollector.h"
#include "TopScoreDocCollector.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture TopScoreDocCollectorTest;

TEST_F(TopScoreDocCollectorTest, testOutOfOrderCollection) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, AnalyzerPtr(), IndexWriter::MaxFieldLengthUNLIMITED);
    for (int32_t i = 0; i < 10; ++i) {
        writer->addDocument(newLucene<Document>());
    }
    writer->commit();
    writer->close();

    Collection<uint8_t> inOrder = newCollection<uint8_t>(false, true);
    Collection<String> actualTSDCClass = newCollection<String>(L"OutOfOrderTopScoreDocCollector", L"InOrderTopScoreDocCollector");

    BooleanQueryPtr bq = newLucene<BooleanQuery>();

    // Add a Query with SHOULD, since bw.scorer() returns BooleanScorer2
    // which delegates to BS if there are no mandatory clauses.
    bq->add(newLucene<MatchAllDocsQuery>(), BooleanClause::SHOULD);

    // Set minNrShouldMatch to 1 so that BQ will not optimize rewrite to return the clause instead of BQ.
    bq->setMinimumNumberShouldMatch(1);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    for (int32_t i = 0; i < inOrder.size(); ++i) {
        TopDocsCollectorPtr tdc = TopScoreDocCollector::create(3, inOrder[i] == 1);
        EXPECT_EQ(actualTSDCClass[i], tdc->getClassName());

        searcher->search(newLucene<MatchAllDocsQuery>(), tdc);

        Collection<ScoreDocPtr> sd = tdc->topDocs()->scoreDocs;
        EXPECT_EQ(3, sd.size());
        for (int32_t j = 0; j < sd.size(); ++j) {
            EXPECT_EQ(j, sd[j]->doc);
        }
    }
}
