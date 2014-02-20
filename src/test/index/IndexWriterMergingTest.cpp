/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterMergingTest;

static bool verifyIndex(const DirectoryPtr& directory, int32_t startAt) {
    bool fail = false;
    IndexReaderPtr reader = IndexReader::open(directory, true);

    int32_t max = reader->maxDoc();
    for (int32_t i = 0; i < max; ++i) {
        DocumentPtr temp = reader->document(i);
        if (temp->getField(L"count")->stringValue() != StringUtils::toString(i + startAt)) {
            fail = true;
        }
    }
    reader->close();
    return fail;
}

static void fillIndex(const DirectoryPtr& dir, int32_t start, int32_t numDocs) {
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMergeFactor(2);
    writer->setMaxBufferedDocs(2);

    for (int32_t i = start; i < (start + numDocs); ++i) {
        DocumentPtr temp = newLucene<Document>();
        temp->add(newLucene<Field>(L"count", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(temp);
    }
    writer->close();
}

/// Tests that index merging (specifically addIndexesNoOptimize()) doesn't change the index order of documents.
TEST_F(IndexWriterMergingTest, testIndexWriterMerging) {
    int32_t num = 100;

    DirectoryPtr indexA = newLucene<MockRAMDirectory>();
    DirectoryPtr indexB = newLucene<MockRAMDirectory>();

    fillIndex(indexA, 0, num);
    EXPECT_TRUE(!verifyIndex(indexA, 0));

    fillIndex(indexB, num, num);
    EXPECT_TRUE(!verifyIndex(indexB, num));

    DirectoryPtr merged = newLucene<MockRAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(merged, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMergeFactor(2);

    writer->addIndexesNoOptimize(newCollection<DirectoryPtr>(indexA, indexB));
    writer->optimize();
    writer->close();

    EXPECT_TRUE(!verifyIndex(merged, 0));
    merged->close();
}
