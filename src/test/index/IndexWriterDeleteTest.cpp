/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "TermQuery.h"
#include "TopDocs.h"
#include "IndexReader.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "IndexFileDeleter.h"
#include "TestPoint.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterDeleteTest;

DECLARE_SHARED_PTR(FailOnlyOnDeleteFlush)

class FailOnlyOnDeleteFlush : public MockDirectoryFailure {
public:
    FailOnlyOnDeleteFlush() {
        sawMaybe = false;
        failed = false;
    }

    virtual ~FailOnlyOnDeleteFlush() {
    }

public:
    bool sawMaybe;
    bool failed;

public:
    virtual MockDirectoryFailurePtr reset() {
        sawMaybe = false;
        failed = false;
        return shared_from_this();
    }

    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (sawMaybe && !failed) {
            if (!TestPoint::getTestPoint(L"applyDeletes")) {
                // Only fail once we are no longer in applyDeletes
                failed = true;
                boost::throw_exception(IOException(L"fail after applyDeletes"));
            }
        }
        if (!failed) {
            if (TestPoint::getTestPoint(L"applyDeletes")) {
                sawMaybe = true;
            }
        }
    }
};

DECLARE_SHARED_PTR(FailOnlyOnAdd)

class FailOnlyOnAdd : public MockDirectoryFailure {
public:
    FailOnlyOnAdd() {
        failed = false;
    }

    virtual ~FailOnlyOnAdd() {
    }

public:
    bool failed;

public:
    virtual MockDirectoryFailurePtr reset() {
        failed = false;
        return shared_from_this();
    }

    virtual void eval(const MockRAMDirectoryPtr& dir) {
        if (!failed) {
            failed = true;
            boost::throw_exception(IOException(L"fail in add doc"));
        }
    }
};

static int32_t getHitCount(const DirectoryPtr& dir, const TermPtr& term) {
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    int32_t hitCount = searcher->search(newLucene<TermQuery>(term), FilterPtr(), 1000)->totalHits;
    searcher->close();
    return hitCount;
}

static void addDoc(const IndexWriterPtr& modifier, int32_t id, int32_t value) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", StringUtils::toString(id), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"value", StringUtils::toString(value), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    modifier->addDocument(doc);
}

static void updateDoc(const IndexWriterPtr& modifier, int32_t id, int32_t value) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"id", StringUtils::toString(id), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    doc->add(newLucene<Field>(L"value", StringUtils::toString(value), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    modifier->updateDocument(newLucene<Term>(L"id", StringUtils::toString(id)), doc);
}

static void checkNoUnreferencedFiles(const DirectoryPtr& dir) {
    HashSet<String> _startFiles = dir->listAll();
    SegmentInfosPtr infos = newLucene<SegmentInfos>();
    infos->read(dir);
    IndexFileDeleterPtr deleter = newLucene<IndexFileDeleter>(dir, newLucene<KeepOnlyLastCommitDeletionPolicy>(), infos, InfoStreamPtr(), DocumentsWriterPtr(), HashSet<String>());
    HashSet<String> _endFiles = dir->listAll();

    Collection<String> startFiles = Collection<String>::newInstance(_startFiles.begin(), _startFiles.end());
    Collection<String> endFiles = Collection<String>::newInstance(_endFiles.begin(), _endFiles.end());

    std::sort(startFiles.begin(), startFiles.end());
    std::sort(endFiles.begin(), endFiles.end());

    EXPECT_TRUE(startFiles.equals(endFiles));
}

TEST_F(IndexWriterDeleteTest, testSimpleCase) {
    Collection<String> keywords = newCollection<String>(L"1", L"2");
    Collection<String> unindexed = newCollection<String>(L"Netherlands", L"Italy");
    Collection<String> unstored = newCollection<String>(L"Amsterdam has lots of bridges", L"Venice has lots of canals");
    Collection<String> text = newCollection<String>(L"Amsterdam", L"Venice");

    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setUseCompoundFile(true);
    modifier->setMaxBufferedDeleteTerms(1);

    for (int32_t i = 0; i < keywords.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", keywords[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"country", unindexed[i], Field::STORE_YES, Field::INDEX_NO));
        doc->add(newLucene<Field>(L"contents", unstored[i], Field::STORE_NO, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"city", text[i], Field::STORE_YES, Field::INDEX_ANALYZED));
        modifier->addDocument(doc);
    }
    modifier->optimize();
    modifier->commit();

    TermPtr term = newLucene<Term>(L"city", L"Amsterdam");
    int32_t hitCount = getHitCount(dir, term);
    EXPECT_EQ(1, hitCount);
    modifier->deleteDocuments(term);
    modifier->commit();
    hitCount = getHitCount(dir, term);
    EXPECT_EQ(0, hitCount);

    modifier->close();
    dir->close();
}

/// test when delete terms only apply to disk segments
TEST_F(IndexWriterDeleteTest, testNonRAMDelete) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(2);
    modifier->setMaxBufferedDeleteTerms(2);

    int32_t id = 0;
    int32_t value = 100;

    for (int32_t i = 0; i < 7; ++i) {
        addDoc(modifier, ++id, value);
    }
    modifier->commit();

    EXPECT_EQ(0, modifier->getNumBufferedDocuments());
    EXPECT_TRUE(0 < modifier->getSegmentCount());

    modifier->commit();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    modifier->deleteDocuments(newLucene<Term>(L"value", StringUtils::toString(value)));

    modifier->commit();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(0, reader->numDocs());
    reader->close();
    modifier->close();
    dir->close();
}

TEST_F(IndexWriterDeleteTest, testMaxBufferedDeletes) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setMaxBufferedDeleteTerms(1);
    writer->deleteDocuments(newLucene<Term>(L"foobar", L"1"));
    writer->deleteDocuments(newLucene<Term>(L"foobar", L"1"));
    writer->deleteDocuments(newLucene<Term>(L"foobar", L"1"));
    EXPECT_EQ(3, writer->getFlushDeletesCount());
    writer->close();
    dir->close();
}

/// test when delete terms only apply to ram segments
TEST_F(IndexWriterDeleteTest, testRAMDeletes) {
    for (int32_t t = 0; t < 2; ++t) {
        DirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

        modifier->setMaxBufferedDocs(4);
        modifier->setMaxBufferedDeleteTerms(4);

        int32_t id = 0;
        int32_t value = 100;

        addDoc(modifier, ++id, value);
        if (t == 0) {
            modifier->deleteDocuments(newLucene<Term>(L"value", StringUtils::toString(value)));
        } else {
            modifier->deleteDocuments(newLucene<TermQuery>(newLucene<Term>(L"value", StringUtils::toString(value))));
        }
        addDoc(modifier, ++id, value);
        if (t == 0) {
            modifier->deleteDocuments(newLucene<Term>(L"value", StringUtils::toString(value)));
            EXPECT_EQ(2, modifier->getNumBufferedDeleteTerms());
            EXPECT_EQ(1, modifier->getBufferedDeleteTermsSize());
        } else {
            modifier->deleteDocuments(newLucene<TermQuery>(newLucene<Term>(L"value", StringUtils::toString(value))));
        }

        addDoc(modifier, ++id, value);
        EXPECT_EQ(0, modifier->getSegmentCount());
        modifier->commit();

        modifier->commit();

        IndexReaderPtr reader = IndexReader::open(dir, true);
        EXPECT_EQ(1, reader->numDocs());

        int32_t hitCount = getHitCount(dir, newLucene<Term>(L"id", StringUtils::toString(id)));
        EXPECT_EQ(1, hitCount);
        reader->close();
        modifier->close();
        dir->close();
    }
}

/// test when delete terms apply to both disk and ram segments
TEST_F(IndexWriterDeleteTest, testBothDeletes) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(100);
    modifier->setMaxBufferedDeleteTerms(100);

    int32_t id = 0;
    int32_t value = 100;
    for (int32_t i = 0; i < 5; ++i) {
        addDoc(modifier, ++id, value);
    }

    value = 200;
    for (int32_t i = 0; i < 5; ++i) {
        addDoc(modifier, ++id, value);
    }

    modifier->commit();

    for (int32_t i = 0; i < 5; ++i) {
        addDoc(modifier, ++id, value);
    }

    modifier->deleteDocuments(newLucene<Term>(L"value", StringUtils::toString(value)));

    modifier->commit();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(5, reader->numDocs());
    modifier->close();
    reader->close();
}

/// test that batched delete terms are flushed together
TEST_F(IndexWriterDeleteTest, testBatchDeletes) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(100);
    modifier->setMaxBufferedDeleteTerms(100);

    int32_t id = 0;
    int32_t value = 100;
    for (int32_t i = 0; i < 7; ++i) {
        addDoc(modifier, ++id, value);
    }
    modifier->commit();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    id = 0;
    modifier->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(++id)));
    modifier->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(++id)));

    modifier->commit();

    reader = IndexReader::open(dir, true);
    EXPECT_EQ(5, reader->numDocs());
    reader->close();

    Collection<TermPtr> terms = Collection<TermPtr>::newInstance(3);
    for (int32_t i = 0; i < terms.size(); ++i) {
        terms[i] = newLucene<Term>(L"id", StringUtils::toString(++id));
    }

    modifier->deleteDocuments(terms);
    modifier->commit();
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(2, reader->numDocs());
    reader->close();

    modifier->close();
    dir->close();
}

TEST_F(IndexWriterDeleteTest, testDeleteAll) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(2);
    modifier->setMaxBufferedDeleteTerms(2);

    int32_t id = 0;
    int32_t value = 100;
    for (int32_t i = 0; i < 7; ++i) {
        addDoc(modifier, ++id, value);
    }
    modifier->commit();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    // Add 1 doc (so we will have something buffered)
    addDoc(modifier, 99, value);

    // Delete all
    modifier->deleteAll();

    // Delete all shouldn't be on disk yet
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    // Add a doc and update a doc (after the deleteAll, before the commit)
    addDoc(modifier, 101, value);
    updateDoc(modifier, 102, value);

    // commit the delete all
    modifier->commit();

    // Validate there are no docs left
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(2, reader->numDocs());
    reader->close();

    modifier->close();
    dir->close();
}

TEST_F(IndexWriterDeleteTest, testDeleteAllRollback) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(2);
    modifier->setMaxBufferedDeleteTerms(2);

    int32_t id = 0;
    int32_t value = 100;
    for (int32_t i = 0; i < 7; ++i) {
        addDoc(modifier, ++id, value);
    }
    modifier->commit();

    addDoc(modifier, 99, value);

    IndexReaderPtr reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    // Delete all
    modifier->deleteAll();

    // Roll it back
    modifier->rollback();
    modifier->close();

    // Validate that the docs are still there
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    dir->close();
}

/// test deleteAll() with near real-time reader
TEST_F(IndexWriterDeleteTest, testDeleteAllNRT) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setMaxBufferedDocs(2);
    modifier->setMaxBufferedDeleteTerms(2);

    int32_t id = 0;
    int32_t value = 100;
    for (int32_t i = 0; i < 7; ++i) {
        addDoc(modifier, ++id, value);
    }
    modifier->commit();

    IndexReaderPtr reader = modifier->getReader();
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    addDoc(modifier, ++id, value);
    addDoc(modifier, ++id, value);

    // Delete all
    modifier->deleteAll();

    reader = modifier->getReader();
    EXPECT_EQ(0, reader->numDocs());
    reader->close();

    // Roll it back
    modifier->rollback();
    modifier->close();

    // Validate that the docs are still there
    reader = IndexReader::open(dir, true);
    EXPECT_EQ(7, reader->numDocs());
    reader->close();

    dir->close();
}

/// Make sure if modifier tries to commit but hits disk full that modifier
/// remains consistent and usable.
static void testOperationsOnDiskFull(bool updates) {
    TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
    int32_t START_COUNT = 157;
    int32_t END_COUNT = 144;

    // First build up a starting index
    RAMDirectoryPtr startDir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(startDir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);
    for (int32_t i = 0; i < 157; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"content", L"aaa " + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    int64_t diskUsage = startDir->sizeInBytes();
    int64_t diskFree = diskUsage + 10;

    LuceneException err;

    bool done = false;

    // Iterate with ever increasing free disk space
    while (!done) {
        MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>(startDir);

        // If IndexReader hits disk full, it can write to the same files again.
        dir->setPreventDoubleWrite(false);

        IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);

        modifier->setMaxBufferedDocs(1000); // use flush or close
        modifier->setMaxBufferedDeleteTerms(1000); // use flush or close

        // For each disk size, first try to commit against dir that will hit random IOExceptions and
        // disk full; after, give it infinite disk space and turn off random IOExceptions and
        // retry with same reader
        bool success = false;

        for (int32_t x = 0; x < 2; ++x) {
            double rate = 0.1;
            double diskRatio = ((double)diskFree) / (double)diskUsage;
            int64_t thisDiskFree = 0;
            String testName;

            if (x == 0) {
                thisDiskFree = diskFree;
                if (diskRatio >= 2.0) {
                    rate /= 2;
                }
                if (diskRatio >= 4.0) {
                    rate /= 2;
                }
                if (diskRatio >= 6.0) {
                    rate = 0.0;
                }
                testName = L"disk full during reader.close() @ " + StringUtils::toString(thisDiskFree) + L" bytes";
            } else {
                thisDiskFree = 0;
                rate = 0.0;
                testName = L"reader re-use after disk full";
            }

            dir->setMaxSizeInBytes(thisDiskFree);
            dir->setRandomIOExceptionRate(rate, diskFree);

            try {
                if (x == 0) {
                    int32_t docId = 12;
                    for (int32_t i = 0; i < 13; ++i) {
                        if (updates) {
                            DocumentPtr doc = newLucene<Document>();
                            doc->add(newLucene<Field>(L"id", StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
                            doc->add(newLucene<Field>(L"content", L"bbb " + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_ANALYZED));
                            modifier->updateDocument(newLucene<Term>(L"id", StringUtils::toString(docId)), doc);
                        } else { // deletes
                            modifier->deleteDocuments(newLucene<Term>(L"id", StringUtils::toString(docId)));
                        }
                        docId += 12;
                    }
                }
                modifier->close();
                success = true;
                if (x == 0) {
                    done = true;
                }
            } catch (IOException& e) {
                err = e;
                if (x == 1) {
                    FAIL() << testName << " hit IOException after disk space was freed up";
                }
            }

            // If the close() succeeded, make sure there are no unreferenced files.
            if (success) {
                checkIndex(dir);
                checkNoUnreferencedFiles(dir);
            }

            // Finally, verify index is not corrupt, and, if we succeeded, we see all docs changed, and if
            // we failed, we see either all docs or no docs changed (transactional semantics):
            IndexReaderPtr newReader;

            try {
                newReader = IndexReader::open(dir, true);
            } catch (IOException& e) {
                FAIL() << testName << ":exception when creating IndexReader after disk full during close:" << e.getError();
            }

            IndexSearcherPtr searcher = newLucene<IndexSearcher>(newReader);
            Collection<ScoreDocPtr> hits;
            EXPECT_NO_THROW(hits = searcher->search(newLucene<TermQuery>(searchTerm), FilterPtr(), 1000)->scoreDocs);
            int32_t result2 = hits.size();
            if (success) {
                if (x == 0 && result2 != END_COUNT) {
                    FAIL() << testName << ": method did not throw exception but hits.size() for search on term 'aaa' is " << result2 << " instead of expected " << END_COUNT;
                } else if (x == 1 && result2 != START_COUNT && result2 != END_COUNT) {
                    // It's possible that the first exception was "recoverable" wrt pending deletes, in which
                    // case the pending deletes are retained and then re-flushing (with plenty of disk space)
                    // will succeed in flushing the deletes
                    FAIL() << testName << ": method did not throw exception but hits.size() for search on term 'aaa' is " << result2 << " instead of expected " << START_COUNT << " or " << END_COUNT;
                }
            } else {
                // On hitting exception we still may have added all docs
                if (result2 != START_COUNT && result2 != END_COUNT) {
                    FAIL() << testName << ": method did throw exception but hits.size() for search on term 'aaa' is " << result2 << " instead of expected " << START_COUNT << " or " << END_COUNT;
                }
            }

            searcher->close();
            newReader->close();

            if (result2 == END_COUNT) {
                break;
            }
        }

        dir->close();

        // Try again with 10 more bytes of free space
        diskFree += 10;
    }

    startDir->close();
}

TEST_F(IndexWriterDeleteTest, testDeletesOnDiskFull) {
    testOperationsOnDiskFull(false);
}

TEST_F(IndexWriterDeleteTest, testUpdatesOnDiskFull) {
    testOperationsOnDiskFull(true);
}

/// This test tests that buffered deletes are cleared when an Exception is hit during flush.
TEST_F(IndexWriterDeleteTest, testErrorAfterApplyDeletes) {
    Collection<String> keywords = newCollection<String>(L"1", L"2");
    Collection<String> unindexed = newCollection<String>(L"Netherlands", L"Italy");
    Collection<String> unstored = newCollection<String>(L"Amsterdam has lots of bridges", L"Venice has lots of canals");
    Collection<String> text = newCollection<String>(L"Amsterdam", L"Venice");

    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    modifier->setUseCompoundFile(true);
    modifier->setMaxBufferedDeleteTerms(2);

    FailOnlyOnDeleteFlushPtr failure = newLucene<FailOnlyOnDeleteFlush>();
    dir->failOn(failure->reset());

    for (int32_t i = 0; i < keywords.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", keywords[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"country", unindexed[i], Field::STORE_YES, Field::INDEX_NO));
        doc->add(newLucene<Field>(L"contents", unstored[i], Field::STORE_NO, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"city", text[i], Field::STORE_YES, Field::INDEX_ANALYZED));
        modifier->addDocument(doc);
    }

    modifier->optimize();
    modifier->commit();

    TermPtr term = newLucene<Term>(L"city", L"Amsterdam");
    int32_t hitCount = getHitCount(dir, term);
    EXPECT_EQ(1, hitCount);

    modifier->deleteDocuments(term);

    DocumentPtr doc = newLucene<Document>();
    modifier->addDocument(doc);

    // The failure object will fail on the first write after the del file gets created when
    // processing the buffered delete

    // In the ac case, this will be when writing the new segments files so we really don't
    // need the new doc, but it's harmless

    // In the !ac case, a new segments file won't be created but in this case, creation of
    // the cfs file happens next so we need the doc (to test that it's okay that we don't
    // lose deletes if failing while creating the cfs file)

    try {
        modifier->commit();
    } catch (IOException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IO)(e));
    }

    // The commit above failed, so we need to retry it (which will succeed, because the
    // failure is a one-shot)

    modifier->commit();
    hitCount = getHitCount(dir, term);

    // Make sure the delete was successfully flushed
    EXPECT_EQ(0, hitCount);

    modifier->close();
    dir->close();
}

/// This test tests that the files created by the docs writer before a segment is written are
/// cleaned up if there's an i/o error
TEST_F(IndexWriterDeleteTest, testErrorInDocsWriterAdd) {
    Collection<String> keywords = newCollection<String>(L"1", L"2");
    Collection<String> unindexed = newCollection<String>(L"Netherlands", L"Italy");
    Collection<String> unstored = newCollection<String>(L"Amsterdam has lots of bridges", L"Venice has lots of canals");
    Collection<String> text = newCollection<String>(L"Amsterdam", L"Venice");

    MockRAMDirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    FailOnlyOnAddPtr failure = newLucene<FailOnlyOnAdd>();
    dir->failOn(failure->reset());

    for (int32_t i = 0; i < keywords.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", keywords[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"country", unindexed[i], Field::STORE_YES, Field::INDEX_NO));
        doc->add(newLucene<Field>(L"contents", unstored[i], Field::STORE_NO, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"city", text[i], Field::STORE_YES, Field::INDEX_ANALYZED));

        try {
            modifier->addDocument(doc);
        } catch (IOException&) {
            break;
        }
    }

    checkNoUnreferencedFiles(dir);

    modifier->close();
    dir->close();
}
