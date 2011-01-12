/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "ConcurrentMergeScheduler.h"
#include "DateTools.h"

namespace Lucene
{
    LuceneTestFixture::LuceneTestFixture()
    {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        ConcurrentMergeScheduler::setTestMode();
    }
    
    LuceneTestFixture::~LuceneTestFixture()
    {
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
        if (ConcurrentMergeScheduler::anyUnhandledExceptions())
        {
            // Clear the failure so that we don't just keep failing subsequent test cases
            ConcurrentMergeScheduler::clearUnhandledExceptions();
            BOOST_FAIL("ConcurrentMergeScheduler hit unhandled exceptions");
        }
    }
}
