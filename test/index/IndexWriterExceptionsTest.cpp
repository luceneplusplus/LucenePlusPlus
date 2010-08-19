/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "LuceneThread.h"
#include "Document.h"
#include "Field.h"
#include "CloseableThreadLocal.h"
#include "Term.h"
#include "MockRAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "ConcurrentMergeScheduler.h"
#include "Random.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(IndexWriterExceptionsTest, LuceneTestFixture)

static CloseableThreadLocal<LuceneThread> doFail;

DECLARE_SHARED_PTR(IndexerThread)

class IndexerThread : public LuceneThread
{
public:
    IndexerThread(IndexWriterPtr writer)
    {
        this->writer = writer;
        this->r = newLucene<Random>(47);
    }
    
    virtual ~IndexerThread()
    {
    }
    
    LUCENE_CLASS(IndexerThread);
    
public:
    IndexWriterPtr writer;
    LuceneException failure;
    RandomPtr r;

public:
    virtual void run()
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"content1", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"content6", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        doc->add(newLucene<Field>(L"content2", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"content3", L"aaa bbb ccc ddd", Field::STORE_YES, Field::INDEX_NO));
        
        doc->add(newLucene<Field>(L"content4", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"content5", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        
        doc->add(newLucene<Field>(L"content7", L"aaa bbb ccc ddd", Field::STORE_NO, Field::INDEX_NOT_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        
        FieldPtr idField = newLucene<Field>(L"id", L"", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
        doc->add(idField);
        
        int64_t stopTime = MiscUtils::currentTimeMillis() + 3000;
        
        while ((int64_t)MiscUtils::currentTimeMillis() < stopTime)
        {
            doFail.set(shared_from_this());
            String id = StringUtils::toString(r->nextInt(50));
            idField->setValue(id);
            TermPtr idTerm = newLucene<Term>(L"id", id);
            try
            {
                writer->updateDocument(idTerm, doc);
            }
            catch (RuntimeException&)
            {
                try
                {
                    checkIndex(writer->getDirectory());
                }
                catch (IOException& ioe)
                {
                    failure = ioe;
                    break;
                }				
            }
            catch (LuceneException& e)
            {
                failure = e;
                break;
            }
            
            doFail.set(LuceneThreadPtr());
            
            // After a possible exception (above) I should be able to add a new document 
            // without hitting an exception
            try
            {
                writer->updateDocument(idTerm, doc);
            }
            catch (LuceneException& e)
            {
                failure = e;
                break;
            }
        }
    }
};

class MockIndexWriter : public IndexWriter
{
public:
    MockIndexWriter(DirectoryPtr dir, AnalyzerPtr a, bool create, int32_t mfl) : IndexWriter(dir, a, create, mfl)
    {
        this->r = newLucene<Random>(17);
    }
    
    virtual ~MockIndexWriter()
    {
    }

protected:
    RandomPtr r;

public:
    virtual bool testPoint(const String& name)
    {
        if (doFail.get() && name != L"startDoFlush" && r->nextInt(20) == 17)
            boost::throw_exception(RuntimeException(L"intentionally failing at " + name));
        return true;
    }
};

BOOST_AUTO_TEST_CASE(testRandomExceptions)
{
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();
        
    writer->setRAMBufferSizeMB(0.1);
    
    IndexerThreadPtr thread = newLucene<IndexerThread>(writer);
    thread->run();
    
    if (!thread->failure.isNull())
        BOOST_FAIL("thread hit unexpected failure");
    
    writer->commit();
    
    try
    {
        writer->close();
    }
    catch (LuceneException&)
    {
        writer->rollback();
    }
    
    // Confirm that when doc hits exception partway through tokenization, it's deleted
    IndexReaderPtr r2 = IndexReader::open(dir, true);
    int32_t count = r2->docFreq(newLucene<Term>(L"content4", L"aaa"));
    int32_t count2 = r2->docFreq(newLucene<Term>(L"content4", L"ddd"));
    BOOST_CHECK_EQUAL(count, count2);
    r2->close();

    checkIndex(dir);
}

BOOST_AUTO_TEST_CASE(testRandomExceptionsThreads)
{
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<MockIndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->setSuppressExceptions();
        
    writer->setRAMBufferSizeMB(0.2);
    
    int32_t NUM_THREADS = 4;
    
    Collection<IndexerThreadPtr> threads = Collection<IndexerThreadPtr>::newInstance(NUM_THREADS);
    for (int32_t i = 0; i < NUM_THREADS; ++i)
    {
        threads[i] = newLucene<IndexerThread>(writer);
        threads[i]->start();
    }
    
    for (int32_t i = 0; i < NUM_THREADS; ++i)
        threads[i]->join();
    
    for (int32_t i = 0; i < NUM_THREADS; ++i)
    {
        if (!threads[i]->failure.isNull())
            BOOST_FAIL("thread hit unexpected failure");
    }
    
    writer->commit();

    try
    {
        writer->close();
    }
    catch (LuceneException&)
    {
        writer->rollback();
    }
    
    // Confirm that when doc hits exception partway through tokenization, it's deleted
    IndexReaderPtr r2 = IndexReader::open(dir, true);
    int32_t count = r2->docFreq(newLucene<Term>(L"content4", L"aaa"));
    int32_t count2 = r2->docFreq(newLucene<Term>(L"content4", L"ddd"));
    BOOST_CHECK_EQUAL(count, count2);
    r2->close();

    checkIndex(dir);
}

BOOST_AUTO_TEST_SUITE_END()
