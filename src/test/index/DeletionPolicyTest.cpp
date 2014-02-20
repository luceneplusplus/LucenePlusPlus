/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexCommit.h"
#include "SegmentInfos.h"
#include "IndexDeletionPolicy.h"
#include "MockRAMDirectory.h"
#include "IndexReader.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "IndexFileNames.h"
#include "Document.h"
#include "Field.h"
#include "SerialMergeScheduler.h"
#include "KeepOnlyLastCommitDeletionPolicy.h"
#include "TermQuery.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture DeletionPolicyTest;

static void verifyCommitOrder(Collection<IndexCommitPtr> commits) {
    IndexCommitPtr firstCommit = commits[0];
    int64_t last = SegmentInfos::generationFromSegmentsFileName(firstCommit->getSegmentsFileName());
    EXPECT_EQ(last, firstCommit->getGeneration());
    int64_t lastVersion = firstCommit->getVersion();
    int64_t lastTimestamp = firstCommit->getTimestamp();
    for (int32_t i = 1; i < commits.size(); ++i) {
        IndexCommitPtr commit = commits[i];
        int64_t now = SegmentInfos::generationFromSegmentsFileName(commit->getSegmentsFileName());
        int64_t nowVersion = commit->getVersion();
        int64_t nowTimestamp = commit->getTimestamp();
        EXPECT_TRUE(now > last); // SegmentInfos commits are out-of-order?
        EXPECT_TRUE(nowVersion > lastVersion); // SegmentInfos versions are out-of-order?
        EXPECT_TRUE(nowTimestamp >= lastTimestamp); // SegmentInfos timestamps are out-of-order?
        EXPECT_EQ(now, commit->getGeneration());
        last = now;
        lastVersion = nowVersion;
        lastTimestamp = nowTimestamp;
    }
}

static void addDoc(const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", L"aaa", Field::STORE_NO, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

DECLARE_SHARED_PTR(DeletionPolicyKeepAllDeletionPolicy)
DECLARE_SHARED_PTR(KeepNoneOnInitDeletionPolicy)
DECLARE_SHARED_PTR(KeepLastNDeletionPolicy)
DECLARE_SHARED_PTR(ExpirationTimeDeletionPolicy)

class DeletionPolicyKeepAllDeletionPolicy : public IndexDeletionPolicy {
public:
    DeletionPolicyKeepAllDeletionPolicy() {
        numOnInit = 0;
        numOnCommit = 0;
    }

    virtual ~DeletionPolicyKeepAllDeletionPolicy() {
    }

    LUCENE_CLASS(DeletionPolicyKeepAllDeletionPolicy);

public:
    int32_t numOnInit;
    int32_t numOnCommit;
    DirectoryPtr dir;

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        ++numOnInit;
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
        IndexCommitPtr lastCommit = commits[commits.size() - 1];
        IndexReaderPtr r = IndexReader::open(dir, true);
        EXPECT_EQ(r->isOptimized(), lastCommit->isOptimized());
        r->close();
        verifyCommitOrder(commits);
        ++numOnCommit;
    }
};

/// This is useful for adding to a big index when you know readers are not using it.
class KeepNoneOnInitDeletionPolicy : public IndexDeletionPolicy {
public:
    KeepNoneOnInitDeletionPolicy() {
        numOnInit = 0;
        numOnCommit = 0;
    }

    virtual ~KeepNoneOnInitDeletionPolicy() {
    }

    LUCENE_CLASS(KeepNoneOnInitDeletionPolicy);

public:
    int32_t numOnInit;
    int32_t numOnCommit;

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        ++numOnInit;
        // On init, delete all commit points
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
            (*commit)->deleteCommit();
            EXPECT_TRUE((*commit)->isDeleted());
        }
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        int32_t size = commits.size();
        // Delete all but last one
        for (int32_t i = 0; i < size - 1; ++i) {
            commits[i]->deleteCommit();
        }
        ++numOnCommit;
    }
};

class KeepLastNDeletionPolicy : public IndexDeletionPolicy {
public:
    KeepLastNDeletionPolicy(int32_t numToKeep) {
        this->numOnInit = 0;
        this->numOnCommit = 0;
        this->numToKeep = numToKeep;
        this->numDelete = 0;
        this->seen = HashSet<String>::newInstance();
    }

    virtual ~KeepLastNDeletionPolicy() {
    }

    LUCENE_CLASS(KeepLastNDeletionPolicy);

public:
    int32_t numOnInit;
    int32_t numOnCommit;
    int32_t numToKeep;
    int32_t numDelete;
    HashSet<String> seen;

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        ++numOnInit;
        // do no deletions on init
        doDeletes(commits, false);
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        doDeletes(commits, true);
    }

protected:
    void doDeletes(Collection<IndexCommitPtr> commits, bool isCommit) {
        // Assert that we really are only called for each new commit
        if (isCommit) {
            String fileName = commits[commits.size() - 1]->getSegmentsFileName();
            if (seen.contains(fileName)) {
                FAIL() << "onCommit was called twice on the same commit point";
            }
            seen.add(fileName);
            ++numOnCommit;
        }
        int32_t size = commits.size();
        for (int32_t i = 0; i < size - numToKeep; ++i) {
            commits[i]->deleteCommit();
            ++numDelete;
        }
    }
};

/// Delete a commit only when it has been obsoleted by N seconds
class ExpirationTimeDeletionPolicy : public IndexDeletionPolicy {
public:
    ExpirationTimeDeletionPolicy(const DirectoryPtr& dir, double seconds) {
        this->dir = dir;
        this->expirationTimeSeconds = seconds;
        this->numDelete = 0;
    }

    virtual ~ExpirationTimeDeletionPolicy() {
    }

    LUCENE_CLASS(ExpirationTimeDeletionPolicy);

public:
    DirectoryPtr dir;
    double expirationTimeSeconds;
    int32_t numDelete;

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);
        onCommit(commits);
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
        verifyCommitOrder(commits);

        IndexCommitPtr lastCommit = commits[commits.size() - 1];

        // Any commit older than expireTime should be deleted
        double expireTime = dir->fileModified(lastCommit->getSegmentsFileName()) / 1000.0 - expirationTimeSeconds;

        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
            double modTime = dir->fileModified((*commit)->getSegmentsFileName()) / 1000.0;
            if (*commit != lastCommit && modTime < expireTime) {
                (*commit)->deleteCommit();
                ++numDelete;
            }
        }
    }
};

/// Test "by time expiration" deletion policy
TEST_F(DeletionPolicyTest, testExpirationTimeDeletionPolicy) {
    const double SECONDS = 2.0;

    bool useCompoundFile = true;

    DirectoryPtr dir = newLucene<RAMDirectory>();
    ExpirationTimeDeletionPolicyPtr policy = newLucene<ExpirationTimeDeletionPolicy>(dir, SECONDS);
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setUseCompoundFile(useCompoundFile);
    writer->close();

    int64_t lastDeleteTime = 0;
    for (int32_t i = 0; i < 7; ++i) {
        // Record last time when writer performed deletes of past commits
        lastDeleteTime = MiscUtils::currentTimeMillis();
        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setUseCompoundFile(useCompoundFile);
        for (int32_t j = 0; j < 17; ++j) {
            addDoc(writer);
        }
        writer->close();

        // Make sure to sleep long enough so that some commit points will be deleted
        LuceneThread::threadSleep(1000.0 * (SECONDS / 5.0));
    }

    // First, make sure the policy in fact deleted something
    EXPECT_TRUE(policy->numDelete > 0); // no commits were deleted

    // Then simplistic check: just verify that the segments_N's that still exist are in fact within SECONDS
    // seconds of the last one's mod time, and, that I can open a reader on each
    int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);

    String fileName = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen);
    dir->deleteFile(IndexFileNames::SEGMENTS_GEN());
    while (gen > 0) {
        try {
            IndexReaderPtr reader = IndexReader::open(dir, true);
            reader->close();
            fileName = IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen);
            int64_t modTime = dir->fileModified(fileName);
            EXPECT_TRUE(lastDeleteTime - modTime <= (SECONDS * 1000));
        } catch (IOException&) {
            // OK
            break;
        }

        dir->deleteFile(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen));
        --gen;
    }

    dir->close();
}

/// Test a silly deletion policy that keeps all commits around.
TEST_F(DeletionPolicyTest, testDeletionPolicyKeepAllDeletionPolicy) {
    for (int32_t pass = 0; pass < 2; ++pass) {
        bool useCompoundFile = ((pass % 2) != 0);

        // Never deletes a commit
        DeletionPolicyKeepAllDeletionPolicyPtr policy = newLucene<DeletionPolicyKeepAllDeletionPolicy>();

        DirectoryPtr dir = newLucene<RAMDirectory>();
        policy->dir = dir;

        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(10);
        writer->setUseCompoundFile(useCompoundFile);
        writer->setMergeScheduler(newLucene<SerialMergeScheduler>());
        for (int32_t i = 0; i < 107; ++i) {
            addDoc(writer);
        }

        writer->close();

        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setUseCompoundFile(useCompoundFile);
        writer->optimize();
        writer->close();

        EXPECT_EQ(2, policy->numOnInit);

        // If we are not auto committing then there should be exactly 2 commits (one per close above)
        EXPECT_EQ(2, policy->numOnCommit);

        // Test listCommits
        Collection<IndexCommitPtr> commits = IndexReader::listCommits(dir);
        // 1 from opening writer + 2 from closing writer
        EXPECT_EQ(3, commits.size());

        // Make sure we can open a reader on each commit
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
            IndexReaderPtr r = IndexReader::open(*commit, IndexDeletionPolicyPtr(), false);
            r->close();
        }

        // Simplistic check: just verify all segments_N's still exist, and, I can open a reader on each
        dir->deleteFile(IndexFileNames::SEGMENTS_GEN());
        int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
        while (gen > 0) {
            IndexReaderPtr reader = IndexReader::open(dir, true);
            reader->close();
            dir->deleteFile(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen));
            --gen;

            if (gen > 0) {
                // Now that we've removed a commit point, which should have orphan'd at least one index file.
                // Open and close a writer and check that it actually removed something
                int32_t preCount = dir->listAll().size();
                writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthLIMITED);
                writer->close();
                int32_t postCount = dir->listAll().size();
                EXPECT_TRUE(postCount < preCount);
            }
        }

        dir->close();
    }
}

/// Uses DeletionPolicyKeepAllDeletionPolicy to keep all commits around, then, opens a new IndexWriter on a previous commit point.
TEST_F(DeletionPolicyTest, testOpenPriorSnapshot) {
    // Never deletes a commit
    DeletionPolicyKeepAllDeletionPolicyPtr policy = newLucene<DeletionPolicyKeepAllDeletionPolicy>();

    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    policy->dir = dir;

    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), (IndexDeletionPolicyPtr)policy, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    for (int32_t i = 0; i < 10; ++i) {
        addDoc(writer);
        if ((1 + i) % 2 == 0) {
            writer->commit();
        }
    }
    writer->close();

    Collection<IndexCommitPtr> commits = IndexReader::listCommits(dir);
    EXPECT_EQ(6, commits.size());
    IndexCommitPtr lastCommit;
    for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
        if (!lastCommit || (*commit)->getGeneration() > lastCommit->getGeneration()) {
            lastCommit = *commit;
        }
    }
    EXPECT_TRUE(lastCommit);

    // Now add 1 doc and optimize
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), (IndexDeletionPolicyPtr)policy, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(writer);
    EXPECT_EQ(11, writer->numDocs());
    writer->optimize();
    writer->close();

    EXPECT_EQ(7, IndexReader::listCommits(dir).size());

    // Now open writer on the commit just before optimize
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), policy, IndexWriter::MaxFieldLengthLIMITED, lastCommit);
    EXPECT_EQ(10, writer->numDocs());

    // Should undo our rollback
    writer->rollback();

    IndexReaderPtr r = IndexReader::open(dir, true);
    // Still optimized, still 11 docs
    EXPECT_TRUE(r->isOptimized());
    EXPECT_EQ(11, r->numDocs());
    r->close();

    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), policy, IndexWriter::MaxFieldLengthLIMITED, lastCommit);
    EXPECT_EQ(10, writer->numDocs());
    // Commits the rollback
    writer->close();

    // Now 8 because we made another commit
    EXPECT_EQ(8, IndexReader::listCommits(dir).size());

    r = IndexReader::open(dir, true);
    // Not optimized because we rolled it back, and now only 10 docs
    EXPECT_TRUE(!r->isOptimized());
    EXPECT_EQ(10, r->numDocs());
    r->close();

    // Reoptimize
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), (IndexDeletionPolicyPtr)policy, IndexWriter::MaxFieldLengthLIMITED);
    writer->optimize();
    writer->close();

    r = IndexReader::open(dir, true);
    EXPECT_TRUE(r->isOptimized());
    EXPECT_EQ(10, r->numDocs());
    r->close();

    // Now open writer on the commit just before optimize, but this time keeping only the last commit
    writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), newLucene<KeepOnlyLastCommitDeletionPolicy>(), IndexWriter::MaxFieldLengthLIMITED, lastCommit);
    EXPECT_EQ(10, writer->numDocs());

    // Reader still sees optimized index, because writer opened on the prior commit has not yet committed
    r = IndexReader::open(dir, true);
    EXPECT_TRUE(r->isOptimized());
    EXPECT_EQ(10, r->numDocs());
    r->close();

    writer->close();

    // Now reader sees unoptimized index:
    r = IndexReader::open(dir, true);
    EXPECT_TRUE(!r->isOptimized());
    EXPECT_EQ(10, r->numDocs());
    r->close();

    dir->close();
}

/// Test keeping NO commit points.  This is a viable and useful case eg where you want to build a big index and you know there are no readers.
TEST_F(DeletionPolicyTest, testKeepNoneOnInitDeletionPolicy) {
    for (int32_t pass = 0; pass < 2; ++pass) {
        bool useCompoundFile = ((pass % 2) != 0);

        KeepNoneOnInitDeletionPolicyPtr policy = newLucene<KeepNoneOnInitDeletionPolicy>();

        DirectoryPtr dir = newLucene<RAMDirectory>();

        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(10);
        writer->setUseCompoundFile(useCompoundFile);
        for (int32_t i = 0; i < 107; ++i) {
            addDoc(writer);
        }
        writer->close();

        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);

        writer->setUseCompoundFile(useCompoundFile);
        writer->optimize();
        writer->close();

        EXPECT_EQ(2, policy->numOnInit);
        // If we are not auto committing then there should be exactly 2 commits (one per close above)
        EXPECT_EQ(2, policy->numOnCommit);

        // Simplistic check: just verify the index is in fact readable
        IndexReaderPtr reader = IndexReader::open(dir, true);
        reader->close();

        dir->close();
    }
}

/// Test a deletion policy that keeps last N commits.
TEST_F(DeletionPolicyTest, testKeepLastNDeletionPolicy) {
    int32_t N = 5;

    for (int32_t pass = 0; pass < 2; ++pass) {
        bool useCompoundFile = ((pass % 2) != 0);

        DirectoryPtr dir = newLucene<RAMDirectory>();

        KeepLastNDeletionPolicyPtr policy = newLucene<KeepLastNDeletionPolicy>(N);

        for (int32_t j = 0; j < N + 1; ++j) {
            IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
            writer->setMaxBufferedDocs(10);
            writer->setUseCompoundFile(useCompoundFile);
            for (int32_t i = 0; i < 17; ++i) {
                addDoc(writer);
            }
            writer->optimize();
            writer->close();
        }

        EXPECT_TRUE(policy->numDelete > 0);
        EXPECT_EQ(N + 1, policy->numOnInit);
        EXPECT_EQ(N + 1, policy->numOnCommit);

        // Simplistic check: just verify only the past N segments_N's still exist, and, I can open a reader on each
        dir->deleteFile(IndexFileNames::SEGMENTS_GEN());
        int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);
        for (int32_t i = 0; i < N + 1; ++i) {
            try {
                IndexReaderPtr reader = IndexReader::open(dir, true);
                reader->close();
                if (i == N) {
                    FAIL() << "should have failed on commits prior to last";
                }
            } catch (IOException& e) {
                if (i != N) {
                    FAIL() << e.getError();
                }
            }
            if (i < N) {
                dir->deleteFile(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen));
            }
            --gen;
        }

        dir->close();
    }
}

/// Test a deletion policy that keeps last N commits around, with reader doing deletes.
TEST_F(DeletionPolicyTest, testKeepLastNDeletionPolicyWithReader) {
    int32_t N = 10;

    for (int32_t pass = 0; pass < 2; ++pass) {
        bool useCompoundFile = ((pass % 2) != 0);

        KeepLastNDeletionPolicyPtr policy = newLucene<KeepLastNDeletionPolicy>(N);

        DirectoryPtr dir = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setUseCompoundFile(useCompoundFile);
        writer->close();
        TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
        QueryPtr query = newLucene<TermQuery>(searchTerm);

        for (int32_t i = 0; i < N + 1; ++i) {
            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);
            writer->setUseCompoundFile(useCompoundFile);
            for (int32_t j = 0; j < 17; ++j) {
                addDoc(writer);
            }
            // this is a commit
            writer->close();
            IndexReaderPtr reader = IndexReader::open(dir, policy, false);
            reader->deleteDocument(3 * i + 1);
            reader->setNorm(4 * i + 1, L"content", 2.0);
            IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);
            Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
            EXPECT_EQ(16 * (1 + i), hits.size());
            // this is a commit
            reader->close();
            searcher->close();
        }
        writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setUseCompoundFile(useCompoundFile);

        writer->optimize();
        // this is a commit
        writer->close();

        EXPECT_EQ(2 * (N + 2), policy->numOnInit);
        EXPECT_EQ(2 * (N + 2) - 1, policy->numOnCommit);

        IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
        Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(176, hits.size());

        // Simplistic check: just verify only the past N segments_N's still exist, and, I can open a reader on each
        int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);

        dir->deleteFile(IndexFileNames::SEGMENTS_GEN());
        int32_t expectedCount = 176;

        for (int32_t i = 0; i < N + 1; ++i) {
            try {
                IndexReaderPtr reader = IndexReader::open(dir, true);
                // Work backwards in commits on what the expected count should be.
                searcher = newLucene<IndexSearcher>(reader);
                hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
                if (i > 1) {
                    if (i % 2 == 0) {
                        expectedCount += 1;
                    } else {
                        expectedCount -= 17;
                    }
                }
                EXPECT_EQ(expectedCount, hits.size());
                searcher->close();
                reader->close();
                if (i == N) {
                    FAIL() << "should have failed on commits before last 5";
                }
            } catch (IOException& e) {
                if (i != N) {
                    FAIL() << e.getError();
                }
            }
            if (i < N) {
                dir->deleteFile(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen));
            }
            --gen;
        }

        dir->close();
    }
}

/// Test a deletion policy that keeps last N commits around, through creates.
TEST_F(DeletionPolicyTest, testKeepLastNDeletionPolicyWithCreates) {
    int32_t N = 10;

    for (int32_t pass = 0; pass < 2; ++pass) {
        bool useCompoundFile = ((pass % 2) != 0);

        KeepLastNDeletionPolicyPtr policy = newLucene<KeepLastNDeletionPolicy>(N);

        DirectoryPtr dir = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setMaxBufferedDocs(10);
        writer->setUseCompoundFile(useCompoundFile);
        writer->close();
        TermPtr searchTerm = newLucene<Term>(L"content", L"aaa");
        QueryPtr query = newLucene<TermQuery>(searchTerm);

        for (int32_t i = 0; i < N + 1; ++i) {
            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, policy, IndexWriter::MaxFieldLengthUNLIMITED);
            writer->setMaxBufferedDocs(10);
            writer->setUseCompoundFile(useCompoundFile);
            for (int32_t j = 0; j < 17; ++j) {
                addDoc(writer);
            }
            // this is a commit
            writer->close();
            IndexReaderPtr reader = IndexReader::open(dir, policy, false);
            reader->deleteDocument(3);
            reader->setNorm(5, L"content", 2.0);
            IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);
            Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
            EXPECT_EQ(16, hits.size());
            // this is a commit
            reader->close();
            searcher->close();

            writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), true, policy, IndexWriter::MaxFieldLengthUNLIMITED);
            // This will not commit: there are no changes pending because we opened for "create"
            writer->close();
        }

        EXPECT_EQ(1 + 3 * (N + 1), policy->numOnInit);
        EXPECT_EQ(3 * ( N + 1), policy->numOnCommit);

        IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, false);
        Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(0, hits.size());

        // Simplistic check: just verify only the past N segments_N's still exist, and, I can open a reader on each
        int64_t gen = SegmentInfos::getCurrentSegmentGeneration(dir);

        dir->deleteFile(IndexFileNames::SEGMENTS_GEN());
        int32_t expectedCount = 0;

        for (int32_t i = 0; i < N + 1; ++i) {
            try {
                IndexReaderPtr reader = IndexReader::open(dir, true);
                // Work backwards in commits on what the expected count should be.
                searcher = newLucene<IndexSearcher>(reader);
                hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
                EXPECT_EQ(expectedCount, hits.size());
                searcher->close();
                if (expectedCount == 0) {
                    expectedCount = 16;
                } else if (expectedCount == 16) {
                    expectedCount = 17;
                } else if (expectedCount == 17) {
                    expectedCount = 0;
                }
                reader->close();
                if (i == N) {
                    FAIL() << "should have failed on commits before last";
                }
            } catch (IOException& e) {
                if (i != N) {
                    FAIL() << e.getError();
                }
            }
            if (i < N) {
                dir->deleteFile(IndexFileNames::fileNameFromGeneration(IndexFileNames::SEGMENTS(), L"", gen));
            }
            --gen;
        }

        dir->close();
    }
}
