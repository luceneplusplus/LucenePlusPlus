/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(SegmentTermEnumTest, LuceneTestFixture)

static void addDoc(IndexWriterPtr writer, const String& value)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", value, Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void verifyDocFreq(DirectoryPtr dir)
{
    IndexReaderPtr reader = IndexReader::open(dir, true);

    // create enumeration of all terms
    TermEnumPtr termEnum = reader->terms();
    // go to the first term (aaa)
    termEnum->next();
    // assert that term is 'aaa'
    BOOST_CHECK_EQUAL(L"aaa", termEnum->term()->text());
    BOOST_CHECK_EQUAL(200, termEnum->docFreq());
    // go to the second term (bbb)
    termEnum->next();
    // assert that term is 'bbb'
    BOOST_CHECK_EQUAL(L"bbb", termEnum->term()->text());
    BOOST_CHECK_EQUAL(100, termEnum->docFreq());

    termEnum->close();

    // create enumeration of terms after term 'aaa', including 'aaa'
    termEnum = reader->terms(newLucene<Term>(L"content", L"aaa"));
    // assert that term is 'aaa'
    BOOST_CHECK_EQUAL(L"aaa", termEnum->term()->text());
    BOOST_CHECK_EQUAL(200, termEnum->docFreq());
    // go to term 'bbb'
    termEnum->next();
    // assert that term is 'bbb'
    BOOST_CHECK_EQUAL(L"bbb", termEnum->term()->text());
    BOOST_CHECK_EQUAL(100, termEnum->docFreq());

    termEnum->close();
}

BOOST_AUTO_TEST_CASE(testTermEnum)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    
    // ADD 100 documents with term : aaa
    // add 100 documents with terms: aaa bbb
    // Therefore, term 'aaa' has document frequency of 200 and term 'bbb' 100
    for (int32_t i = 0; i < 100; ++i)
    {
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

BOOST_AUTO_TEST_CASE(testPrevTermAtEnd)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer  = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, L"aaa bbb");
    writer->close();
    SegmentReaderPtr reader = SegmentReader::getOnlySegmentReader(dir);
    SegmentTermEnumPtr termEnum = boost::dynamic_pointer_cast<SegmentTermEnum>(reader->terms());
    BOOST_CHECK(termEnum->next());
    BOOST_CHECK_EQUAL(L"aaa", termEnum->term()->text());
    BOOST_CHECK(termEnum->next());
    BOOST_CHECK_EQUAL(L"aaa", termEnum->prev()->text());
    BOOST_CHECK_EQUAL(L"bbb", termEnum->term()->text());
    BOOST_CHECK(!termEnum->next());
    BOOST_CHECK_EQUAL(L"bbb", termEnum->prev()->text());
}

BOOST_AUTO_TEST_SUITE_END()
