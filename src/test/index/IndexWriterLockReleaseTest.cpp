/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexWriter.h"
#include "FSDirectory.h"
#include "StandardAnalyzer.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture IndexWriterLockReleaseTest;

TEST_F(IndexWriterLockReleaseTest, testIndexWriterLockRelease) {
    String testDir(getTempDir(L"testIndexWriter"));
    FileUtils::createDirectory(testDir);

    DirectoryPtr dir = FSDirectory::open(testDir);
    IndexWriterPtr im;

    try {
        im = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    } catch (FileNotFoundException& e) {
        EXPECT_TRUE(check_exception(LuceneException::FileNotFound)(e));
    }

    try {
        im = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    } catch (FileNotFoundException& e) {
        EXPECT_TRUE(check_exception(LuceneException::FileNotFound)(e));
    }

    dir->close();

    FileUtils::removeDirectory(testDir);
}
