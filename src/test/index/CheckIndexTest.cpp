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
#include "Document.h"
#include "Field.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "CheckIndex.h"

using namespace Lucene;

typedef LuceneTestFixture CheckIndexTest;

TEST_F(CheckIndexTest, testDeletedDocs) {
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    for (int32_t i = 0; i < 19; ++i) {
        writer->addDocument(doc);
    }
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(5);
    reader->close();

    CheckIndexPtr checker = newLucene<CheckIndex>(dir);
    IndexStatusPtr indexStatus = checker->checkIndex();
    EXPECT_TRUE(indexStatus->clean);

    SegmentInfoStatusPtr seg = indexStatus->segmentInfos[0];
    EXPECT_TRUE(seg->openReaderPassed);

    EXPECT_TRUE(seg->diagnostics);

    EXPECT_TRUE(seg->fieldNormStatus);
    EXPECT_TRUE(seg->fieldNormStatus->error.isNull());
    EXPECT_EQ(1, seg->fieldNormStatus->totFields);

    EXPECT_TRUE(seg->termIndexStatus);
    EXPECT_TRUE(seg->termIndexStatus->error.isNull());
    EXPECT_EQ(1, seg->termIndexStatus->termCount);
    EXPECT_EQ(19, seg->termIndexStatus->totFreq);
    EXPECT_EQ(18, seg->termIndexStatus->totPos);

    EXPECT_TRUE(seg->storedFieldStatus);
    EXPECT_TRUE(seg->storedFieldStatus->error.isNull());
    EXPECT_EQ(18, seg->storedFieldStatus->docCount);
    EXPECT_EQ(18, seg->storedFieldStatus->totFields);

    EXPECT_TRUE(seg->termVectorStatus);
    EXPECT_TRUE(seg->termVectorStatus->error.isNull());
    EXPECT_EQ(18, seg->termVectorStatus->docCount);
    EXPECT_EQ(18, seg->termVectorStatus->totVectors);

    EXPECT_TRUE(!seg->diagnostics.empty());

    Collection<String> onlySegments = Collection<String>::newInstance();
    onlySegments.add(L"_0");

    EXPECT_TRUE(checker->checkIndex(onlySegments)->clean);
}
