/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "MockRAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "TermEnum.h"
#include "Term.h"
#include "SegmentReader.h"
#include "SegmentTermEnum.h"

using namespace Lucene;

typedef LuceneTestFixture SegmentTermEnumTest;

static void addDoc(const IndexWriterPtr& writer, const String& value) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", value, Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void verifyDocFreq(const DirectoryPtr& dir) {
    IndexReaderPtr reader = IndexReader::open(dir, true);

    // create enumeration of all terms
    TermEnumPtr termEnum = reader->terms();
    // go to the first term (aaa)
    termEnum->next();
    // assert that term is 'aaa'
    EXPECT_EQ(L"aaa", termEnum->term()->text());
    EXPECT_EQ(200, termEnum->docFreq());
    // go to the second term (bbb)
    termEnum->next();
    // assert that term is 'bbb'
    EXPECT_EQ(L"bbb", termEnum->term()->text());
    EXPECT_EQ(100, termEnum->docFreq());

    termEnum->close();

    // create enumeration of terms after term 'aaa', including 'aaa'
    termEnum = reader->terms(newLucene<Term>(L"content", L"aaa"));
    // assert that term is 'aaa'
    EXPECT_EQ(L"aaa", termEnum->term()->text());
    EXPECT_EQ(200, termEnum->docFreq());
    // go to term 'bbb'
    termEnum->next();
    // assert that term is 'bbb'
    EXPECT_EQ(L"bbb", termEnum->term()->text());
    EXPECT_EQ(100, termEnum->docFreq());

    termEnum->close();
}

TEST_F(SegmentTermEnumTest, testTermEnum) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    // ADD 100 documents with term : aaa
    // add 100 documents with terms: aaa bbb
    // Therefore, term 'aaa' has document frequency of 200 and term 'bbb' 100
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer, L"aaa");
        addDoc(writer, L"aaa bbb");
    }

    writer->close();

    // verify document frequency of terms in an unoptimized index
    verifyDocFreq(dir);

    // merge segments by optimizing the index
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();

    // verify document frequency of terms in an optimized index
    verifyDocFreq(dir);
}

TEST_F(SegmentTermEnumTest, testPrevTermAtEnd) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, L"aaa bbb");
    writer->close();
    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(dir);
    SegmentTermEnumPtr termEnum = boost::dynamic_pointer_cast<SegmentTermEnum>(reader->terms());
    EXPECT_TRUE(termEnum->next());
    EXPECT_EQ(L"aaa", termEnum->term()->text());
    EXPECT_TRUE(termEnum->next());
    EXPECT_EQ(L"aaa", termEnum->prev()->text());
    EXPECT_EQ(L"bbb", termEnum->term()->text());
    EXPECT_TRUE(!termEnum->next());
    EXPECT_EQ(L"bbb", termEnum->prev()->text());
}
