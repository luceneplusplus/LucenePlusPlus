/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexReader.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "SimpleAnalyzer.h"
#include "LogDocMergePolicy.h"
#include "Document.h"
#include "Field.h"
#include "ReadOnlySegmentReader.h"
#include "ReadOnlyDirectoryReader.h"
#include "ParallelReader.h"
#include "SegmentReader.h"
#include "_SegmentReader.h"
#include "Similarity.h"
#include "Term.h"
#include "MultiReader.h"
#include "MiscUtils.h"

using namespace Lucene;

/// Tests cloning multiple types of readers, modifying the deletedDocs and norms and verifies copy on write semantics
/// of the deletedDocs and norms is implemented properly
BOOST_FIXTURE_TEST_SUITE(IndexReaderCloneTest, LuceneTestFixture)

static DocumentPtr createDocument(int32_t n, int32_t numFields)
{
    StringStream sb;
    DocumentPtr doc = newLucene<Document>();
    sb << L"a" << n;
    doc->add(newLucene<Field>(L"field1", sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"fielda", sb.str(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED_NO_NORMS));
    doc->add(newLucene<Field>(L"fieldb", sb.str(), Field::STORE_YES, Field::INDEX_NO));
    sb << L" b" << n;
    for (int32_t i = 1; i < numFields; ++i)
        doc->add(newLucene<Field>(L"field" + StringUtils::toString(i + 1), sb.str(), Field::STORE_YES, Field::INDEX_ANALYZED));
    return doc;
}

static void createIndex(DirectoryPtr dir, bool multiSegment)
{
    IndexWriter::unlock(dir);
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);

    w->setMergePolicy(newLucene<LogDocMergePolicy>(w));

    for (int32_t i = 0; i < 100; ++i)
    {
        w->addDocument(createDocument(i, 4));
        if (multiSegment && (i % 10) == 0)
            w->commit();
    }

    if (!multiSegment)
        w->optimize();

    w->close();

    IndexReaderPtr r = IndexReader::open(dir, false);
    if (multiSegment)
        BOOST_CHECK(r->getSequentialSubReaders().size() > 1);
    else
        BOOST_CHECK_EQUAL(r->getSequentialSubReaders().size(), 1);
    r->close();
}

static bool isReadOnly(IndexReaderPtr r)
{
    return (MiscUtils::typeOf<ReadOnlySegmentReader>(r) || MiscUtils::typeOf<ReadOnlyDirectoryReader>(r));
}

static bool deleteWorked(int32_t doc, IndexReaderPtr r)
{
    bool exception = false;
    try
    {
        // trying to delete from the original reader should throw an exception
        r->deleteDocument(doc);
    }
    catch (...)
    {
        exception = true;
    }
    return !exception;
}

/// 1. Get a norm from the original reader
/// 2. Clone the original reader
/// 3. Delete a document and set the norm of the cloned reader
/// 4. Verify the norms are not the same on each reader
/// 5. Verify the doc deleted is only in the cloned reader
/// 6. Try to delete a document in the original reader, an exception should be thrown
static void performDefaultTests(IndexReaderPtr r1)
{
    double norm1 = Similarity::decodeNorm(r1->norms(L"field1")[4]);

    IndexReaderPtr pr1Clone = LuceneDynamicCast<IndexReader>(r1->clone());
    pr1Clone->deleteDocument(10);
    pr1Clone->setNorm(4, L"field1", 0.5);
    BOOST_CHECK(Similarity::decodeNorm(r1->norms(L"field1")[4]) == norm1);
    BOOST_CHECK_NE(Similarity::decodeNorm(pr1Clone->norms(L"field1")[4]), norm1);

    BOOST_CHECK(!r1->isDeleted(10));
    BOOST_CHECK(pr1Clone->isDeleted(10));

    // try to update the original reader, which should throw an exception
    BOOST_CHECK_EXCEPTION(r1->deleteDocument(11), LuceneException, check_exception(LuceneException::Null));
    pr1Clone->close();
}

static void modifyIndex(int32_t i, DirectoryPtr dir)
{
    switch (i)
    {
        case 0:
        {
            IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
            w->deleteDocuments(newLucene<Term>(L"field2", L"a11"));
            w->deleteDocuments(newLucene<Term>(L"field2", L"b30"));
            w->close();
            break;
        }
        case 1:
        {
            IndexReaderPtr reader = IndexReader::open(dir, false);
            reader->setNorm(4, L"field1", (uint8_t)123);
            reader->setNorm(44, L"field2", (uint8_t)222);
            reader->setNorm(44, L"field4", (uint8_t)22);
            reader->close();
            break;
        }
        case 2:
        {
            IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
            w->optimize();
            w->close();
            break;
        }
        case 3:
        {
            IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
            w->addDocument(createDocument(101, 4));
            w->optimize();
            w->addDocument(createDocument(102, 4));
            w->addDocument(createDocument(103, 4));
            w->close();
            break;
        }
        case 4:
        {
            IndexReaderPtr reader = IndexReader::open(dir, false);
            reader->setNorm(5, L"field1", (uint8_t)123);
            reader->setNorm(55, L"field2", (uint8_t)222);
            reader->close();
            break;
        }
        case 5:
        {
            IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
            w->addDocument(createDocument(101, 4));
            w->close();
            break;
        }
    }
}

static void checkDelDocsRefCountEquals(int32_t refCount, SegmentReaderPtr reader)
{
    BOOST_CHECK_EQUAL(refCount, reader->deletedDocsRef->refCount());
}

static void checkDocDeleted(SegmentReaderPtr reader, SegmentReaderPtr reader2, int32_t doc)
{
    BOOST_CHECK_EQUAL(reader->isDeleted(doc), reader2->isDeleted(doc));
}

BOOST_AUTO_TEST_CASE(testCloneReadOnlySegmentReader)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr reader = IndexReader::open(dir1, false);
    IndexReaderPtr readOnlyReader = LuceneDynamicCast<IndexReader>(reader->clone(true));
    BOOST_CHECK(isReadOnly(readOnlyReader));
    BOOST_CHECK(!deleteWorked(1, readOnlyReader));
    reader->close();
    readOnlyReader->close();
    dir1->close();
}

/// Open non-readOnly reader1, clone to non-readOnly reader2, make sure we can change reader2
BOOST_AUTO_TEST_CASE(testCloneNoChangesStillReadOnly)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    IndexReaderPtr r2 = LuceneDynamicCast<IndexReader>(r1->clone(false));
    BOOST_CHECK(deleteWorked(1, r2));
    r1->close();
    r2->close();
    dir1->close();
}

/// Open non-readOnly reader1, clone to non-readOnly reader2, make sure we can change reader1
BOOST_AUTO_TEST_CASE(testCloneWriteToOrig)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    IndexReaderPtr r2 = LuceneDynamicCast<IndexReader>(r1->clone(false));
    BOOST_CHECK(deleteWorked(1, r1));
    r1->close();
    r2->close();
    dir1->close();
}

/// Open non-readOnly reader1, clone to non-readOnly reader2, make sure we can change reader2
BOOST_AUTO_TEST_CASE(testCloneWriteToClone)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    IndexReaderPtr r2 = LuceneDynamicCast<IndexReader>(r1->clone(false));
    BOOST_CHECK(deleteWorked(1, r2));
    // should fail because reader1 holds the write lock
    BOOST_CHECK(!deleteWorked(1, r1));
    r2->close();
    // should fail because we are now stale (reader1 committed changes)
    BOOST_CHECK(!deleteWorked(1, r1));
    r1->close();

    dir1->close();
}

/// Create single-segment index, open non-readOnly SegmentReader, add docs, reopen to multireader, then do delete
BOOST_AUTO_TEST_CASE(testReopenSegmentReaderToMultiReader)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr reader1 = IndexReader::open(dir1, false);

    modifyIndex(5, dir1);

    IndexReaderPtr reader2 = reader1->reopen();
    BOOST_CHECK_NE(reader1, reader2);

    BOOST_CHECK(deleteWorked(1, reader2));
    reader1->close();
    reader2->close();
    dir1->close();
}

/// Open non-readOnly reader1, clone to readOnly reader2
BOOST_AUTO_TEST_CASE(testCloneWriteableToReadOnly)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr reader = IndexReader::open(dir1, false);
    IndexReaderPtr readOnlyReader = LuceneDynamicCast<IndexReader>(reader->clone(true));

    BOOST_CHECK(isReadOnly(readOnlyReader));
    BOOST_CHECK(!deleteWorked(1, readOnlyReader));
    BOOST_CHECK(!readOnlyReader->hasChanges());

    reader->close();
    readOnlyReader->close();
    dir1->close();
}

/// Open non-readOnly reader1, reopen to readOnly reader2
BOOST_AUTO_TEST_CASE(testReopenWriteableToReadOnly)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr reader = IndexReader::open(dir1, false);
    int32_t docCount = reader->numDocs();
    BOOST_CHECK(deleteWorked(1, reader));
    BOOST_CHECK_EQUAL(docCount - 1, reader->numDocs());

    IndexReaderPtr readOnlyReader = reader->reopen(true);
    BOOST_CHECK(isReadOnly(readOnlyReader));
    BOOST_CHECK(!deleteWorked(1, readOnlyReader));
    BOOST_CHECK_EQUAL(docCount - 1, readOnlyReader->numDocs());
    reader->close();
    readOnlyReader->close();
    dir1->close();
}

/// Open readOnly reader1, clone to non-readOnly reader2
BOOST_AUTO_TEST_CASE(testCloneReadOnlyToWriteable)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr reader1 = IndexReader::open(dir1, true);
    IndexReaderPtr reader2 = LuceneDynamicCast<IndexReader>(reader1->clone(false));

    BOOST_CHECK(!isReadOnly(reader2));
    BOOST_CHECK(!deleteWorked(1, reader1));
    // this readonly reader shouldn't yet have a write lock
    BOOST_CHECK(!reader2->hasChanges());
    BOOST_CHECK(deleteWorked(1, reader2));
    reader1->close();
    reader2->close();
    dir1->close();
}

/// Open non-readOnly reader1 on multi-segment index, then optimize the index, then clone to readOnly reader2
BOOST_AUTO_TEST_CASE(testReadOnlyCloneAfterOptimize)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr reader1 = IndexReader::open(dir1, false);
    IndexWriterPtr w = newLucene<IndexWriter>(dir1, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    w->optimize();
    w->close();
    IndexReaderPtr reader2 = LuceneDynamicCast<IndexReader>(reader1->clone(true));
    BOOST_CHECK(isReadOnly(reader2));
    reader1->close();
    reader2->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testCloneReadOnlyDirectoryReader)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    IndexReaderPtr reader = IndexReader::open(dir1, false);
    IndexReaderPtr readOnlyReader = LuceneDynamicCast<IndexReader>(reader->clone(true));
    BOOST_CHECK(isReadOnly(readOnlyReader));
    reader->close();
    readOnlyReader->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testParallelReader)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    createIndex(dir2, true);

    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    IndexReaderPtr r2 = IndexReader::open(dir2, false);

    ParallelReaderPtr pr1 = newLucene<ParallelReader>();
    pr1->add(r1);
    pr1->add(r2);

    performDefaultTests(pr1);
    pr1->close();
    dir1->close();
    dir2->close();
}

BOOST_AUTO_TEST_CASE(testMixedReaders)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);
    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    createIndex(dir2, true);

    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    IndexReaderPtr r2 = IndexReader::open(dir2, false);

    Collection<IndexReaderPtr> multiReaders = newCollection<IndexReaderPtr>(r1, r2);
    MultiReaderPtr multiReader = newLucene<MultiReader>(multiReaders);
    performDefaultTests(multiReader);
    multiReader->close();
    dir1->close();
    dir2->close();
}

BOOST_AUTO_TEST_CASE(testSegmentReaderUndeleteall)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    SegmentReaderPtr origSegmentReader = SegmentReader::getOnlySegmentReader(dir1);
    origSegmentReader->deleteDocument(10);
    checkDelDocsRefCountEquals(1, origSegmentReader);
    origSegmentReader->undeleteAll();
    BOOST_CHECK(!origSegmentReader->deletedDocsRef);
    origSegmentReader->close();
    // need to test norms?
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testSegmentReaderCloseReferencing)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    SegmentReaderPtr origSegmentReader = SegmentReader::getOnlySegmentReader(dir1);
    origSegmentReader->deleteDocument(1);
    origSegmentReader->setNorm(4, L"field1", 0.5);

    SegmentReaderPtr clonedSegmentReader = LuceneDynamicCast<SegmentReader>(origSegmentReader->clone());
    checkDelDocsRefCountEquals(2, origSegmentReader);
    origSegmentReader->close();
    checkDelDocsRefCountEquals(1, origSegmentReader);
    // check the norm refs
    NormPtr norm = clonedSegmentReader->_norms.get(L"field1");
    BOOST_CHECK_EQUAL(1, norm->bytesRef()->refCount());
    clonedSegmentReader->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testSegmentReaderDelDocsReferenceCounting)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr origReader = IndexReader::open(dir1, false);
    SegmentReaderPtr origSegmentReader = SegmentReader::getOnlySegmentReader(origReader);
    // deletedDocsRef should be null because nothing has updated yet
    BOOST_CHECK(!origSegmentReader->deletedDocsRef);

    // we deleted a document, so there is now a deletedDocs bitvector and a reference to it
    origReader->deleteDocument(1);
    checkDelDocsRefCountEquals(1, origSegmentReader);

    // the cloned segmentreader should have 2 references, 1 to itself, and 1 to the original segmentreader
    IndexReaderPtr clonedReader = LuceneDynamicCast<IndexReader>(origReader->clone());
    SegmentReaderPtr clonedSegmentReader = SegmentReader::getOnlySegmentReader(clonedReader);
    checkDelDocsRefCountEquals(2, origSegmentReader);
    // deleting a document creates a new deletedDocs bitvector, the refs goes to 1
    clonedReader->deleteDocument(2);
    checkDelDocsRefCountEquals(1, origSegmentReader);
    checkDelDocsRefCountEquals(1, clonedSegmentReader);

    // make sure the deletedocs objects are different (copy on write)
    BOOST_CHECK_NE(origSegmentReader->deletedDocs, clonedSegmentReader->deletedDocs);

    checkDocDeleted(origSegmentReader, clonedSegmentReader, 1);
    BOOST_CHECK(!origSegmentReader->isDeleted(2)); // doc 2 should not be deleted in original segmentreader
    BOOST_CHECK(clonedSegmentReader->isDeleted(2)); // doc 2 should be deleted in cloned segmentreader

    BOOST_CHECK_EXCEPTION(origReader->deleteDocument(4), LockObtainFailedException, check_exception(LuceneException::LockObtainFailed));

    origReader->close();
    // try closing the original segment reader to see if it affects the clonedSegmentReader
    clonedReader->deleteDocument(3);
    clonedReader->flush();
    checkDelDocsRefCountEquals(1, clonedSegmentReader);

    // test a reopened reader
    IndexReaderPtr reopenedReader = clonedReader->reopen();
    IndexReaderPtr cloneReader2 = LuceneDynamicCast<IndexReader>(reopenedReader->clone());
    SegmentReaderPtr cloneSegmentReader2 = SegmentReader::getOnlySegmentReader(cloneReader2);
    checkDelDocsRefCountEquals(2, cloneSegmentReader2);
    clonedReader->close();
    reopenedReader->close();
    cloneReader2->close();

    dir1->close();
}

BOOST_AUTO_TEST_CASE(testCloneWithDeletes)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr origReader = IndexReader::open(dir1, false);
    origReader->deleteDocument(1);

    IndexReaderPtr clonedReader = LuceneDynamicCast<IndexReader>(origReader->clone());
    origReader->close();
    clonedReader->close();

    IndexReaderPtr r = IndexReader::open(dir1, false);
    BOOST_CHECK(r->isDeleted(1));
    r->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testCloneWithSetNorm)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr orig = IndexReader::open(dir1, false);
    orig->setNorm(1, L"field1", 17.0);
    uint8_t encoded = Similarity::encodeNorm(17.0);
    BOOST_CHECK_EQUAL(encoded, orig->norms(L"field1")[1]);

    // the cloned segmentreader should have 2 references, 1 to itself, and 1 to the original segmentreader
    IndexReaderPtr clonedReader = LuceneDynamicCast<IndexReader>(orig->clone());
    orig->close();
    clonedReader->close();

    IndexReaderPtr r = IndexReader::open(dir1, false);
    BOOST_CHECK_EQUAL(encoded, r->norms(L"field1")[1]);
    r->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testCloneSubreaders)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, true);

    IndexReaderPtr reader = IndexReader::open(dir1, false);
    reader->deleteDocument(1); // acquire write lock
    Collection<IndexReaderPtr> subs = reader->getSequentialSubReaders();
    BOOST_CHECK(subs.size() > 1);

    Collection<IndexReaderPtr> clones = Collection<IndexReaderPtr>::newInstance(subs.size());
    for (int32_t x = 0; x < subs.size(); ++x)
        clones[x] = LuceneDynamicCast<IndexReader>(subs[x]->clone());
    reader->close();
    for (int32_t x = 0; x < subs.size(); ++x)
        clones[x]->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testIncDecRef)
{
    DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
    createIndex(dir1, false);
    IndexReaderPtr r1 = IndexReader::open(dir1, false);
    r1->incRef();
    IndexReaderPtr r2 = LuceneDynamicCast<IndexReader>(r1->clone(false));
    r1->deleteDocument(5);
    r1->decRef();

    r1->incRef();

    r2->close();
    r1->decRef();
    r1->close();
    dir1->close();
}

BOOST_AUTO_TEST_CASE(testCloseStoredFields)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<SimpleAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    w->setUseCompoundFile(false);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"yes it's stored", Field::STORE_YES, Field::INDEX_ANALYZED));
    w->addDocument(doc);
    w->close();
    IndexReaderPtr r1 = IndexReader::open(dir, false);
    IndexReaderPtr r2 = LuceneDynamicCast<IndexReader>(r1->clone(false));
    r1->close();
    r2->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
