/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneGlobalFixture.h"
#include "TestUtils.h"
#include "TestPoint.h"
#include "FileUtils.h"

namespace Lucene {

void LuceneGlobalFixture::SetUp() {
    FileUtils::removeDirectory(getTempDir());
    FileUtils::createDirectory(getTempDir());
    TestPoint::enableTestPoints();
}

void LuceneGlobalFixture::TearDown() {
    FileUtils::removeDirectory(getTempDir());
    Lucene::CycleCheck::dumpRefs();
}

}
