/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "CompressionTools.h"

using namespace Lucene;

typedef LuceneTestFixture CompressionToolsTest;

TEST_F(CompressionToolsTest, testCompressDecompress) {
    ByteArray compress(CompressionTools::compressString(L"test compressed string"));
    EXPECT_TRUE(compress.size() > 0);

    String decompress(CompressionTools::decompressString(compress));
    EXPECT_EQ(decompress, L"test compressed string");
}
