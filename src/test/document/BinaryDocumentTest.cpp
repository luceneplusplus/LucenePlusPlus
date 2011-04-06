/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Field.h"
#include "Document.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "IndexReader.h"
#include "CompressionTools.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(BinaryDocumentTest, LuceneTestFixture)

static String binaryValStored = L"this text will be stored as a byte array in the index";
static String binaryValCompressed = L"this text will be also stored and compressed as a byte array in the index";

BOOST_AUTO_TEST_CASE(testBinaryFieldInIndex)
{
    ByteArray binaryStored = ByteArray::newInstance(binaryValStored.length() * sizeof(wchar_t));
    std::wcsncpy((wchar_t*)binaryStored.get(), binaryValStored.c_str(), binaryValStored.length());
    
    FieldablePtr binaryFldStored = newLucene<Field>(L"binaryStored", binaryStored, Field::STORE_YES);
    FieldablePtr stringFldStored = newLucene<Field>(L"stringStored", binaryValStored, Field::STORE_YES, Field::INDEX_NO, Field::TERM_VECTOR_NO);
    
    // binary fields with store off are not allowed
    BOOST_CHECK_EXCEPTION(newLucene<Field>(L"fail", binaryStored, Field::STORE_NO), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));

    DocumentPtr doc = newLucene<Document>();

    doc->add(binaryFldStored);
    doc->add(stringFldStored);

    // test for field count
    BOOST_CHECK_EQUAL(2, doc->getFields().size());

    // add the doc to a ram index
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    writer->close();

    // open a reader and fetch the document
    IndexReaderPtr reader = IndexReader::open(dir, false);
    DocumentPtr docFromReader = reader->document(0);
    BOOST_CHECK(docFromReader);

    // fetch the binary stored field and compare it's content with the original one
    ByteArray storedTest = docFromReader->getBinaryValue(L"binaryStored");
    String binaryFldStoredTest((wchar_t*)storedTest.get(), storedTest.size() / sizeof(wchar_t));
    BOOST_CHECK_EQUAL(binaryFldStoredTest, binaryValStored);

    // fetch the string field and compare it's content with the original one
    String stringFldStoredTest = docFromReader->get(L"stringStored");
    BOOST_CHECK_EQUAL(stringFldStoredTest, binaryValStored);

    // delete the document from index
    reader->deleteDocument(0);
    BOOST_CHECK_EQUAL(0, reader->numDocs());

    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testCompressionTools)
{
    ByteArray binaryCompressed = ByteArray::newInstance(binaryValCompressed.length() * sizeof(wchar_t));
    std::wcsncpy((wchar_t*)binaryCompressed.get(), binaryValCompressed.c_str(), binaryValCompressed.length());
    
    FieldablePtr binaryFldCompressed = newLucene<Field>(L"binaryCompressed", CompressionTools::compress(binaryCompressed), Field::STORE_YES);
    FieldablePtr stringFldCompressed = newLucene<Field>(L"stringCompressed", CompressionTools::compressString(binaryValCompressed), Field::STORE_YES);
    
    DocumentPtr doc = newLucene<Document>();

    doc->add(binaryFldCompressed);
    doc->add(stringFldCompressed);

    // add the doc to a ram index
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->addDocument(doc);
    writer->close();

    // open a reader and fetch the document
    IndexReaderPtr reader = IndexReader::open(dir, false);
    DocumentPtr docFromReader = reader->document(0);
    BOOST_CHECK(docFromReader);

    // fetch the binary compressed field and compare it's content with the original one
    ByteArray compressTest = CompressionTools::decompress(docFromReader->getBinaryValue(L"binaryCompressed"));
    String binaryFldCompressedTest((wchar_t*)compressTest.get(), compressTest.size() / sizeof(wchar_t));
    BOOST_CHECK_EQUAL(binaryFldCompressedTest, binaryValCompressed);
    
    BOOST_CHECK_EQUAL(CompressionTools::decompressString(docFromReader->getBinaryValue(L"stringCompressed")), binaryValCompressed);

    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
