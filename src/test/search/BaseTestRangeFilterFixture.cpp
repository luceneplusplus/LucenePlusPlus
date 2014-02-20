/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTestRangeFilterFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Random.h"

namespace Lucene {

TestIndex::TestIndex(int32_t minR, int32_t maxR, bool allowNegativeRandomInts) {
    this->minR = minR;
    this->maxR = maxR;
    this->allowNegativeRandomInts = allowNegativeRandomInts;
    this->index = newLucene<RAMDirectory>();
}

TestIndex::~TestIndex() {
}

BaseTestRangeFilterFixture::BaseTestRangeFilterFixture() {
    signedIndex = newLucene<TestIndex>(INT_MAX, INT_MIN, true);
    unsignedIndex = newLucene<TestIndex>(INT_MAX, 0, false);
    minId = 0;
    maxId = 10000;
    intLength = StringUtils::toString(INT_MAX).length();
    random = newLucene<Random>();

    build(signedIndex);
    build(unsignedIndex);
}

BaseTestRangeFilterFixture::~BaseTestRangeFilterFixture() {
}

void BaseTestRangeFilterFixture::build(const TestIndexPtr& index) {
    IndexWriterPtr writer = newLucene<IndexWriter>(index->index, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t d = minId; d <= maxId; ++d) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", pad(d), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        int32_t r = index->allowNegativeRandomInts ? random->nextInt() : random->nextInt(INT_MAX);
        index->maxR = std::max(index->maxR, r);
        index->minR = std::min(index->minR, r);
        doc->add(newLucene<Field>(L"rand", pad(r), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"body", L"body", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }

    writer->optimize();
    writer->close();
}

String BaseTestRangeFilterFixture::pad(int32_t n) {
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

}
