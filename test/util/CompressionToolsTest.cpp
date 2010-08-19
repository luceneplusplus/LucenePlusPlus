/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "CompressionTools.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CompressionToolsTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testCompressDecompress)
{
    ByteArray compress(CompressionTools::compressString(L"test compressed string"));
    BOOST_CHECK(compress.length() > 0);
    
    String decompress(CompressionTools::decompressString(compress));
    BOOST_CHECK_EQUAL(decompress, L"test compressed string");
}

BOOST_AUTO_TEST_SUITE_END()
