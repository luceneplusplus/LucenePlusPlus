/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "MMapDirectory.h"
#include "FSDirectory.h"
#include "StandardAnalyzer.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "Document.h"
#include "Field.h"
#include "Random.h"
#include "FileUtils.h"

using namespace Lucene;

typedef LuceneTestFixture MMapDirectoryTest;

static RandomPtr rndToken = newLucene<Random>();

String randomToken() {
    static const wchar_t* alphabet = L"abcdefghijklmnopqrstuvwxyz";
    int32_t tl = 1 + rndToken->nextInt(7);
    StringStream sb;
    for (int32_t cx = 0; cx < tl; ++cx) {
        sb << alphabet[rndToken->nextInt(25)];
    }
    return sb.str();
}

String randomField() {
    int32_t fl = 1 + rndToken->nextInt(3);
    StringStream fb;
    for (int32_t fx = 0; fx < fl; ++fx) {
        fb << randomToken() << L" ";
    }
    return fb.str();
}

TEST_F(MMapDirectoryTest, testMmapIndex) {
    String storePathname(FileUtils::joinPath(getTempDir(), L"testLuceneMmap"));

    FSDirectoryPtr storeDirectory(newLucene<MMapDirectory>(storePathname));

    // plan to add a set of useful stopwords, consider changing some of the interior filters.
    StandardAnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT, HashSet<String>());

    IndexWriterPtr writer = newLucene<IndexWriter>(storeDirectory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(storeDirectory, true);

    for (int32_t dx = 0; dx < 1000; ++dx) {
        String f(randomField());
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"data", f, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }

    searcher->close();
    writer->close();

    FileUtils::removeDirectory(storePathname);
}
