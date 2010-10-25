/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(CheckIndexTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testDeletedDocs)
{
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);      
    writer->setMaxBufferedDocs(2);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    for (int32_t i = 0; i < 19; ++i)
        writer->addDocument(doc);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(5);
    reader->close();
    
    CheckIndexPtr checker = newLucene<CheckIndex>(dir);
    IndexStatusPtr indexStatus = checker->checkIndex();
    BOOST_CHECK(indexStatus->clean);
    
    SegmentInfoStatusPtr seg = indexStatus->segmentInfos[0];
    BOOST_CHECK(seg->openReaderPassed);
    
    BOOST_CHECK(seg->diagnostics);
    
    BOOST_CHECK(seg->fieldNormStatus);
    BOOST_CHECK(seg->fieldNormStatus->error.isNull());
    BOOST_CHECK_EQUAL(1, seg->fieldNormStatus->totFields);

    BOOST_CHECK(seg->termIndexStatus);
    BOOST_CHECK(seg->termIndexStatus->error.isNull());
    BOOST_CHECK_EQUAL(1, seg->termIndexStatus->termCount);
    BOOST_CHECK_EQUAL(19, seg->termIndexStatus->totFreq);
    BOOST_CHECK_EQUAL(18, seg->termIndexStatus->totPos);

    BOOST_CHECK(seg->storedFieldStatus);
    BOOST_CHECK(seg->storedFieldStatus->error.isNull());
    BOOST_CHECK_EQUAL(18, seg->storedFieldStatus->docCount);
    BOOST_CHECK_EQUAL(18, seg->storedFieldStatus->totFields);

    BOOST_CHECK(seg->termVectorStatus);
    BOOST_CHECK(seg->termVectorStatus->error.isNull());
    BOOST_CHECK_EQUAL(18, seg->termVectorStatus->docCount);
    BOOST_CHECK_EQUAL(18, seg->termVectorStatus->totVectors);
    
    BOOST_CHECK(!seg->diagnostics.empty());
    
    Collection<String> onlySegments = Collection<String>::newInstance();
    onlySegments.add(L"_0");
    
    BOOST_CHECK(checker->checkIndex(onlySegments)->clean);
}

BOOST_AUTO_TEST_SUITE_END()
