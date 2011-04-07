/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "Document.h"
#include "Field.h"
#include "DateTools.h"
#include "FileReader.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "SegmentInfos.h"
#include "IndexCommit.h"
#include "FieldSortedTermVectorMapper.h"
#include "TermVectorEntryFreqSortedComparator.h"
#include "Term.h"
#include "TermDocs.h"
#include "SetBasedFieldSelector.h"
#include "FieldSelector.h"
#include "FSDirectory.h"
#include "IndexFileDeleter.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TermQuery.h"
#include "SegmentReader.h"
#include "FieldCache.h"
#include "ReadOnlyDirectoryReader.h"
#include "ReadOnlySegmentReader.h"
#include "FileUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(IndexReaderTest, LuceneTestFixture)

static void addDocumentWithFields(IndexWriterPtr writer)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"keyword", L"test1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"text", L"test1", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"unindexed", L"test1", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"unstored", L"test1", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void addDocumentWithDifferentFields(IndexWriterPtr writer)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"keyword2", L"test1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"text2", L"test1", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"unindexed2", L"test1", Field::STORE_YES, Field::INDEX_NO));
    doc->add(newLucene<Field>(L"unstored2", L"test1", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void addDocumentWithTermVectorFields(IndexWriterPtr writer)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"tvnot", L"tvnot", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
    doc->add(newLucene<Field>(L"termvector", L"termvector", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
    doc->add(newLucene<Field>(L"tvoffset", L"tvoffset", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_OFFSETS));
    doc->add(newLucene<Field>(L"tvposition", L"tvposition", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS));
    doc->add(newLucene<Field>(L"tvpositionoffset", L"tvpositionoffset", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
    writer->addDocument(doc);
}

static void addDoc(IndexWriterPtr writer, const String& value)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", value, Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void checkTermDocsCount(IndexReaderPtr reader, TermPtr term, int32_t expected)
{
    TermDocsPtr tdocs;

    LuceneException finally;
    try
    {
        tdocs = reader->termDocs(term);
        BOOST_CHECK(tdocs);
        int32_t count = 0;
        while (tdocs->next())
            ++count;
        BOOST_CHECK_EQUAL(expected, count);
    }
    catch (LuceneException& e)
    {
        finally = e;
    }
    if (tdocs)
        tdocs->close();
    finally.throwException();
}

static DirectoryPtr getDirectory()
{
    return FSDirectory::open(FileUtils::joinPath(getTempDir(), L"testIndex"));
}

static DocumentPtr createDocument(const String& id)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"id", id, Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS));
    return doc;
}
  
BOOST_AUTO_TEST_CASE(testCommitUserData)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();

    MapStringString commitUserData = MapStringString::newInstance();
    commitUserData.put(L"foo", L"fighters");

    // set up writer
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < 27; ++i)
        addDocumentWithFields(writer);
    writer->close();

    IndexReaderPtr r = IndexReader::open(d, false);
    r->deleteDocument(5);
    r->flush(commitUserData);
    r->close();

    SegmentInfosPtr sis = newLucene<SegmentInfos>();
    sis->read(d);
    IndexReaderPtr r2 = IndexReader::open(d, false);
    IndexCommitPtr c = r->getIndexCommit();
    MapStringString expectedData = c->getUserData();
    
    BOOST_CHECK_EQUAL(expectedData.size(), commitUserData.size());
    for (MapStringString::iterator expected = expectedData.begin(); expected != expectedData.end(); ++expected)
        BOOST_CHECK(commitUserData.find(expected->first) != commitUserData.end());
    for (MapStringString::iterator commit = commitUserData.begin(); commit != commitUserData.end(); ++commit)
        BOOST_CHECK(expectedData.find(commit->first) != expectedData.end());

    BOOST_CHECK_EQUAL(sis->getCurrentSegmentFileName(), c->getSegmentsFileName());

    BOOST_CHECK(c->equals(r->getIndexCommit()));

    // Change the index
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < 7; ++i)
        addDocumentWithFields(writer);
    writer->close();

    IndexReaderPtr r3 = r2->reopen();
    BOOST_CHECK(!c->equals(r3->getIndexCommit()));
    BOOST_CHECK(!r2->getIndexCommit()->isOptimized());
    r3->close();

    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();

    r3 = r2->reopen();
    BOOST_CHECK(r3->getIndexCommit()->isOptimized());
    r2->close();
    r3->close();
    d->close();
}

BOOST_AUTO_TEST_CASE(testIsCurrent)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    // set up reader
    IndexReaderPtr reader = IndexReader::open(d, false);
    BOOST_CHECK(reader->isCurrent());
    // modify index by adding another document
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    BOOST_CHECK(!reader->isCurrent());
    // re-create index
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    BOOST_CHECK(!reader->isCurrent());
    reader->close();
    d->close();
}

BOOST_AUTO_TEST_CASE(testGetFieldNames)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();
    // set up writer
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    // set up reader
    IndexReaderPtr reader = IndexReader::open(d, false);
    BOOST_CHECK(reader->isCurrent());
    HashSet<String> fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_ALL);
    BOOST_CHECK(fieldNames.contains(L"keyword"));
    BOOST_CHECK(fieldNames.contains(L"text"));
    BOOST_CHECK(fieldNames.contains(L"unindexed"));
    BOOST_CHECK(fieldNames.contains(L"unstored"));
    reader->close();
    // add more documents
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    // want to get some more segments here
    for (int32_t i = 0; i < 5 * writer->getMergeFactor(); ++i)
        addDocumentWithFields(writer);
    // new fields are in some different segments (we hope)
    for (int32_t i = 0; i < 5 * writer->getMergeFactor(); ++i)
        addDocumentWithDifferentFields(writer);
    // new termvector fields
    for (int32_t i = 0; i < 5 * writer->getMergeFactor(); ++i)
        addDocumentWithTermVectorFields(writer);
    
    writer->close();
    // verify fields again
    reader = IndexReader::open(d, false);
    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_ALL);
    BOOST_CHECK_EQUAL(13, fieldNames.size()); // the following fields
    BOOST_CHECK(fieldNames.contains(L"keyword"));
    BOOST_CHECK(fieldNames.contains(L"text"));
    BOOST_CHECK(fieldNames.contains(L"unindexed"));
    BOOST_CHECK(fieldNames.contains(L"unstored"));
    BOOST_CHECK(fieldNames.contains(L"keyword2"));
    BOOST_CHECK(fieldNames.contains(L"text2"));
    BOOST_CHECK(fieldNames.contains(L"unindexed2"));
    BOOST_CHECK(fieldNames.contains(L"unstored2"));
    BOOST_CHECK(fieldNames.contains(L"tvnot"));
    BOOST_CHECK(fieldNames.contains(L"termvector"));
    BOOST_CHECK(fieldNames.contains(L"tvposition"));
    BOOST_CHECK(fieldNames.contains(L"tvoffset"));
    BOOST_CHECK(fieldNames.contains(L"tvpositionoffset"));

    // verify that only indexed fields were returned
    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_INDEXED);
    BOOST_CHECK_EQUAL(11, fieldNames.size()); // 6 original + the 5 termvector fields 
    BOOST_CHECK(fieldNames.contains(L"keyword"));
    BOOST_CHECK(fieldNames.contains(L"text"));
    BOOST_CHECK(fieldNames.contains(L"unstored"));
    BOOST_CHECK(fieldNames.contains(L"keyword2"));
    BOOST_CHECK(fieldNames.contains(L"text2"));
    BOOST_CHECK(fieldNames.contains(L"unstored2"));
    BOOST_CHECK(fieldNames.contains(L"tvnot"));
    BOOST_CHECK(fieldNames.contains(L"termvector"));
    BOOST_CHECK(fieldNames.contains(L"tvposition"));
    BOOST_CHECK(fieldNames.contains(L"tvoffset"));
    BOOST_CHECK(fieldNames.contains(L"tvpositionoffset"));

    // verify that only unindexed fields were returned
    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_UNINDEXED);
    BOOST_CHECK_EQUAL(2, fieldNames.size()); // the following fields
    BOOST_CHECK(fieldNames.contains(L"unindexed"));
    BOOST_CHECK(fieldNames.contains(L"unindexed2"));

    // verify index term vector fields  
    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR);
    BOOST_CHECK_EQUAL(1, fieldNames.size()); // 1 field has term vector only
    BOOST_CHECK(fieldNames.contains(L"termvector"));

    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_POSITION);
    BOOST_CHECK_EQUAL(1, fieldNames.size()); // 4 fields are indexed with term vectors
    BOOST_CHECK(fieldNames.contains(L"tvposition"));

    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_OFFSET);
    BOOST_CHECK_EQUAL(1, fieldNames.size()); // 4 fields are indexed with term vectors
    BOOST_CHECK(fieldNames.contains(L"tvoffset"));

    fieldNames = reader->getFieldNames(IndexReader::FIELD_OPTION_TERMVECTOR_WITH_POSITION_OFFSET);
    BOOST_CHECK_EQUAL(1, fieldNames.size()); // 4 fields are indexed with term vectors
    BOOST_CHECK(fieldNames.contains(L"tvpositionoffset"));
    reader->close();
    d->close();
}

BOOST_AUTO_TEST_CASE(testTermVectors)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();
    // set up writer
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    // want to get some more segments here
    // new termvector fields
    for (int32_t i = 0; i < 5 * writer->getMergeFactor(); ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"tvnot", L"one two two three three three", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_NO));
        doc->add(newLucene<Field>(L"termvector", L"one two two three three three", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_YES));
        doc->add(newLucene<Field>(L"tvoffset", L"one two two three three three", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_OFFSETS));
        doc->add(newLucene<Field>(L"tvposition", L"one two two three three three", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS));
        doc->add(newLucene<Field>(L"tvpositionoffset", L"one two two three three three", Field::STORE_YES, Field::INDEX_ANALYZED, Field::TERM_VECTOR_WITH_POSITIONS_OFFSETS));
        writer->addDocument(doc);
    }
    writer->close();
    IndexReaderPtr reader = IndexReader::open(d, false);
    FieldSortedTermVectorMapperPtr mapper = newLucene<FieldSortedTermVectorMapper>(TermVectorEntryFreqSortedComparator::compare);
    reader->getTermFreqVector(0, mapper);
    MapStringCollectionTermVectorEntry map = mapper->getFieldToTerms();
    BOOST_CHECK(map);
    BOOST_CHECK_EQUAL(map.size(), 4);
    Collection<TermVectorEntryPtr> set = map.get(L"termvector");
    for (Collection<TermVectorEntryPtr>::iterator entry = set.begin(); entry != set.end(); ++entry)
        BOOST_CHECK(*entry);
}

BOOST_AUTO_TEST_CASE(testBasicDelete)
{
    RAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");

    // add 100 documents with term : aaa
    for (int32_t i = 0; i < 100; ++i)
        addDoc(writer, searchTerm->text());
    writer->close();
    
    // open reader at this point - this should fix the view of the
    // index at the point of having 100 "aaa" documents and 0 "bbb"
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    checkTermDocsCount(reader, searchTerm, 100);
    reader->close();

    // delete documents containing term: aaa
    int32_t deleted = 0;
    reader = IndexReader::open(dir, false);
    deleted = reader->deleteDocuments(searchTerm);
    BOOST_CHECK_EQUAL(100, deleted);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    checkTermDocsCount(reader, searchTerm, 0);

    // open a 2nd reader to make sure first reader can commit its changes (.del) 
    // while second reader is open
    IndexReaderPtr reader2 = IndexReader::open(dir, false);
    reader->close();

    // create a new reader and re-test
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    checkTermDocsCount(reader, searchTerm, 0);
    reader->close();
    reader2->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testBinaryFields)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    uint8_t _bin[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    ByteArray bin(ByteArray::newInstance(10));
    std::memcpy(bin.get(), _bin, 10);
    
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    for (int32_t i = 0; i < 10; ++i)
    {
        addDoc(writer, L"document number " + StringUtils::toString(i + 1));
        addDocumentWithFields(writer);
        addDocumentWithDifferentFields(writer);
        addDocumentWithTermVectorFields(writer);
    }
    writer->close();
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"bin1", bin, Field::STORE_YES));
    doc->add(newLucene<Field>(L"junk", L"junk text", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    doc = reader->document(reader->maxDoc() - 1);
    Collection<FieldPtr> fields = doc->getFields(L"bin1");
    BOOST_CHECK(fields);
    BOOST_CHECK_EQUAL(1, fields.size());
    FieldPtr b1 = fields[0];
    BOOST_CHECK(b1->isBinary());
    ByteArray data1 = b1->getBinaryValue();
    BOOST_CHECK_EQUAL(bin.size(), b1->getBinaryLength());
    BOOST_CHECK(std::memcmp(bin.get(), data1.get() + b1->getBinaryOffset(), bin.size()) == 0);
    HashSet<String> lazyFields = HashSet<String>::newInstance();
    lazyFields.add(L"bin1");
    FieldSelectorPtr sel = newLucene<SetBasedFieldSelector>(HashSet<String>::newInstance(), lazyFields);
    doc = reader->document(reader->maxDoc() - 1, sel);
    Collection<FieldablePtr> fieldables = doc->getFieldables(L"bin1");
    BOOST_CHECK(fieldables);
    BOOST_CHECK_EQUAL(1, fieldables.size());
    FieldablePtr fb1 = fieldables[0];
    BOOST_CHECK(fb1->isBinary());
    BOOST_CHECK_EQUAL(bin.size(), fb1->getBinaryLength());
    data1 = fb1->getBinaryValue();
    BOOST_CHECK_EQUAL(bin.size(), fb1->getBinaryLength());
    BOOST_CHECK(std::memcmp(bin.get(), data1.get() + fb1->getBinaryOffset(), bin.size()) == 0);
    reader->close();
    
    // force optimize
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();
    reader = IndexReader::open(dir, false);
    doc = reader->document(reader->maxDoc() - 1);
    fields = doc->getFields(L"bin1");
    BOOST_CHECK(fields);
    BOOST_CHECK_EQUAL(1, fields.size());
    b1 = fields[0];
    BOOST_CHECK(b1->isBinary());
    data1 = b1->getBinaryValue();
    BOOST_CHECK_EQUAL(bin.size(), b1->getBinaryLength());
    BOOST_CHECK(std::memcmp(bin.get(), data1.get() + b1->getBinaryOffset(), bin.size()) == 0);
    reader->close();
}

/// Make sure attempts to make changes after reader is closed throws IOException
BOOST_AUTO_TEST_CASE(testChangesAfterClose)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    // add 11 documents with term : aaa
    for (int32_t i = 0; i < 11; ++i)
        addDoc(writer, searchTerm->text());
    writer->close();
    
    IndexReaderPtr reader = IndexReader::open(dir, false);

    // Close reader
    reader->close();
    
    // Then, try to make changes
    BOOST_CHECK_EXCEPTION(reader->deleteDocument(4), AlreadyClosedException, check_exception(LuceneException::AlreadyClosed));
    BOOST_CHECK_EXCEPTION(reader->setNorm(5, L"aaa", 2.0), AlreadyClosedException, check_exception(LuceneException::AlreadyClosed));
    BOOST_CHECK_EXCEPTION(reader->undeleteAll(), AlreadyClosedException, check_exception(LuceneException::AlreadyClosed));
}

/// Make sure we get lock obtain failed exception with 2 writers
BOOST_AUTO_TEST_CASE(testLockObtainFailed)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    // add 11 documents with term : aaa
    for (int32_t i = 0; i < 11; ++i)
        addDoc(writer, searchTerm->text());
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    
    // Try to make changes
    BOOST_CHECK_EXCEPTION(reader->deleteDocument(4), LockObtainFailedException, check_exception(LuceneException::LockObtainFailed));
    BOOST_CHECK_EXCEPTION(reader->setNorm(5, L"aaa", 2.0), LockObtainFailedException, check_exception(LuceneException::LockObtainFailed));
    BOOST_CHECK_EXCEPTION(reader->undeleteAll(), LockObtainFailedException, check_exception(LuceneException::LockObtainFailed));

    writer->close();
    reader->close();
}

BOOST_AUTO_TEST_CASE(testWritingNorms)
{
    String indexDir(FileUtils::joinPath(getTempDir(), L"lucenetestnormwriter"));
    DirectoryPtr dir = FSDirectory::open(indexDir);
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, searchTerm->text());
    writer->close();
    
    //  now open reader & set norm for doc 0
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->setNorm(0, L"content", 2.0);

    // we should be holding the write lock now
    BOOST_CHECK(IndexWriter::isLocked(dir));

    reader->commit(MapStringString());

    // we should not be holding the write lock now
    BOOST_CHECK(!IndexWriter::isLocked(dir));

    // open a 2nd reader
    IndexReaderPtr reader2 = IndexReader::open(dir, false);

    // set norm again for doc 0
    reader->setNorm(0, L"content", 3.0);
    BOOST_CHECK(IndexWriter::isLocked(dir));

    reader->close();

    // we should not be holding the write lock now
    BOOST_CHECK(!IndexWriter::isLocked(dir));

    reader2->close();
    dir->close();

    FileUtils::removeDirectory(indexDir);
}

/// Make sure you can set norms and commit, and there are no extra norms files left
BOOST_AUTO_TEST_CASE(testWritingNormsNoReader)
{
    RAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    
    // add 1 documents with term : aaa
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    
    writer->setUseCompoundFile(false);
    addDoc(writer, searchTerm->text());
    writer->close();

    // now open reader & set norm for doc 0 (writes to _0_1.s0)
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->setNorm(0, L"content", 2.0);
    reader->close();

    // now open reader again & set norm for doc 0 (writes to _0_2.s0)
    reader = IndexReader::open(dir, false);
    reader->setNorm(0, L"content", 2.0);
    reader->close();
    BOOST_CHECK(!dir->fileExists(L"_0_1.s0"));

    dir->close();
}

static void deleteReaderWriterConflict(bool optimize)
{
    DirectoryPtr dir = getDirectory();

    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    TermPtr searchTerm2 = newLucene<Term>(L"content", L"bbb");

    // add 100 documents with term : aaa
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 100; ++i)
        addDoc(writer, searchTerm->text());
    writer->close();

    // open reader at this point - this should fix the view of the index at the point of 
    // having 100 "aaa" documents and 0 "bbb"
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    BOOST_CHECK_EQUAL(0, reader->docFreq(searchTerm2));
    checkTermDocsCount(reader, searchTerm, 100);
    checkTermDocsCount(reader, searchTerm2, 0);

    // add 100 documents with term : bbb
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 100; ++i)
        addDoc(writer, searchTerm2->text());
    
    // request optimization
    // This causes a new segment to become current for all subsequent
    // searchers. Because of this, deletions made via a previously open
    // reader, which would be applied to that reader's segment, are lost
    // for subsequent searchers/readers
    if (optimize)
        writer->optimize();
    writer->close();

    // The reader should not see the new data
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    BOOST_CHECK_EQUAL(0, reader->docFreq(searchTerm2));
    checkTermDocsCount(reader, searchTerm, 100);
    checkTermDocsCount(reader, searchTerm2, 0);
    
    // delete documents containing term: aaa
    // NOTE: the reader was created when only "aaa" documents were in
    int32_t deleted = 0;
    BOOST_CHECK_EXCEPTION(deleted = reader->deleteDocuments(searchTerm), StaleReaderException, check_exception(LuceneException::StaleReader));

    // Re-open index reader and try again. This time it should see the new data.
    reader->close();
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm2));
    checkTermDocsCount(reader, searchTerm, 100);
    checkTermDocsCount(reader, searchTerm2, 100);

    deleted = reader->deleteDocuments(searchTerm);
    BOOST_CHECK_EQUAL(100, deleted);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm2));
    checkTermDocsCount(reader, searchTerm, 0);
    checkTermDocsCount(reader, searchTerm2, 100);
    reader->close();

    // create a new reader and re-test
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm));
    BOOST_CHECK_EQUAL(100, reader->docFreq(searchTerm2));
    checkTermDocsCount(reader, searchTerm, 0);
    checkTermDocsCount(reader, searchTerm2, 100);
    reader->close();
}

BOOST_AUTO_TEST_CASE(testDeleteReaderWriterConflictUnoptimized)
{
    deleteReaderWriterConflict(false);
}

BOOST_AUTO_TEST_CASE(testDeleteReaderWriterConflictOptimized)
{
    deleteReaderWriterConflict(true);
}

BOOST_AUTO_TEST_CASE(testFilesOpenClose)
{
    // Create initial data set
    String dirFile = FileUtils::joinPath(getTempDir(), L"testIndex");
    DirectoryPtr dir = getDirectory();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, L"test");
    writer->close();
    dir->close();

    // Try to erase the data - this ensures that the writer closed all files
    FileUtils::removeDirectory(dirFile);
    dir = getDirectory();

    // Now create the data set again, just as before
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, L"test");
    writer->close();
    dir->close();

    // Now open existing directory and test that reader closes all files
    dir = getDirectory();
    IndexReaderPtr reader1 = IndexReader::open(dir, false);
    reader1->close();
    dir->close();

    // The following will fail if reader did not close all files
    BOOST_CHECK(FileUtils::removeDirectory(dirFile));
}

BOOST_AUTO_TEST_CASE(testLastModified)
{
    String fileDir = FileUtils::joinPath(getTempDir(), L"testIndex");
    for (int32_t i = 0; i < 2; ++i)
    {
        LuceneException finally;
        try
        {
            DirectoryPtr dir = i == 0 ? newLucene<MockRAMDirectory>() : getDirectory();
            BOOST_CHECK(!IndexReader::indexExists(dir));
            IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
            addDocumentWithFields(writer);
            BOOST_CHECK(IndexWriter::isLocked(dir)); // writer open, so dir is locked
            writer->close();
            BOOST_CHECK(IndexReader::indexExists(dir));
            IndexReaderPtr reader = IndexReader::open(dir, false);
            BOOST_CHECK(!IndexWriter::isLocked(dir)); // reader only, no lock
            int64_t version = IndexReader::lastModified(dir);
            if (i == 1)
            {
                int64_t version2 = IndexReader::lastModified(dir);
                BOOST_CHECK_EQUAL(version, version2);
            }
            reader->close();
            
            // modify index and check version has been incremented
            LuceneThread::threadSleep(1000);

            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
            addDocumentWithFields(writer);
            writer->close();
            reader = IndexReader::open(dir, false);
            BOOST_CHECK(version <= IndexReader::lastModified(dir));
            reader->close();
            dir->close();
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        if (i == 1)
            BOOST_CHECK(FileUtils::removeDirectory(fileDir));
        finally.throwException();
    }
}

BOOST_AUTO_TEST_CASE(testVersion)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    BOOST_CHECK(!IndexReader::indexExists(dir));
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    BOOST_CHECK(IndexWriter::isLocked(dir)); // writer open, so dir is locked
    writer->close();
    BOOST_CHECK(IndexReader::indexExists(dir));
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK(!IndexWriter::isLocked(dir)); // reader only, no lock
    int64_t version = IndexReader::getCurrentVersion(dir);
    reader->close();
    // modify index and check version has been incremented
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    reader = IndexReader::open(dir, false);
    BOOST_CHECK(version < IndexReader::getCurrentVersion(dir));
    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testLock)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    IndexReaderPtr reader = IndexReader::open(dir, false);
    BOOST_CHECK_EXCEPTION(reader->deleteDocument(0), LockObtainFailedException, check_exception(LuceneException::LockObtainFailed));
    IndexWriter::unlock(dir); // this should not be done in the real world! 
    reader->deleteDocument(0);
    reader->close();
    writer->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testUndeleteAll)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    addDocumentWithFields(writer);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(0);
    reader->deleteDocument(1);
    reader->undeleteAll();
    reader->close();
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(2, reader->numDocs()); // nothing has really been deleted thanks to undeleteAll()
    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testUndeleteAllAfterClose)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    addDocumentWithFields(writer);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(0);
    reader->deleteDocument(1);
    reader->close();
    reader = IndexReader::open(dir, false);
    reader->undeleteAll();
    BOOST_CHECK_EQUAL(2, reader->numDocs()); // nothing has really been deleted thanks to undeleteAll()
    reader->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testUndeleteAllAfterCloseThenReopen)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    addDocumentWithFields(writer);
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocument(0);
    reader->deleteDocument(1);
    reader->close();
    reader = IndexReader::open(dir, false);
    reader->undeleteAll();
    reader->close();
    reader = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(2, reader->numDocs()); // nothing has really been deleted thanks to undeleteAll()
    reader->close();
    dir->close();
}

static void deleteReaderReaderConflict(bool optimize)
{
    DirectoryPtr dir = getDirectory();

    TermPtr searchTerm1 = newLucene<Term>(L"content", L"aaa");
    TermPtr searchTerm2 = newLucene<Term>(L"content", L"bbb");
    TermPtr searchTerm3 = newLucene<Term>(L"content", L"ccc");
    
    //  add 100 documents with term : aaa
    //  add 100 documents with term : bbb
    //  add 100 documents with term : ccc
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 100; ++i)
    {
        addDoc(writer, searchTerm1->text());
        addDoc(writer, searchTerm2->text());
        addDoc(writer, searchTerm3->text());
    }
    if (optimize)
        writer->optimize();
    writer->close();

    // open two readers
    // both readers get segment info as exists at this time
    IndexReaderPtr reader1 = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm3));
    checkTermDocsCount(reader1, searchTerm1, 100);
    checkTermDocsCount(reader1, searchTerm2, 100);
    checkTermDocsCount(reader1, searchTerm3, 100);

    IndexReaderPtr reader2 = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm3));
    checkTermDocsCount(reader2, searchTerm1, 100);
    checkTermDocsCount(reader2, searchTerm2, 100);
    checkTermDocsCount(reader2, searchTerm3, 100);

    // delete docs from reader 2 and close it
    // delete documents containing term: aaa
    // when the reader is closed, the segment info is updated and
    // the first reader is now stale
    reader2->deleteDocuments(searchTerm1);
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm3));
    checkTermDocsCount(reader2, searchTerm1, 0);
    checkTermDocsCount(reader2, searchTerm2, 100);
    checkTermDocsCount(reader2, searchTerm3, 100);
    reader2->close();

    // Make sure reader 1 is unchanged since it was open earlier
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm3));
    checkTermDocsCount(reader1, searchTerm1, 100);
    checkTermDocsCount(reader1, searchTerm2, 100);
    checkTermDocsCount(reader1, searchTerm3, 100);
    
    // attempt to delete from stale reader
    // delete documents containing term: bbb
    BOOST_CHECK_EXCEPTION(reader1->deleteDocuments(searchTerm2), StaleReaderException, check_exception(LuceneException::StaleReader));

    // recreate reader and try again
    reader1->close();
    reader1 = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm3));
    checkTermDocsCount(reader1, searchTerm1, 0);
    checkTermDocsCount(reader1, searchTerm2, 100);
    checkTermDocsCount(reader1, searchTerm3, 100);

    reader1->deleteDocuments(searchTerm2);
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader1->docFreq(searchTerm3));
    checkTermDocsCount(reader1, searchTerm1, 0);
    checkTermDocsCount(reader1, searchTerm2, 0);
    checkTermDocsCount(reader1, searchTerm3, 100);
    reader1->close();

    // Open another reader to confirm that everything is deleted
    reader2 = IndexReader::open(dir, false);
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm1));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm2));
    BOOST_CHECK_EQUAL(100, reader2->docFreq(searchTerm3));
    checkTermDocsCount(reader2, searchTerm1, 0);
    checkTermDocsCount(reader2, searchTerm2, 0);
    checkTermDocsCount(reader2, searchTerm3, 100);
    reader2->close();

    dir->close();
}

BOOST_AUTO_TEST_CASE(testDeleteReaderReaderConflictUnoptimized)
{
    deleteReaderReaderConflict(false);
}

BOOST_AUTO_TEST_CASE(testDeleteReaderReaderConflictOptimized)
{
    deleteReaderReaderConflict(true);
}

/// Make sure if reader tries to commit but hits disk full that reader remains consistent and usable.
BOOST_AUTO_TEST_CASE(testDiskFull)
{
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    int32_t START_COUNT = 157;
    int32_t END_COUNT = 144;
    
    // First build up a starting index
    RAMDirectoryPtr startDir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(startDir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 157; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"content", L"aaa " + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();
    
    int64_t diskUsage = startDir->sizeInBytes();
    int64_t diskFree = diskUsage + 100;      

    LuceneException err;

    bool done = false;
    
    // Iterate with ever increasing free disk space
    while (!done)
    {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>(startDir);
        
        // If IndexReader hits disk full, it can write to the same files again.
        dir->setPreventDoubleWrite(false);

        IndexReaderPtr reader = IndexReader::open(dir, false);

        // For each disk size, first try to commit against dir that will hit random IOExceptions and
        // disk full; after, give it infinite disk space and turn off random IOExceptions and
        // retry with same reader
        bool success = false;
        
        for (int32_t x = 0; x < 2; ++x)
        {
            double rate = 0.05;
            double diskRatio = ((double)diskFree) / (double)diskUsage;
            int64_t thisDiskFree = 0;
            String testName;
            
            if (x == 0)
            {
                thisDiskFree = diskFree;
                if (diskRatio >= 2.0)
                    rate /= 2;
                if (diskRatio >= 4.0)
                    rate /= 2;
                if (diskRatio >= 6.0)
                    rate = 0.0;
                testName = L"disk full during reader.close() @ " + StringUtils::toString(thisDiskFree) + L" bytes";
            }
            else
            {
                thisDiskFree = 0;
                rate = 0.0;
                testName = L"reader re-use after disk full";
            }
            
            dir->setMaxSizeInBytes(thisDiskFree);
            dir->setRandomIOExceptionRate(rate, diskFree);
            
            try
            {
                if (x == 0)
                {
                    int32_t docId = 12;
                    for (int32_t i = 0; i < 13; ++i)
                    {
                        reader->deleteDocument(docId);
                        reader->setNorm(docId, L"contents", 2.0);
                        docId += 12;
                    }
                }
                reader->close();
                success = true;
                if (x == 0)
                    done = true;
            }
            catch (IOException& e)
            {
                err = e;
                if (x == 1)
                    BOOST_FAIL(testName << " hit IOException after disk space was freed up");
            }
            
            // Whether we succeeded or failed, check that all un-referenced files were in fact deleted (ie,
            // we did not create garbage).  Just create a new IndexFileDeleter, have it delete unreferenced 
            // files, then verify that in fact no files were deleted
            HashSet<String> _startFiles = dir->listAll();
            SegmentInfosPtr infos = newLucene<SegmentInfos>();
            infos->read(dir);
            IndexFileDeleterPtr deleter = newLucene<IndexFileDeleter>(dir, newLucene<KeepOnlyLastCommitDeletionPolicy>(), infos, InfoStreamPtr(), DocumentsWriterPtr(), HashSet<String>());
            HashSet<String> _endFiles = dir->listAll();

            Collection<String> startFiles = Collection<String>::newInstance(_startFiles.begin(), _startFiles.end());
            Collection<String> endFiles = Collection<String>::newInstance(_endFiles.begin(), _endFiles.end());

            std::sort(startFiles.begin(), startFiles.end());
            std::sort(endFiles.begin(), endFiles.end());
            
            if (!startFiles.equals(endFiles))
            {
                String successStr = success ? L"success" : L"IOException";
                BOOST_FAIL("reader.close() failed to delete unreferenced files after " << successStr << " (" << diskFree << " bytes)");
            }
            
            // Finally, verify index is not corrupt, and, if we succeeded, we see all docs changed, and if
            // we failed, we see either all docs or no docs changed (transactional semantics)
            IndexReaderPtr newReader;
            BOOST_CHECK_NO_THROW(newReader = IndexReader::open(dir, false));
            
            IndexSearcherPtr searcher = newLucene<IndexSearcher>(newReader);
            Collection<ScoreDocPtr> hits;
            BOOST_CHECK_NO_THROW(hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs);
            int32_t result2 = hits.size();
            if (success)
            {
                if (result2 != END_COUNT)
                    BOOST_FAIL(testName << ": method did not throw exception but hits.size() for search on term 'aaa' is " << result2 << " instead of expected " << END_COUNT);
            }
            else
            {
                // On hitting exception we still may have added all docs
                if (result2 != START_COUNT && result2 != END_COUNT)
                    BOOST_FAIL(testName << ": method did throw exception but hits.size() for search on term 'aaa' is " << result2 << " instead of expected " << END_COUNT);
            }
            
            searcher->close();
            newReader->close();

            if (result2 == END_COUNT)
                break;
        }
        
        dir->close();
        
        // Try again with 10 more bytes of free space
        diskFree += 10;
    }
    
    startDir->close();
}

BOOST_AUTO_TEST_CASE(testDocsOutOfOrder)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 11; ++i)
        addDoc(writer, L"aaa");
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    
    // Try to delete an invalid docId, yet, within range of the final bits of the BitVector
    BOOST_CHECK_EXCEPTION(reader->deleteDocument(11), IndexOutOfBoundsException, check_exception(LuceneException::IndexOutOfBounds));
    
    reader->close();
    
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
    
    // We must add more docs to get a new segment written
    for (int32_t i = 0; i < 11; ++i)
        addDoc(writer, L"aaa");
    
    BOOST_CHECK_NO_THROW(writer->optimize());
    writer->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testExceptionReleaseWriteLock)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer, L"aaa");
    writer->close();
    
    IndexReaderPtr reader = IndexReader::open(dir, false);
    
    BOOST_CHECK_EXCEPTION(reader->deleteDocument(1), IndexOutOfBoundsException, check_exception(LuceneException::IndexOutOfBounds));
    
    reader->close();
    
    BOOST_CHECK(!IndexWriter::isLocked(dir));
    
    reader = IndexReader::open(dir, false);
    
    BOOST_CHECK_EXCEPTION(reader->setNorm(1, L"content", 2.0), IndexOutOfBoundsException, check_exception(LuceneException::IndexOutOfBounds));
    
    reader->close();
    
    BOOST_CHECK(!IndexWriter::isLocked(dir));
    
    dir->close();
}

BOOST_AUTO_TEST_CASE(testOpenReaderAfterDelete)
{
    String indexDir(FileUtils::joinPath(getTempDir(), L"deletetest"));
    DirectoryPtr dir = FSDirectory::open(indexDir);
    BOOST_CHECK_EXCEPTION(IndexReader::open(dir, false), NoSuchDirectoryException, check_exception(LuceneException::NoSuchDirectory));
    
    FileUtils::removeDirectory(indexDir);
    
    BOOST_CHECK_EXCEPTION(IndexReader::open(dir, false), NoSuchDirectoryException, check_exception(LuceneException::NoSuchDirectory));
    
    dir->close();
}

BOOST_AUTO_TEST_CASE(testGetIndexCommit)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();
    // set up writer
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < 27; ++i)
        addDocumentWithFields(writer);
    writer->close();

    SegmentInfosPtr sis = newLucene<SegmentInfos>();
    sis->read(d);
    IndexReaderPtr r = IndexReader::open(d, false);
    IndexCommitPtr c = r->getIndexCommit();

    BOOST_CHECK_EQUAL(sis->getCurrentSegmentFileName(), c->getSegmentsFileName());

    BOOST_CHECK(c->equals(r->getIndexCommit()));

    // Change the index
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < 7; ++i)
        addDocumentWithFields(writer);
    writer->close();

    IndexReaderPtr r2 = r->reopen();
    BOOST_CHECK(!c->equals(r2->getIndexCommit()));
    BOOST_CHECK(!r2->getIndexCommit()->isOptimized());
    r2->close();

    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();

    r2 = r->reopen();
    BOOST_CHECK(r2->getIndexCommit()->isOptimized());

    r->close();
    r2->close();
    d->close();
}

BOOST_AUTO_TEST_CASE(testReadOnly)
{
    RAMDirectoryPtr d = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->commit();
    addDocumentWithFields(writer);
    writer->close();

    IndexReaderPtr r = IndexReader::open(d, true);
    BOOST_CHECK_EXCEPTION(r->deleteDocument(0), UnsupportedOperationException, check_exception(LuceneException::UnsupportedOperation));
    
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    addDocumentWithFields(writer);
    writer->close();
    
    // Make sure reopen is still readonly
    IndexReaderPtr r2 = r->reopen();
    r->close();

    BOOST_CHECK_NE(r, r2);
    BOOST_CHECK_EXCEPTION(r2->deleteDocument(0), UnsupportedOperationException, check_exception(LuceneException::UnsupportedOperation));
    
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();
    
    // Make sure reopen to a single segment is still readonly
    IndexReaderPtr r3 = r2->reopen();
    r2->close();

    BOOST_CHECK_NE(r, r2);
    BOOST_CHECK_EXCEPTION(r3->deleteDocument(0), UnsupportedOperationException, check_exception(LuceneException::UnsupportedOperation));
    
    // Make sure write lock isn't held
    writer = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    r3->close();
}

BOOST_AUTO_TEST_CASE(testIndexReader)
{
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(createDocument(L"a"));
    writer->addDocument(createDocument(L"b"));
    writer->addDocument(createDocument(L"c"));
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocuments(newLucene<Term>(L"id", L"a"));
    reader->flush();
    reader->deleteDocuments(newLucene<Term>(L"id", L"b"));
    reader->close();
    IndexReader::open(dir, true)->close();
}

BOOST_AUTO_TEST_CASE(testIndexReaderUnDeleteAll)
{
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    dir->setPreventDoubleWrite(false);
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(createDocument(L"a"));
    writer->addDocument(createDocument(L"b"));
    writer->addDocument(createDocument(L"c"));
    writer->close();
    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocuments(newLucene<Term>(L"id", L"a"));
    reader->flush();
    reader->deleteDocuments(newLucene<Term>(L"id", L"b"));
    reader->undeleteAll();
    reader->deleteDocuments(newLucene<Term>(L"id", L"b"));
    reader->close();
    IndexReader::open(dir, true)->close();
    dir->close();
}

/// Make sure on attempting to open an IndexReader on a non-existent directory, you get a good exception
BOOST_AUTO_TEST_CASE(testNoDir)
{
    String indexDir(FileUtils::joinPath(getTempDir(), L"doesnotexist"));
    DirectoryPtr dir = FSDirectory::open(indexDir);
    BOOST_CHECK_EXCEPTION(IndexReader::open(dir, true), NoSuchDirectoryException, check_exception(LuceneException::NoSuchDirectory));
    dir->close();
}

BOOST_AUTO_TEST_CASE(testNoDupCommitFileNames)
{
    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->addDocument(createDocument(L"a"));
    writer->addDocument(createDocument(L"a"));
    writer->addDocument(createDocument(L"a"));
    writer->close();
    
    Collection<IndexCommitPtr> commits = IndexReader::listCommits(dir);
    for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit)
    {
        HashSet<String> files = (*commit)->getFileNames();
        HashSet<String> seen = HashSet<String>::newInstance();
        for (HashSet<String>::iterator fileName = files.begin(); fileName != files.end(); ++fileName)
        {
            BOOST_CHECK(!seen.contains(*fileName));
            seen.add(*fileName);
        }
    }
    
    dir->close();
}

/// Ensure that on a cloned reader, segments reuse the doc values arrays in FieldCache
BOOST_AUTO_TEST_CASE(testFieldCacheReuseAfterClone)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"number", L"17", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    // Open reader
    IndexReaderPtr r = SegmentReader::getOnlySegmentReader(dir);
    Collection<int32_t> ints = FieldCache::DEFAULT()->getInts(r, L"number");
    BOOST_CHECK_EQUAL(1, ints.size());
    BOOST_CHECK_EQUAL(17, ints[0]);

    // Clone reader
    IndexReaderPtr r2 = boost::dynamic_pointer_cast<IndexReader>(r->clone());
    r->close();
    BOOST_CHECK_NE(r2, r);
    Collection<int32_t> ints2 = FieldCache::DEFAULT()->getInts(r2, L"number");
    r2->close();

    BOOST_CHECK_EQUAL(1, ints2.size());
    BOOST_CHECK_EQUAL(17, ints2[0]);
    BOOST_CHECK(ints.equals(ints2));

    dir->close();
}

/// Ensure that on a reopened reader, that any shared segments reuse the doc values arrays in FieldCache
BOOST_AUTO_TEST_CASE(testFieldCacheReuseAfterReopen)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"number", L"17", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);
    writer->commit();
    
    // Open reader1
    IndexReaderPtr r = IndexReader::open(dir, false);
    IndexReaderPtr r1 = SegmentReader::getOnlySegmentReader(r);
    Collection<int32_t> ints = FieldCache::DEFAULT()->getInts(r1, L"number");
    BOOST_CHECK_EQUAL(1, ints.size());
    BOOST_CHECK_EQUAL(17, ints[0]);

    // Add new segment
    writer->addDocument(doc);
    writer->commit();

    // Reopen reader1 --> reader2
    IndexReaderPtr r2 = r->reopen();
    r->close();
    IndexReaderPtr sub0 = r2->getSequentialSubReaders()[0];
    Collection<int32_t> ints2 = FieldCache::DEFAULT()->getInts(sub0, L"number");
    r2->close();
    BOOST_CHECK(ints.equals(ints2));

    dir->close();
}

/// Make sure all SegmentReaders are new when reopen switches readOnly
BOOST_AUTO_TEST_CASE(testReopenChangeReadonly)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"number", L"17", Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);
    writer->commit();
    
    // Open reader1
    IndexReaderPtr r = IndexReader::open(dir, false);
    BOOST_CHECK(boost::dynamic_pointer_cast<DirectoryReader>(r));
    IndexReaderPtr r1 = SegmentReader::getOnlySegmentReader(r);
    Collection<int32_t> ints = FieldCache::DEFAULT()->getInts(r1, L"number");
    BOOST_CHECK_EQUAL(1, ints.size());
    BOOST_CHECK_EQUAL(17, ints[0]);

    // Reopen to readonly with no chnages
    IndexReaderPtr r3 = r->reopen(true);
    BOOST_CHECK(boost::dynamic_pointer_cast<ReadOnlyDirectoryReader>(r3));
    r3->close();

    // Add new segment
    writer->addDocument(doc);
    writer->commit();

    // Reopen reader1 --> reader2
    IndexReaderPtr r2 = r->reopen(true);
    r->close();
    BOOST_CHECK(boost::dynamic_pointer_cast<ReadOnlyDirectoryReader>(r2));
    Collection<IndexReaderPtr> subs = r2->getSequentialSubReaders();
    Collection<int32_t> ints2 = FieldCache::DEFAULT()->getInts(subs[0], L"number");
    r2->close();

    BOOST_CHECK(boost::dynamic_pointer_cast<ReadOnlySegmentReader>(subs[0]));
    BOOST_CHECK(boost::dynamic_pointer_cast<ReadOnlySegmentReader>(subs[1]));
    BOOST_CHECK(ints.equals(ints2));

    dir->close();
}

BOOST_AUTO_TEST_CASE(testUniqueTermCount)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c d e f g h i j k l m n o p q r s t u v w x y z", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"number", L"0 1 2 3 4 5 6 7 8 9", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->addDocument(doc);
    writer->commit();
    
    IndexReaderPtr r = IndexReader::open(dir, false);
    IndexReaderPtr r1 = SegmentReader::getOnlySegmentReader(r);
    BOOST_CHECK_EQUAL(36, r1->getUniqueTermCount());
    writer->addDocument(doc);
    writer->commit();
    IndexReaderPtr r2 = r->reopen();
    r->close();
    BOOST_CHECK_EXCEPTION(r2->getUniqueTermCount(), UnsupportedOperationException, check_exception(LuceneException::UnsupportedOperation));
    Collection<IndexReaderPtr> subs = r2->getSequentialSubReaders();
    for (Collection<IndexReaderPtr>::iterator sub = subs.begin(); sub != subs.end(); ++sub)
        BOOST_CHECK_EQUAL(36, (*sub)->getUniqueTermCount());
    r2->close();
    writer->close();
    dir->close();
}

/// don't load terms index
BOOST_AUTO_TEST_CASE(testNoTermsIndex)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"a b c d e f g h i j k l m n o p q r s t u v w x y z", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"number", L"0 1 2 3 4 5 6 7 8 9", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->addDocument(doc);
    writer->close();
    
    IndexReaderPtr r = IndexReader::open(dir, IndexDeletionPolicyPtr(), true, -1);
    BOOST_CHECK_EXCEPTION(r->docFreq(newLucene<Term>(L"field", L"f")), IllegalStateException, check_exception(LuceneException::IllegalState));
    BOOST_CHECK(!boost::dynamic_pointer_cast<SegmentReader>(r->getSequentialSubReaders()[0])->termsIndexLoaded());
    BOOST_CHECK_EQUAL(-1, boost::dynamic_pointer_cast<SegmentReader>(r->getSequentialSubReaders()[0])->getTermInfosIndexDivisor());
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(doc);
    writer->close();

    // ensure re-open carries over no terms index
    IndexReaderPtr r2 = r->reopen();
    r->close();
    Collection<IndexReaderPtr> subReaders = r2->getSequentialSubReaders();
    BOOST_CHECK_EQUAL(2, subReaders.size());
    for (Collection<IndexReaderPtr>::iterator sub = subReaders.begin(); sub != subReaders.end(); ++sub)
    {
        SegmentReaderPtr subReader = boost::dynamic_pointer_cast<SegmentReader>(*sub);
        BOOST_CHECK(!subReader->termsIndexLoaded());
    }
    r2->close();
    dir->close();
}

BOOST_AUTO_TEST_CASE(testPrepareCommitIsCurrent)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    writer->addDocument(doc);
    IndexReaderPtr r = IndexReader::open(dir, true);
    BOOST_CHECK(r->isCurrent());
    writer->addDocument(doc);
    writer->prepareCommit();
    BOOST_CHECK(r->isCurrent());
    IndexReaderPtr r2 = r->reopen();
    BOOST_CHECK(r == r2);
    writer->commit();
    BOOST_CHECK(!r->isCurrent());
    writer->close();
    r->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
