/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexCommit.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(IndexCommitTest, LuceneTestFixture)

namespace TestEqualsHashCode
{
    class TestIndexCommit1 : public IndexCommit
    {
    public:
        TestIndexCommit1(DirectoryPtr dir)
        {
            this->dir = dir;
        }
        
        virtual ~TestIndexCommit1()
        {
        }

    protected:
        DirectoryPtr dir;

    public:
        virtual String getSegmentsFileName()
        {
            return L"a";
        }
        
        virtual int64_t getVersion()
        {
            return 12;
        }
        
        virtual DirectoryPtr getDirectory()
        {
            return dir;
        }
        
        virtual HashSet<String> getFileNames()
        {
            return HashSet<String>();
        }
        
        virtual void deleteCommit()
        {
        }
        
        virtual int64_t getGeneration()
        {
            return 0;
        }
        
        virtual int64_t getTimestamp()
        {
            return -1;
        }
        
        virtual MapStringString getUserData()
        {
            return MapStringString();
        }
        
        virtual bool isDeleted()
        {
            return false;
        }
        
        virtual bool isOptimized()
        {
            return false;
        }
    };
    
    class TestIndexCommit2 : public TestIndexCommit1
    {
    public:
        TestIndexCommit2(DirectoryPtr dir) : TestIndexCommit1(dir)
        {
        }
        
        virtual ~TestIndexCommit2()
        {
        }
    
    public:
        virtual String getSegmentsFileName()
        {
            return L"b";
        }
    };
}

BOOST_AUTO_TEST_CASE(testEqualsHashCode)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    
    IndexCommitPtr ic1 = newLucene<TestEqualsHashCode::TestIndexCommit1>(dir);
    IndexCommitPtr ic2 = newLucene<TestEqualsHashCode::TestIndexCommit2>(dir);
    
    BOOST_CHECK(ic1->equals(ic2));
    BOOST_CHECK_EQUAL(ic1->hashCode(), ic2->hashCode());
}

BOOST_AUTO_TEST_SUITE_END()
