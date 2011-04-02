/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "ScoreDoc.h"
#include "PhraseQuery.h"
#include "Term.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "SegmentReader.h"
#include "IndexSearcher.h"
#include "IndexInput.h"
#include "TopDocs.h"
#include "TermPositions.h"
#include "IndexReader.h"

using namespace Lucene;

DECLARE_SHARED_PTR(SeeksCountingStream)

/// Simply extends IndexInput in a way that we are able to count the number of invocations of seek()
class SeeksCountingStream : public IndexInput
{
public:
    SeeksCountingStream(IndexInputPtr input)
    {
        this->input = input;
    }
    
    virtual ~SeeksCountingStream()
    {
    }
    
    LUCENE_CLASS(SeeksCountingStream);

protected:
    IndexInputPtr input;

public:
    virtual uint8_t readByte()
    {
        return input->readByte();
    }
    
    virtual void readBytes(uint8_t* b, int32_t offset, int32_t length)
    {
        input->readBytes(b, offset, length);
    }
    
    virtual void close()
    {
        input->close();
    }
    
    virtual int64_t getFilePointer()
    {
        return input->getFilePointer();
    }
    
    virtual void seek(int64_t pos); // implemented below
    
    virtual int64_t length()
    {
        return input->length();
    }
    
    LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr())
    {
        return newLucene<SeeksCountingStream>(boost::dynamic_pointer_cast<IndexInput>(input->clone()));
    }
};

class SeekCountingDirectory : public RAMDirectory
{
public:
    virtual ~SeekCountingDirectory()
    {
    }

public:
    virtual IndexInputPtr openInput(const String& name)
    {
        IndexInputPtr ii = RAMDirectory::openInput(name);
        if (boost::ends_with(name, L".prx"))
        {
            // we decorate the proxStream with a wrapper class that allows to count the number of calls of seek()
            ii = newLucene<SeeksCountingStream>(ii);
        }
        return ii;
    }
};

class LazyProxSkippingFixture : public LuceneTestFixture
{
public:
    LazyProxSkippingFixture()
    {
        seeksCounter = 0;
        field = L"tokens";
        term1 = L"xx";
        term2 = L"yy";
        term3 = L"zz";
    }
    
    virtual ~LazyProxSkippingFixture()
    {
    }

protected:
    SearcherPtr searcher;
    
    String field;
    String term1;
    String term2;
    String term3;

public:
    static int32_t seeksCounter;

public:
    void createIndex(int32_t numHits)
    {
        int32_t numDocs = 500;

        DirectoryPtr directory = newLucene<SeekCountingDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setUseCompoundFile(false);
        writer->setMaxBufferedDocs(10);
        for (int32_t i = 0; i < numDocs; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            String content;
            if (i % (numDocs / numHits) == 0)
            {
                // add a document that matches the query "term1 term2"
                content = term1 + L" " + term2;
            }
            else if (i % 15 == 0)
            {
                // add a document that only contains term1
                content = term1 + L" " + term1;
            }
            else
            {
                // add a document that contains term2 but not term 1
                content = term3 + L" " + term2;
            }
            
            doc->add(newLucene<Field>(field, content, Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        
        // make sure the index has only a single segment
        writer->optimize();
        writer->close();

        SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(directory);
        searcher = newLucene<IndexSearcher>(reader);        
    }
    
    Collection<ScoreDocPtr> search()
    {
        // create PhraseQuery "term1 term2" and search
        PhraseQueryPtr pq = newLucene<PhraseQuery>();
        pq->add(newLucene<Term>(field, term1));
        pq->add(newLucene<Term>(field, term2));
        return searcher->search(pq, FilterPtr(), 1000)->scoreDocs;
    }
    
    void performTest(int32_t numHits)
    {
        createIndex(numHits);
        seeksCounter = 0;
        Collection<ScoreDocPtr> hits = search();
        // verify that the right number of docs was found
        BOOST_CHECK_EQUAL(numHits, hits.size());

        // check if the number of calls of seek() does not exceed the number of hits
        BOOST_CHECK(seeksCounter > 0);
        BOOST_CHECK(seeksCounter <= numHits + 1);
    }
};

int32_t LazyProxSkippingFixture::seeksCounter = 0;

void SeeksCountingStream::seek(int64_t pos)
{
    ++LazyProxSkippingFixture::seeksCounter;
    input->seek(pos);
}

/// Tests lazy skipping on the proximity file.
BOOST_FIXTURE_TEST_SUITE(LazyProxSkippingTest, LazyProxSkippingFixture)

BOOST_AUTO_TEST_CASE(testLazySkipping)
{
    // test whether only the minimum amount of seeks() are performed
    performTest(5);
    performTest(10);
}

BOOST_AUTO_TEST_CASE(testSeek)
{
    DirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 10; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(field, L"a b", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    
    writer->close();
    IndexReaderPtr reader = IndexReader::open(directory, true);
    TermPositionsPtr tp = reader->termPositions();
    tp->seek(newLucene<Term>(field, L"b"));
    for (int32_t i = 0; i < 10; ++i)
    {
        tp->next();
        BOOST_CHECK_EQUAL(tp->doc(), i);
        BOOST_CHECK_EQUAL(tp->nextPosition(), 1);
    }
    tp->seek(newLucene<Term>(field, L"a"));
    for (int32_t i = 0; i < 10; ++i)
    {
        tp->next();
        BOOST_CHECK_EQUAL(tp->doc(), i);
        BOOST_CHECK_EQUAL(tp->nextPosition(), 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()
