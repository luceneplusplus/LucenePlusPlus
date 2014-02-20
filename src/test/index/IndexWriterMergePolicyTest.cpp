/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "RAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "LogDocMergePolicy.h"
#include "Document.h"
#include "Field.h"
#include "ConcurrentMergeScheduler.h"
#include "IndexReader.h"
#include "Term.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterMergePolicyTest;

static void addDoc(const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

static void checkInvariants(const IndexWriterPtr& writer) {
    writer->waitForMerges();
    int32_t maxBufferedDocs = writer->getMaxBufferedDocs();
    int32_t mergeFactor = writer->getMergeFactor();
    int32_t maxMergeDocs = writer->getMaxMergeDocs();

    int32_t ramSegmentCount = writer->getNumBufferedDocuments();
    EXPECT_TRUE(ramSegmentCount < maxBufferedDocs);

    int32_t lowerBound = -1;
    int32_t upperBound = maxBufferedDocs;
    int32_t numSegments = 0;

    int32_t segmentCount = writer->getSegmentCount();
    for (int32_t i = segmentCount - 1; i >= 0; --i) {
        int32_t docCount = writer->getDocCount(i);
        EXPECT_TRUE(docCount > lowerBound);

        if (docCount <= upperBound) {
            ++numSegments;
        } else {
            if (upperBound * mergeFactor <= maxMergeDocs) {
                EXPECT_TRUE(numSegments < mergeFactor);
            }

            do {
                lowerBound = upperBound;
                upperBound *= mergeFactor;
            } while (docCount > upperBound);
            numSegments = 1;
        }
    }
    if (upperBound * mergeFactor <= maxMergeDocs) {
        EXPECT_TRUE(numSegments < mergeFactor);
    }
}

TEST_F(IndexWriterMergePolicyTest, testNormalCase) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(10);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
        checkInvariants(writer);
    }

    writer->close();
}

TEST_F(IndexWriterMergePolicyTest, testNoOverMerge) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(10);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

    bool noOverMerge = false;
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
        checkInvariants(writer);
        if (writer->getNumBufferedDocuments() + writer->getSegmentCount() >= 18) {
            noOverMerge = true;
        }
    }

    EXPECT_TRUE(noOverMerge);

    writer->close();
}

/// Test the case where flush is forced after every addDoc
TEST_F(IndexWriterMergePolicyTest, testForceFlush) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(10);
    LogDocMergePolicyPtr mp = newLucene<LogDocMergePolicy>(writer);
    mp->setMinMergeDocs(100);
    writer->setMergePolicy(mp);

    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
        writer->close();

        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(10);
        writer->setMergePolicy(mp);
        mp->setMinMergeDocs(100);
        writer->setMergeFactor(10);
        checkInvariants(writer);
    }

    writer->close();
}

/// Test the case where mergeFactor changes
TEST_F(IndexWriterMergePolicyTest, testMergeFactorChange) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(100);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

    for (int32_t i = 0; i < 250; ++i) {
        addDoc(writer);
        checkInvariants(writer);
    }

    writer->setMergeFactor(5);

    // merge policy only fixes segments on levels where merges have been triggered, so check invariants after all adds
    for (int32_t i = 0; i < 10; ++i) {
        addDoc(writer);
    }
    checkInvariants(writer);

    writer->close();
}

/// Test the case where both mergeFactor and maxBufferedDocs change
TEST_F(IndexWriterMergePolicyTest, testMaxBufferedDocsChange) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(101);
    writer->setMergeFactor(101);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));

    // leftmost* segment has 1 doc
    // rightmost* segment has 100 docs
    for (int32_t i = 1; i <= 100; ++i) {
        for (int32_t j = 0; j < i; ++j) {
            addDoc(writer);
            checkInvariants(writer);
        }
        writer->close();

        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(101);
        writer->setMergeFactor(101);
        writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));
    }

    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(10);

    // merge policy only fixes segments on levels where merges have been triggered, so check invariants after all adds
    for (int32_t i = 0; i < 100; ++i) {
        addDoc(writer);
    }
    checkInvariants(writer);

    for (int32_t i = 100; i < 1000; ++i) {
        addDoc(writer);
    }
    writer->commit();
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();
    writer->commit();
    checkInvariants(writer);

    writer->close();
}

/// Test the case where a merge results in no doc at all
TEST_F(IndexWriterMergePolicyTest, testMergeDocCount0) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(100);

    for (int32_t i = 0; i < 250; ++i) {
        addDoc(writer);
        checkInvariants(writer);
    }
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, false);
    reader->deleteDocuments(newLucene<Term>(L"content", L"aaa"));
    reader->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMergePolicy(newLucene<LogDocMergePolicy>(writer));
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(5);

    // merge factor is changed, so check invariants after all adds
    for (int32_t i = 0; i < 10; ++i) {
        addDoc(writer);
    }
    writer->commit();
    boost::dynamic_pointer_cast<ConcurrentMergeScheduler>(writer->getMergeScheduler())->sync();
    writer->commit();
    checkInvariants(writer);
    EXPECT_EQ(10, writer->maxDoc());

    writer->close();
}
