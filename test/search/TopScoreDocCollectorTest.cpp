/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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

BOOST_FIXTURE_TEST_SUITE(TopScoreDocCollectorTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testOutOfOrderCollection)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, AnalyzerPtr(), IndexWriter::MaxFieldLengthUNLIMITED);
    for (int32_t i = 0; i < 10; ++i)
        writer->addDocument(newLucene<Document>());
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
    for (int32_t i = 0; i < inOrder.size(); ++i)
    {
        TopDocsCollectorPtr tdc = TopScoreDocCollector::create(3, inOrder[i] == 1);
        BOOST_CHECK_EQUAL(actualTSDCClass[i], tdc->getClassName());

        searcher->search(newLucene<MatchAllDocsQuery>(), tdc);

        Collection<ScoreDocPtr> sd = tdc->topDocs()->scoreDocs;
        BOOST_CHECK_EQUAL(3, sd.size());
        for (int32_t j = 0; j < sd.size(); ++j)
            BOOST_CHECK_EQUAL(j, sd[j]->doc);
    }
}

BOOST_AUTO_TEST_SUITE_END()
