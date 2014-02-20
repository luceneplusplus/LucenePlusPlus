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
#include "SimpleAnalyzer.h"
#include "ParallelReader.h"
#include "IndexReader.h"
#include "Document.h"
#include "Field.h"

using namespace Lucene;

typedef LuceneTestFixture ParallelReaderEmptyIndexTest;

/// Creates two empty indexes and wraps a ParallelReader around.
/// Adding this reader to a new index should not throw any exception.
TEST_F(ParallelReaderEmptyIndexTest, testEmptyIndex) {
    RAMDirectoryPtr rd1 = newLucene<MockRAMDirectory>();
    IndexWriterPtr iw = newLucene<IndexWriter>(rd1, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    iw->close();

    RAMDirectoryPtr rd2 = newLucene<MockRAMDirectory>(rd1);
    RAMDirectoryPtr rdOut = newLucene<MockRAMDirectory>();

    IndexWriterPtr iwOut = newLucene<IndexWriter>(rdOut, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(rd1,true));
    pr->add(IndexReader::open(rd2,true));

    iwOut->addIndexes(newCollection<IndexReaderPtr>(pr));

    iwOut->optimize();
    iwOut->close();
    checkIndex(rdOut);
    rdOut->close();
    rd1->close();
    rd2->close();
}

/// This method creates an empty index (numFields=0, numDocs=0) but is marked to have TermVectors.
/// Adding this index to another index should not throw any exception.
TEST_F(ParallelReaderEmptyIndexTest, testEmptyIndexWithVectors) {
    RAMDirectoryPtr rd1 = newLucene<MockRAMDirectory>();
    {
        IndexWriterPtr iw = newLucene<IndexWriter>(rd1, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"test", L"", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
        iw->addDocument(doc);
        doc->add(newLucene<Field>(L"test", L"", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
        iw->addDocument(doc);
        iw->close();

        IndexReaderPtr ir = IndexReader::open(rd1,false);
        ir->deleteDocument(0);
        ir->close();

        iw = newLucene<IndexWriter>(rd1, newLucene<SimpleAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
        iw->optimize();
        iw->close();
    }

    RAMDirectoryPtr rd2 = newLucene<MockRAMDirectory>();
    {
        IndexWriterPtr iw = newLucene<IndexWriter>(rd2, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
        DocumentPtr doc = newLucene<Document>();
        iw->addDocument(doc);
        iw->close();
    }

    RAMDirectoryPtr rdOut = newLucene<MockRAMDirectory>();

    IndexWriterPtr iwOut = newLucene<IndexWriter>(rdOut, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(rd1, true));
    pr->add(IndexReader::open(rd2, true));

    iwOut->addIndexes(newCollection<IndexReaderPtr>(pr));

    // ParallelReader closes any IndexReader you added to it
    pr->close();

    rd1->close();
    rd2->close();

    iwOut->optimize();
    iwOut->close();

    checkIndex(rdOut);
    rdOut->close();
}
