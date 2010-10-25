/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DocHelper.h"
#include "Document.h"
#include "MockRAMDirectory.h"
#include "SegmentInfo.h"
#include "SegmentReader.h"
#include "SegmentTermDocs.h"
#include "Term.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "TermDocs.h"
#include "Field.h"

using namespace Lucene;

class SegmentTermDocsTestFixture : public LuceneTestFixture, public DocHelper
{
public:
    SegmentTermDocsTestFixture()
    {
        testDoc = newLucene<Document>();
        dir = newLucene<RAMDirectory>();
        DocHelper::setupDoc(testDoc);
        info = DocHelper::writeDoc(dir, testDoc);
    }
    
    virtual ~SegmentTermDocsTestFixture()
    {
    }

protected:
    DocumentPtr testDoc;
    DirectoryPtr dir;
    SegmentInfoPtr info;

public:
    void checkTermDocs(int32_t indexDivisor)
    {
        // After adding the document, we should be able to read it back in
        SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
        BOOST_CHECK(reader);
        BOOST_CHECK_EQUAL(indexDivisor, reader->getTermInfosIndexDivisor());
        SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
        BOOST_CHECK(segTermDocs);
        segTermDocs->seek(newLucene<Term>(DocHelper::TEXT_FIELD_2_KEY, L"field"));
        if (segTermDocs->next())
        {
            int32_t docId = segTermDocs->doc();
            BOOST_CHECK_EQUAL(docId, 0);
            int32_t freq = segTermDocs->freq();
            BOOST_CHECK_EQUAL(freq, 3);  
        }
        reader->close();
    }
    
    void checkBadSeek(int32_t indexDivisor)
    {
        {
            // After adding the document, we should be able to read it back in
            SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
            BOOST_CHECK(reader);
            SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
            BOOST_CHECK(segTermDocs);
            segTermDocs->seek(newLucene<Term>(L"textField2", L"bad"));
            BOOST_CHECK(!segTermDocs->next());
            reader->close();
        }
        {
            // After adding the document, we should be able to read it back in
            SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
            BOOST_CHECK(reader);
            SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
            BOOST_CHECK(segTermDocs);
            segTermDocs->seek(newLucene<Term>(L"junk", L"bad"));
            BOOST_CHECK(!segTermDocs->next());
            reader->close();
        }
    }
    
    void checkSkipTo(int32_t indexDivisor)
    {
        DirectoryPtr dir = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        TermPtr ta = newLucene<Term>(L"content", L"aaa");
        for (int32_t i = 0; i < 10; ++i)
            addDoc(writer, L"aaa aaa aaa aaa");

        TermPtr tb = newLucene<Term>(L"content", L"bbb");
        for (int32_t i = 0; i < 16; ++i)
            addDoc(writer, L"bbb bbb bbb bbb");

        TermPtr tc = newLucene<Term>(L"content", L"ccc");
        for (int32_t i = 0; i < 50; ++i)
            addDoc(writer, L"ccc ccc ccc ccc");

        // assure that we deal with a single segment  
        writer->optimize();
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, IndexDeletionPolicyPtr(), true, indexDivisor);

        TermDocsPtr tdocs = reader->termDocs();

        // without optimization (assumption skipInterval == 16)

        // with next
        tdocs->seek(ta);
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(0, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(1, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->skipTo(0));
        BOOST_CHECK_EQUAL(2, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(4));
        BOOST_CHECK_EQUAL(4, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(9));
        BOOST_CHECK_EQUAL(9, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(10));

        // without next
        tdocs->seek(ta);
        BOOST_CHECK(tdocs->skipTo(0));
        BOOST_CHECK_EQUAL(0, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(4));
        BOOST_CHECK_EQUAL(4, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(9));
        BOOST_CHECK_EQUAL(9, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(10));

        // exactly skipInterval documents and therefore with optimization

        // with next
        tdocs->seek(tb);
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(10, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(11, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->skipTo(5));
        BOOST_CHECK_EQUAL(12, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(15));
        BOOST_CHECK_EQUAL(15, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(24));
        BOOST_CHECK_EQUAL(24, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(25));
        BOOST_CHECK_EQUAL(25, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(26));

        // without next
        tdocs->seek(tb);
        BOOST_CHECK(tdocs->skipTo(5));
        BOOST_CHECK_EQUAL(10, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(15));
        BOOST_CHECK_EQUAL(15, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(24));
        BOOST_CHECK_EQUAL(24, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(25));
        BOOST_CHECK_EQUAL(25, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(26));

        // much more than skipInterval documents and therefore with optimization

        // with next
        tdocs->seek(tc);
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(26, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->next());
        BOOST_CHECK_EQUAL(27, tdocs->doc());
        BOOST_CHECK_EQUAL(4, tdocs->freq());
        BOOST_CHECK(tdocs->skipTo(5));
        BOOST_CHECK_EQUAL(28, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(40));
        BOOST_CHECK_EQUAL(40, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(57));
        BOOST_CHECK_EQUAL(57, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(74));
        BOOST_CHECK_EQUAL(74, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(75));
        BOOST_CHECK_EQUAL(75, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(76));

        // without next
        tdocs->seek(tc);
        BOOST_CHECK(tdocs->skipTo(5));
        BOOST_CHECK_EQUAL(26, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(40));
        BOOST_CHECK_EQUAL(40, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(57));
        BOOST_CHECK_EQUAL(57, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(74));
        BOOST_CHECK_EQUAL(74, tdocs->doc());
        BOOST_CHECK(tdocs->skipTo(75));
        BOOST_CHECK_EQUAL(75, tdocs->doc());
        BOOST_CHECK(!tdocs->skipTo(76));

        tdocs->close();
        reader->close();
        dir->close();
    }
    
    void addDoc(IndexWriterPtr writer, const String& value)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"content", value, Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
};

BOOST_FIXTURE_TEST_SUITE(SegmentTermDocsTest, SegmentTermDocsTestFixture)

BOOST_AUTO_TEST_CASE(testTermDocs)
{
    checkTermDocs(1);
}

BOOST_AUTO_TEST_CASE(testBadSeek)
{
    checkBadSeek(1);
}

BOOST_AUTO_TEST_CASE(testSkipTo)
{
    checkSkipTo(1);
}

BOOST_AUTO_TEST_CASE(testIndexDivisor)
{
    dir = newLucene<MockRAMDirectory>();
    testDoc = newLucene<Document>();
    DocHelper::setupDoc(testDoc);
    DocHelper::writeDoc(dir, testDoc);
    checkTermDocs(2);
    checkBadSeek(2);
    checkSkipTo(2);
}

BOOST_AUTO_TEST_SUITE_END()
