/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Document.h"
#include "Field.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "Term.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(DocumentTest, LuceneTestFixture)

static String binaryVal = L"this text will be stored as a byte array in the index";
static String binaryVal2 = L"this text will be also stored as a byte array in the index";

static DocumentPtr makeDocumentWithFields()
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"keyword", L"test1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"keyword", L"test2", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"text", L"test1", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"text", L"test2", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"unindexed", L"test1", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"unindexed", L"test2", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"unstored", L"test1", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"unstored", L"test2", Field::STORE_NO, Field::INDEX_ANALYZED));
    return doc;
}

static void checkDocument(DocumentPtr doc, bool fromIndex)
{
    Collection<String> keywordFieldValues = doc->getValues(L"keyword");
    Collection<String> textFieldValues = doc->getValues(L"text");
    Collection<String> unindexedFieldValues = doc->getValues(L"unindexed");
    Collection<String> unstoredFieldValues = doc->getValues(L"unstored");

    BOOST_CHECK_EQUAL(keywordFieldValues.size(), 2);
    BOOST_CHECK_EQUAL(textFieldValues.size(), 2);
    BOOST_CHECK_EQUAL(unindexedFieldValues.size(), 2);
    // this test cannot work for documents retrieved from the index since unstored fields will obviously not be returned
    if (!fromIndex)
        BOOST_CHECK_EQUAL(unstoredFieldValues.size(), 2);

    BOOST_CHECK_EQUAL(keywordFieldValues[0], L"test1");
    BOOST_CHECK_EQUAL(keywordFieldValues[1], L"test2");
    BOOST_CHECK_EQUAL(textFieldValues[0], L"test1");
    BOOST_CHECK_EQUAL(textFieldValues[1], L"test2");
    BOOST_CHECK_EQUAL(unindexedFieldValues[0], L"test1");
    BOOST_CHECK_EQUAL(unindexedFieldValues[1], L"test2");
    // this test cannot work for documents retrieved from the index since unstored fields will obviously not be returned
    if (!fromIndex)
    {
        BOOST_CHECK_EQUAL(unstoredFieldValues[0], L"test1");
        BOOST_CHECK_EQUAL(unstoredFieldValues[1], L"test2");
    }
}
    
BOOST_AUTO_TEST_CASE(testBinaryField)
{
    DocumentPtr doc = newLucene<Document>();
    FieldablePtr stringFld = newLucene<Field>(L"string", binaryVal, Field::STORE_YES, Field::INDEX_NO);

    ByteArray binaryBytes1 = ByteArray::newInstance(binaryVal.length() * sizeof(wchar_t));
    std::wcsncpy((wchar_t*)binaryBytes1.get(), binaryVal.c_str(), binaryVal.length());
    FieldablePtr binaryFld = newLucene<Field>(L"binary", binaryBytes1, Field::STORE_YES);

    ByteArray binaryBytes2 = ByteArray::newInstance(binaryVal2.length() * sizeof(wchar_t));
    std::wcsncpy((wchar_t*)binaryBytes2.get(), binaryVal2.c_str(), binaryVal2.length());
    FieldablePtr binaryFld2 = newLucene<Field>(L"binary", binaryBytes2, Field::STORE_YES);

    doc->add(stringFld);
    doc->add(binaryFld);

    BOOST_CHECK_EQUAL(2, doc->getFields().size());

    BOOST_CHECK(binaryFld->isBinary());
    BOOST_CHECK(binaryFld->isStored());
    BOOST_CHECK(!binaryFld->isIndexed());
    BOOST_CHECK(!binaryFld->isTokenized());

    ByteArray bytesTest = doc->getBinaryValue(L"binary");
    String binaryTest((wchar_t*)bytesTest.get(), bytesTest.size() / sizeof(wchar_t));
    BOOST_CHECK_EQUAL(binaryTest, binaryVal);

    String stringTest = doc->get(L"string");
    BOOST_CHECK_EQUAL(binaryTest, stringTest);

    doc->add(binaryFld2);

    BOOST_CHECK_EQUAL(3, doc->getFields().size());

    Collection<ByteArray> binaryTests = doc->getBinaryValues(L"binary");

    BOOST_CHECK_EQUAL(2, binaryTests.size());

    bytesTest = binaryTests[0];
    binaryTest = String((wchar_t*)bytesTest.get(), bytesTest.size() / sizeof(wchar_t));
    
    ByteArray bytesTest2 = binaryTests[1];
    String binaryTest2((wchar_t*)bytesTest2.get(), bytesTest2.size() / sizeof(wchar_t));
    
    BOOST_CHECK_NE(binaryTest, binaryTest2);

    BOOST_CHECK_EQUAL(binaryTest, binaryVal);
    BOOST_CHECK_EQUAL(binaryTest2, binaryVal2);

    doc->removeField(L"string");
    BOOST_CHECK_EQUAL(2, doc->getFields().size());

    doc->removeFields(L"binary");
    BOOST_CHECK_EQUAL(0, doc->getFields().size());
}

/// Tests {@link Document#removeField(String)} method for a brand new Document that has not been indexed yet.
BOOST_AUTO_TEST_CASE(testRemoveForNewDocument)
{
    DocumentPtr doc = makeDocumentWithFields();
    BOOST_CHECK_EQUAL(8, doc->getFields().size());
    doc->removeFields(L"keyword");
    BOOST_CHECK_EQUAL(6, doc->getFields().size());
    doc->removeFields(L"doesnotexists"); // removing non-existing fields is silently ignored
    doc->removeFields(L"keyword"); // removing a field more than once
    BOOST_CHECK_EQUAL(6, doc->getFields().size());
    doc->removeField(L"text");
    BOOST_CHECK_EQUAL(5, doc->getFields().size());
    doc->removeField(L"text");
    BOOST_CHECK_EQUAL(4, doc->getFields().size());
    doc->removeField(L"text");
    BOOST_CHECK_EQUAL(4, doc->getFields().size());
    doc->removeField(L"doesnotexists"); // removing non-existing fields is silently ignored
    BOOST_CHECK_EQUAL(4, doc->getFields().size());
    doc->removeFields(L"unindexed");
    BOOST_CHECK_EQUAL(2, doc->getFields().size());
    doc->removeFields(L"unstored");
    BOOST_CHECK_EQUAL(0, doc->getFields().size());
    doc->removeFields(L"doesnotexists"); // removing non-existing fields is silently ignored
    BOOST_CHECK_EQUAL(0, doc->getFields().size());
}

BOOST_AUTO_TEST_CASE(testConstructorExceptions)
{
    newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_NO); // ok
    newLucene<Field>(L"name", L"value", Field::STORE_NO, Field::INDEX_NOT_ANALYZED); // ok
    
    BOOST_CHECK_EXCEPTION(newLucene<Field>(L"name", L"value", Field::STORE_NO, Field::INDEX_NO), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
    
    newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_NO, Field::TERM_VECTOR_NO); // ok
    
    BOOST_CHECK_EXCEPTION(newLucene<Field>(L"name", L"value", Field::STORE_YES, Field::INDEX_NO, Field::TERM_VECTOR_YES), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
}

/// Tests {@link Document#getValues(String)} method for a brand new Document that has not been indexed yet.
BOOST_AUTO_TEST_CASE(testGetValuesForNewDocument)
{
    checkDocument(makeDocumentWithFields(), false);
}

/// Tests {@link Document#getValues(String)} method for a Document retrieved from an index.
BOOST_AUTO_TEST_CASE(testGetValuesForIndexedDocument)
{
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(makeDocumentWithFields());
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(dir, true);

    // search for something that does exists
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"keyword", L"test1"));

    // ensure that queries return expected results without DateFilter first
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());

    checkDocument(searcher->doc(hits[0]->doc), true);
    searcher->close();
}

BOOST_AUTO_TEST_CASE(testFieldSetValue)
{
    FieldPtr field = newLucene<Field>(L"id", L"id1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(field);
    doc->add(newLucene<Field>(L"keyword", L"test", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    field->setValue(L"id2");
    writer->addDocument(doc);
    field->setValue(L"id3");
    writer->addDocument(doc);
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"keyword", L"test"));

    // ensure that queries return expected results without DateFilter first
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(3, hits.size());
    int32_t result = 0;
    for (int32_t i = 0; i < 3; ++i)
    {
        DocumentPtr doc2 = searcher->doc(hits[i]->doc);
        FieldPtr f = doc2->getField(L"id");
        if (f->stringValue() == L"id1")
            result |= 1;
        else if (f->stringValue() == L"id2")
            result |= 2;
        else if (f->stringValue() == L"id3")
            result |= 4;
        else
            BOOST_FAIL("unexpected id field");
    }
    searcher->close();
    dir->close();
    BOOST_CHECK_EQUAL(7, result);
}

BOOST_AUTO_TEST_CASE(testFieldSetValueChangeBinary)
{
    FieldPtr field1 = newLucene<Field>(L"field1", ByteArray::newInstance(0), Field::STORE_YES);
    FieldPtr field2 = newLucene<Field>(L"field2", L"", Field::STORE_YES, Field::INDEX_ANALYZED);

    BOOST_CHECK_EXCEPTION(field1->setValue(L"abc"), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
    BOOST_CHECK_EXCEPTION(field2->setValue(ByteArray::newInstance(0)), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
}

BOOST_AUTO_TEST_SUITE_END()
