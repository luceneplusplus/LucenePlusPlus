/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "ConcurrentMergeScheduler.h"
#include "DateTools.h"

namespace Lucene {

LuceneTestFixture::LuceneTestFixture() {
    DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
    ConcurrentMergeScheduler::setTestMode();
}

LuceneTestFixture::~LuceneTestFixture() {
    // Moved out to a separate function since GTEST_FAIL cannot be used in destructors
    destructorBody();
}

void LuceneTestFixture::destructorBody() {
    DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
    if (ConcurrentMergeScheduler::anyUnhandledExceptions()) {
        // Clear the failure so that we don't just keep failing subsequent test cases
        ConcurrentMergeScheduler::clearUnhandledExceptions();
        boost::throw_exception(RuntimeException(L"ConcurrentMergeScheduler hit unhandled exceptions"));
    }
}

}
