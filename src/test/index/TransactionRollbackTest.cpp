/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MockRAMDirectory.h"
#include "IndexDeletionPolicy.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "BitSet.h"
#include "IndexCommit.h"

using namespace Lucene;

/// Keeps all commit points (used to build index)
class TransactionRollbackKeepAllDeletionPolicy : public IndexDeletionPolicy {
public:
    virtual ~TransactionRollbackKeepAllDeletionPolicy() {
    }

    LUCENE_CLASS(TransactionRollbackKeepAllDeletionPolicy);

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
    }
};

/// Rolls back to previous commit point
class RollbackDeletionPolicy : public IndexDeletionPolicy {
public:
    RollbackDeletionPolicy(int32_t rollbackPoint) {
        this->rollbackPoint = rollbackPoint;
    }

    virtual ~RollbackDeletionPolicy() {
    }

    LUCENE_CLASS(RollbackDeletionPolicy);

protected:
    int32_t rollbackPoint;

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
            MapStringString userData = (*commit)->getUserData();
            if (!userData.empty()) {
                // Label for a commit point is "Records 1-30"
                // This code reads the last id ("30" in this example) and deletes it if it is after the desired rollback point
                String x = userData.get(L"index");
                String lastVal = x.substr(x.find_last_of(L"-") + 1);
                int32_t last = StringUtils::toInt(lastVal);
                if (last > rollbackPoint) {
                    (*commit)->deleteCommit();
                }
            }
        }
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
    }
};

class DeleteLastCommitPolicy : public IndexDeletionPolicy {
public:
    virtual ~DeleteLastCommitPolicy() {
    }

    LUCENE_CLASS(DeleteLastCommitPolicy);

public:
    virtual void onInit(Collection<IndexCommitPtr> commits) {
        commits[commits.size() - 1]->deleteCommit();
    }

    virtual void onCommit(Collection<IndexCommitPtr> commits) {
    }
};

/// Test class to illustrate using IndexDeletionPolicy to provide multi-level rollback capability.
/// This test case creates an index of records 1 to 100, introducing a commit point every 10 records.
///
/// A "keep all" deletion policy is used to ensure we keep all commit points for testing purposes
class TransactionRollbackTest : public LuceneTestFixture {
public:
    TransactionRollbackTest() {
        FIELD_RECORD_ID = L"record_id";
        dir = newLucene<MockRAMDirectory>();
        // Build index, of records 1 to 100, committing after each batch of 10
        IndexDeletionPolicyPtr sdp = newLucene<TransactionRollbackKeepAllDeletionPolicy>();
        IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), sdp, IndexWriter::MaxFieldLengthUNLIMITED);
        for (int32_t currentRecordId = 1; currentRecordId <= 100; ++currentRecordId) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD_RECORD_ID, StringUtils::toString(currentRecordId), Field::STORE_YES, Field::INDEX_ANALYZED));
            w->addDocument(doc);

            if (currentRecordId % 10 == 0) {
                MapStringString data = MapStringString::newInstance();
                data.put(L"index", L"records 1-" + StringUtils::toString(currentRecordId));
                w->commit(data);
            }
        }

        w->close();
    }

    virtual ~TransactionRollbackTest() {
    }

protected:
    String FIELD_RECORD_ID;
    DirectoryPtr dir;

public:
    /// Rolls back index to a chosen ID
    void rollBackLast(int32_t id) {
        String ids = L"-" + StringUtils::toString(id);
        IndexCommitPtr last;
        Collection<IndexCommitPtr> commits = IndexReader::listCommits(dir);
        for (Collection<IndexCommitPtr>::iterator commit = commits.begin(); commit != commits.end(); ++commit) {
            MapStringString ud = (*commit)->getUserData();
            if (!ud.empty()) {
                if (boost::ends_with(ud.get(L"index"), ids)) {
                    last = *commit;
                }
            }
        }

        EXPECT_TRUE(last);

        IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), newLucene<RollbackDeletionPolicy>(id), IndexWriter::MaxFieldLengthUNLIMITED, last);
        MapStringString data = MapStringString::newInstance();
        data.put(L"index", L"Rolled back to 1-" + StringUtils::toString(id));
        w->commit(data);
        w->close();
    }

    void checkExpecteds(const BitSetPtr& expecteds) {
        IndexReaderPtr r = IndexReader::open(dir, true);

        // Perhaps not the most efficient approach but meets our needs here.
        for (int32_t i = 0; i < r->maxDoc(); ++i) {
            if (!r->isDeleted(i)) {
                String sval = r->document(i)->get(FIELD_RECORD_ID);
                if (!sval.empty()) {
                    int32_t val = StringUtils::toInt(sval);
                    EXPECT_TRUE(expecteds->get(val));
                    expecteds->set(val, false);
                }
            }
        }
        r->close();
        EXPECT_EQ(0, expecteds->cardinality());
    }
};

TEST_F(TransactionRollbackTest, testRepeatedRollBacks) {
    int32_t expectedLastRecordId = 100;
    while (expectedLastRecordId > 10) {
        expectedLastRecordId -= 10;
        rollBackLast(expectedLastRecordId);

        BitSetPtr expecteds = newLucene<BitSet>(100);
        expecteds->set(1, (expectedLastRecordId + 1), true);
        checkExpecteds(expecteds);
    }
}

TEST_F(TransactionRollbackTest, testRollbackDeletionPolicy) {
    for (int32_t i = 0; i < 2; ++i) {
        // Unless you specify a prior commit point, rollback should not work
        newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), (IndexDeletionPolicyPtr)newLucene<DeleteLastCommitPolicy>(), IndexWriter::MaxFieldLengthUNLIMITED)->close();
        IndexReaderPtr r = IndexReader::open(dir, true);
        EXPECT_EQ(100, r->numDocs());
        r->close();
    }
}
