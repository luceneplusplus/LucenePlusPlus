/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Random.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "NumericField.h"
#include "IndexSearcher.h"
#include "TermRangeQuery.h"
#include "NumericRangeQuery.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture MultiValuedNumericRangeQueryTest;

static String pad(int32_t n) {
    int32_t intLength = String(L"00000000000").length();

    StringStream buf;
    String p = L"0";
    if (n < 0) {
        p = L"-";
        n = INT_MAX + n + 1;
    }
    buf << p;
    String s = StringUtils::toString(n);
    for (int32_t i = s.length(); i <= intLength; ++i) {
        buf << L"0";
    }
    buf << s;
    return buf.str();
}

TEST_F(MultiValuedNumericRangeQueryTest, testMultiValuedNRQ) {
    RandomPtr rnd = newLucene<Random>();

    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthUNLIMITED);

    for (int32_t l = 0; l < 5000; ++l) {
        DocumentPtr doc = newLucene<Document>();
        for (int32_t m = 0, c = rnd->nextInt(10); m <= c; ++m) {
            int32_t value = rnd->nextInt(INT_MAX);
            doc->add(newLucene<Field>(L"asc", pad(value), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<NumericField>(L"trie", Field::STORE_NO, true)->setIntValue(value));
        }
        writer->addDocument(doc);
    }
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
    for (int32_t i = 0; i < 50; ++i) {
        int32_t lower = rnd->nextInt(INT_MAX);
        int32_t upper = rnd->nextInt(INT_MAX);
        if (lower > upper) {
            std::swap(lower, upper);
        }
        TermRangeQueryPtr cq = newLucene<TermRangeQuery>(L"asc", pad(lower), pad(upper), true, true);
        NumericRangeQueryPtr tq = NumericRangeQuery::newIntRange(L"trie", lower, upper, true, true);
        TopDocsPtr trTopDocs = searcher->search(cq, 1);
        TopDocsPtr nrTopDocs = searcher->search(tq, 1);
        EXPECT_EQ(trTopDocs->totalHits, nrTopDocs->totalHits);
    }
    searcher->close();
}
