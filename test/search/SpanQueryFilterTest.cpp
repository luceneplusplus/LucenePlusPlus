/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "SpanTermQuery.h"
#include "Term.h"
#include "SpanQueryFilter.h"
#include "SpanFilterResult.h"
#include "DocIdSet.h"
#include "DocIdSetIterator.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(SpanQueryFilterTest, LuceneTestFixture)

static int32_t getDocIdSetSize(DocIdSetPtr docIdSet)
{
    int32_t size = 0;
    DocIdSetIteratorPtr it = docIdSet->iterator();
    while (it->nextDoc() != DocIdSetIterator::NO_MORE_DOCS)
        ++size;
    return size;
}

static void checkContainsDocId(DocIdSetPtr docIdSet, int32_t docId)
{
    DocIdSetIteratorPtr it = docIdSet->iterator();
    BOOST_CHECK_NE(it->advance(docId), DocIdSetIterator::NO_MORE_DOCS);
    BOOST_CHECK_EQUAL(it->docID(), docId);
}

BOOST_AUTO_TEST_CASE(testFilterWorks)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 500; ++i)
    {
        DocumentPtr document = newLucene<Document>();
        document->add(newLucene<Field>(L"field", intToEnglish(i) + L" equals " + intToEnglish(i), Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(document);
    }
    
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);

    SpanTermQueryPtr query = newLucene<SpanTermQuery>(newLucene<Term>(L"field", intToEnglish(10)));
    SpanQueryFilterPtr filter = newLucene<SpanQueryFilter>(query);
    SpanFilterResultPtr result = filter->bitSpans(reader);
    DocIdSetPtr docIdSet = result->getDocIdSet();
    BOOST_CHECK(docIdSet);
    checkContainsDocId(docIdSet, 10);
    Collection<PositionInfoPtr> spans = result->getPositions();
    BOOST_CHECK(spans);
    int32_t size = getDocIdSetSize(docIdSet);
    BOOST_CHECK_EQUAL(spans.size(), size);
    for (Collection<PositionInfoPtr>::iterator info = spans.begin(); info != spans.end(); ++info)
    {
        BOOST_CHECK(*info);
        // The doc should indicate the bit is on
        checkContainsDocId(docIdSet, (*info)->getDoc());
        // There should be two positions in each
        BOOST_CHECK_EQUAL((*info)->getPositions().size(), 2);
    }
    reader->close();
}

BOOST_AUTO_TEST_SUITE_END()
