/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexCommit.h"

using namespace Lucene;

typedef LuceneTestFixture IndexCommitTest;

namespace TestEqualsHashCode {

class TestIndexCommit1 : public IndexCommit {
public:
    TestIndexCommit1(const DirectoryPtr& dir) {
        this->dir = dir;
    }

    virtual ~TestIndexCommit1() {
    }

protected:
    DirectoryPtr dir;

public:
    virtual String getSegmentsFileName() {
        return L"a";
    }

    virtual int64_t getVersion() {
        return 12;
    }

    virtual DirectoryPtr getDirectory() {
        return dir;
    }

    virtual HashSet<String> getFileNames() {
        return HashSet<String>();
    }

    virtual void deleteCommit() {
    }

    virtual int64_t getGeneration() {
        return 0;
    }

    virtual int64_t getTimestamp() {
        return -1;
    }

    virtual MapStringString getUserData() {
        return MapStringString();
    }

    virtual bool isDeleted() {
        return false;
    }

    virtual bool isOptimized() {
        return false;
    }
};

class TestIndexCommit2 : public TestIndexCommit1 {
public:
    TestIndexCommit2(const DirectoryPtr& dir) : TestIndexCommit1(dir) {
    }

    virtual ~TestIndexCommit2() {
    }

public:
    virtual String getSegmentsFileName() {
        return L"b";
    }
};

}

TEST_F(IndexCommitTest, testEqualsHashCode) {
    DirectoryPtr dir = newLucene<RAMDirectory>();

    IndexCommitPtr ic1 = newLucene<TestEqualsHashCode::TestIndexCommit1>(dir);
    IndexCommitPtr ic2 = newLucene<TestEqualsHashCode::TestIndexCommit2>(dir);

    EXPECT_TRUE(ic1->equals(ic2));
    EXPECT_EQ(ic1->hashCode(), ic2->hashCode());
}
