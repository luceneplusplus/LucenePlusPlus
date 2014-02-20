/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
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

typedef LuceneTestFixture SpanQueryFilterTest;

static int32_t getDocIdSetSize(const DocIdSetPtr& docIdSet) {
    int32_t size = 0;
    DocIdSetIteratorPtr it = docIdSet->iterator();
    while (it->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        ++size;
    }
    return size;
}

static void checkContainsDocId(const DocIdSetPtr& docIdSet, int32_t docId) {
    DocIdSetIteratorPtr it = docIdSet->iterator();
    EXPECT_NE(it->advance(docId), DocIdSetIterator::NO_MORE_DOCS);
    EXPECT_EQ(it->docID(), docId);
}

TEST_F(SpanQueryFilterTest, testFilterWorks) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 500; ++i) {
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
    EXPECT_TRUE(docIdSet);
    checkContainsDocId(docIdSet, 10);
    Collection<PositionInfoPtr> spans = result->getPositions();
    EXPECT_TRUE(spans);
    int32_t size = getDocIdSetSize(docIdSet);
    EXPECT_EQ(spans.size(), size);
    for (Collection<PositionInfoPtr>::iterator info = spans.begin(); info != spans.end(); ++info) {
        EXPECT_TRUE(*info);
        // The doc should indicate the bit is on
        checkContainsDocId(docIdSet, (*info)->getDoc());
        // There should be two positions in each
        EXPECT_EQ((*info)->getPositions().size(), 2);
    }
    reader->close();
}
