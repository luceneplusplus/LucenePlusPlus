/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "test_lucene.h"
#include "TestUtils.h"
#include "CheckIndex.h"
#include "ConcurrentMergeScheduler.h"
#include "IndexWriter.h"
#include "Random.h"
#include "MiscUtils.h"
#include "FileUtils.h"

namespace Lucene
{
    static RandomPtr randomTest = newLucene<Random>();
    static String testDir;
    
    void setTestDir(const String& dir)
    {
        testDir = dir;
    }
    
    String getTestDir()
    {
        if (testDir.empty())
            boost::throw_exception(RuntimeException(L"test directory not set"));
        return testDir;
    }
    
    String getTempDir()
    {
        static String tempDir;
        
        if (tempDir.empty())
        {
            tempDir = FileUtils::joinPath(getTestDir(), L"temp");
            FileUtils::createDirectory(tempDir);
        }
        
        return tempDir;
    }
    
    String getTempDir(const String& desc)
    {
        return FileUtils::joinPath(getTempDir(), desc + L"." + StringUtils::toString(randomTest->nextInt()));
    }
    
    void syncConcurrentMerges(IndexWriterPtr writer)
    {
        syncConcurrentMerges(writer->getMergeScheduler());
    }

    void syncConcurrentMerges(MergeSchedulerPtr ms)
    {
        if (MiscUtils::typeOf<ConcurrentMergeScheduler>(ms))
            boost::static_pointer_cast<ConcurrentMergeScheduler>(ms)->sync();
    }
        
    bool checkIndex(DirectoryPtr dir)
    {
        CheckIndexPtr checker = newLucene<CheckIndex>(dir);
        IndexStatusPtr indexStatus = checker->checkIndex();
        if (!indexStatus || !indexStatus->clean)
        {
            boost::throw_exception(RuntimeException(L"CheckIndex failed"));
            return false;
        }
        return true;
    }
}
