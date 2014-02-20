/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"

using namespace Lucene;

typedef LuceneTestFixture VersionTest;

TEST_F(VersionTest, testVersion) {
    for (int32_t version = (int32_t)LuceneVersion::LUCENE_20; version <= (int32_t)LuceneVersion::LUCENE_CURRENT; ++version) {
        EXPECT_TRUE(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_CURRENT, (LuceneVersion::Version)version));
    }
    EXPECT_TRUE(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_30, LuceneVersion::LUCENE_29));
    EXPECT_TRUE(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_30, LuceneVersion::LUCENE_30));
    EXPECT_TRUE(!LuceneVersion::onOrAfter(LuceneVersion::LUCENE_29, LuceneVersion::LUCENE_30));
}
