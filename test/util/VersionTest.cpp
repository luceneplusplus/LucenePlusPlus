/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(VersionTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testVersion)
{
    for (int32_t version = (int32_t)LuceneVersion::LUCENE_20; version <= (int32_t)LuceneVersion::LUCENE_CURRENT; ++version)
        BOOST_CHECK(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_CURRENT, (LuceneVersion::Version)version));
    BOOST_CHECK(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_30, LuceneVersion::LUCENE_29));
    BOOST_CHECK(LuceneVersion::onOrAfter(LuceneVersion::LUCENE_30, LuceneVersion::LUCENE_30));
    BOOST_CHECK(!LuceneVersion::onOrAfter(LuceneVersion::LUCENE_29, LuceneVersion::LUCENE_30));
}

BOOST_AUTO_TEST_SUITE_END()
