/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

class SegmentTermDocsTest : public LuceneTestFixture, public DocHelper {
public:
    SegmentTermDocsTest() {
        testDoc = newLucene<Document>();
        dir = newLucene<RAMDirectory>();
        DocHelper::setupDoc(testDoc);
        info = DocHelper::writeDoc(dir, testDoc);
    }

    virtual ~SegmentTermDocsTest() {
    }

protected:
    DocumentPtr testDoc;
    DirectoryPtr dir;
    SegmentInfoPtr info;

public:
    void checkTermDocs(int32_t indexDivisor) {
        // After adding the document, we should be able to read it back in
        SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
        EXPECT_TRUE(reader);
        EXPECT_EQ(indexDivisor, reader->getTermInfosIndexDivisor());
        SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
        EXPECT_TRUE(segTermDocs);
        segTermDocs->seek(newLucene<Term>(DocHelper::TEXT_FIELD_2_KEY, L"field"));
        if (segTermDocs->next()) {
            int32_t docId = segTermDocs->doc();
            EXPECT_EQ(docId, 0);
            int32_t freq = segTermDocs->freq();
            EXPECT_EQ(freq, 3);
        }
        reader->close();
    }

    void checkBadSeek(int32_t indexDivisor) {
        {
            // After adding the document, we should be able to read it back in
            SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
            EXPECT_TRUE(reader);
            SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
            EXPECT_TRUE(segTermDocs);
            segTermDocs->seek(newLucene<Term>(L"textField2", L"bad"));
            EXPECT_TRUE(!segTermDocs->next());
            reader->close();
        }
        {
            // After adding the document, we should be able to read it back in
            SegmentReaderPtr reader = SegmentReader::get(true, info, indexDivisor);
            EXPECT_TRUE(reader);
            SegmentTermDocsPtr segTermDocs = newLucene<SegmentTermDocs>(reader);
            EXPECT_TRUE(segTermDocs);
            segTermDocs->seek(newLucene<Term>(L"junk", L"bad"));
            EXPECT_TRUE(!segTermDocs->next());
            reader->close();
        }
    }

    void checkSkipTo(int32_t indexDivisor) {
        DirectoryPtr dir = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        TermPtr ta = newLucene<Term>(L"content", L"aaa");
        for (int32_t i = 0; i < 10; ++i) {
            addDoc(writer, L"aaa aaa aaa aaa");
        }

        TermPtr tb = newLucene<Term>(L"content", L"bbb");
        for (int32_t i = 0; i < 16; ++i) {
            addDoc(writer, L"bbb bbb bbb bbb");
        }

        TermPtr tc = newLucene<Term>(L"content", L"ccc");
        for (int32_t i = 0; i < 50; ++i) {
            addDoc(writer, L"ccc ccc ccc ccc");
        }

        // assure that we deal with a single segment
        writer->optimize();
        writer->close();

        IndexReaderPtr reader = IndexReader::open(dir, IndexDeletionPolicyPtr(), true, indexDivisor);

        TermDocsPtr tdocs = reader->termDocs();

        // without optimization (assumption skipInterval == 16)

        // with next
        tdocs->seek(ta);
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(0, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(1, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->skipTo(0));
        EXPECT_EQ(2, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(4));
        EXPECT_EQ(4, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(9));
        EXPECT_EQ(9, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(10));

        // without next
        tdocs->seek(ta);
        EXPECT_TRUE(tdocs->skipTo(0));
        EXPECT_EQ(0, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(4));
        EXPECT_EQ(4, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(9));
        EXPECT_EQ(9, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(10));

        // exactly skipInterval documents and therefore with optimization

        // with next
        tdocs->seek(tb);
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(10, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(11, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->skipTo(5));
        EXPECT_EQ(12, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(15));
        EXPECT_EQ(15, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(24));
        EXPECT_EQ(24, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(25));
        EXPECT_EQ(25, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(26));

        // without next
        tdocs->seek(tb);
        EXPECT_TRUE(tdocs->skipTo(5));
        EXPECT_EQ(10, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(15));
        EXPECT_EQ(15, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(24));
        EXPECT_EQ(24, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(25));
        EXPECT_EQ(25, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(26));

        // much more than skipInterval documents and therefore with optimization

        // with next
        tdocs->seek(tc);
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(26, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->next());
        EXPECT_EQ(27, tdocs->doc());
        EXPECT_EQ(4, tdocs->freq());
        EXPECT_TRUE(tdocs->skipTo(5));
        EXPECT_EQ(28, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(40));
        EXPECT_EQ(40, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(57));
        EXPECT_EQ(57, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(74));
        EXPECT_EQ(74, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(75));
        EXPECT_EQ(75, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(76));

        // without next
        tdocs->seek(tc);
        EXPECT_TRUE(tdocs->skipTo(5));
        EXPECT_EQ(26, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(40));
        EXPECT_EQ(40, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(57));
        EXPECT_EQ(57, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(74));
        EXPECT_EQ(74, tdocs->doc());
        EXPECT_TRUE(tdocs->skipTo(75));
        EXPECT_EQ(75, tdocs->doc());
        EXPECT_TRUE(!tdocs->skipTo(76));

        tdocs->close();
        reader->close();
        dir->close();
    }

    void addDoc(const IndexWriterPtr& writer, const String& value) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"content", value, Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
};

TEST_F(SegmentTermDocsTest, testTermDocs) {
    checkTermDocs(1);
}

TEST_F(SegmentTermDocsTest, testBadSeek) {
    checkBadSeek(1);
}

TEST_F(SegmentTermDocsTest, testSkipTo) {
    checkSkipTo(1);
}

TEST_F(SegmentTermDocsTest, testIndexDivisor) {
    dir = newLucene<MockRAMDirectory>();
    testDoc = newLucene<Document>();
    DocHelper::setupDoc(testDoc);
    DocHelper::writeDoc(dir, testDoc);
    checkTermDocs(2);
    checkBadSeek(2);
    checkSkipTo(2);
}
