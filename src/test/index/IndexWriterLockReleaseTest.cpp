/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(IndexWriterLockReleaseTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testIndexWriterLockRelease)
{
    String testDir(getTempDir(L"testIndexWriter"));
    FileUtils::createDirectory(testDir);
    
    DirectoryPtr dir = FSDirectory::open(testDir);
    IndexWriterPtr im;
    
    BOOST_CHECK_EXCEPTION(im = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED), FileNotFoundException, check_exception(LuceneException::FileNotFound));
    
    BOOST_CHECK_EXCEPTION(im = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED), FileNotFoundException, check_exception(LuceneException::FileNotFound));
    
    dir->close();
    
    FileUtils::removeDirectory(testDir);
}

BOOST_AUTO_TEST_SUITE_END()
