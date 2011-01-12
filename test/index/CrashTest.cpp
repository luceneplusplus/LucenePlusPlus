/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "NoLockFactory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "ConcurrentMergeScheduler.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CrashTest, LuceneTestFixture)

static IndexWriterPtr initIndex(MockRAMDirectoryPtr dir)
{
    dir->setLockFactory(NoLockFactory::getNoLockFactory());

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);      

    writer->setMaxBufferedDocs(10);
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", L"0", Field::STORE_YES, Field::INDEX_ANALYZED));
    for (int32_t i = 0; i < 157; ++i)
        writer->addDocument(doc);

    return writer;
}

static IndexWriterPtr initIndex()
{
    return initIndex(newLucene<MockRAMDirectory>());
}

static void crash(IndexWriterPtr writer)
{
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    ConcurrentMergeSchedulerPtr cms = boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler());
    dir->crash();
    cms->sync();
    dir->clearCrash();
}

BOOST_AUTO_TEST_CASE(testCrashWhileIndexing)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    crash(writer);
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK(reader->numDocs() < 157);
}

BOOST_AUTO_TEST_CASE(testWriterAfterCrash)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    dir->setPreventDoubleWrite(false);
    crash(writer);
    writer = initIndex();
    writer->close();
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK(reader->numDocs() < 314);
}

BOOST_AUTO_TEST_CASE(testCrashAfterReopen)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    writer->close();
    writer = initIndex(dir);
    BOOST_CHECK_EQUAL(314, writer->maxDoc());
    crash(writer);
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK(reader->numDocs() >= 157);
}

BOOST_AUTO_TEST_CASE(testCrashAfterClose)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    writer->close();
    dir->crash();
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(157, reader->numDocs());
}

BOOST_AUTO_TEST_CASE(testCrashAfterCloseNoWait)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    writer->close(false);
    dir->crash();
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(157, reader->numDocs());
}

BOOST_AUTO_TEST_CASE(testCrashReaderDeletes)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    writer->close(false);
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(3);
    
    dir->crash();
    
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(157, reader->numDocs());
}

BOOST_AUTO_TEST_CASE(testCrashReaderDeletesAfterClose)
{
    IndexWriterPtr writer = initIndex();
    MockRAMDirectoryPtr dir = boost::dynamic_pointer_cast<MockRAMDirectory>(writer->getDirectory());
    writer->close(false);
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(3);
    reader->close();
    
    dir->crash();
    
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(156, reader->numDocs());
}

BOOST_AUTO_TEST_SUITE_END()
