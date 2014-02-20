/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FilterIndexReader.h"
#include "IndexReader.h"
#include "IndexWriter.h"
#include "MockRAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TermEnum.h"
#include "Term.h"

using namespace Lucene;

DECLARE_SHARED_PTR(TestReader)
DECLARE_SHARED_PTR(TestTermEnum)
DECLARE_SHARED_PTR(TestTermPositions)

/// Filter that only permits terms containing 'e'
class TestTermEnum : public FilterTermEnum {
public:
    TestTermEnum(const TermEnumPtr& termEnum) : FilterTermEnum(termEnum) {
    }

    virtual ~TestTermEnum() {
    }

    LUCENE_CLASS(TestTermEnum);

public:
    virtual bool next() {
        while (in->next()) {
            if (in->term()->text().find(L'e') != String::npos) {
                return true;
            }
        }
        return false;
    }
};

/// Filter that only returns odd numbered documents.
class TestTermPositions : public FilterTermPositions {
public:
    TestTermPositions(const TermPositionsPtr& in) : FilterTermPositions(in) {
    }

    virtual ~TestTermPositions() {
    }

    LUCENE_CLASS(TestTermPositions);

public:
    virtual bool next() {
        while (in->next()) {
            if ((in->doc() % 2) == 1) {
                return true;
            }
        }
        return false;
    }
};

class TestReader : public FilterIndexReader {
public:
    TestReader(const IndexReaderPtr& reader) : FilterIndexReader(reader) {
    }

    virtual ~TestReader() {
    }

    LUCENE_CLASS(TestReader);

public:
    /// Filter terms with TestTermEnum.
    virtual TermEnumPtr terms() {
        return newLucene<TestTermEnum>(in->terms());
    }

    /// Filter positions with TestTermPositions.
    virtual TermPositionsPtr termPositions() {
        return newLucene<TestTermPositions>(in->termPositions());
    }
};

typedef LuceneTestFixture FilterIndexReaderTest;

/// Tests the IndexReader::getFieldNames implementation
TEST_F(FilterIndexReaderTest, testFilterIndexReader) {
    RAMDirectoryPtr directory = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr d1 = newLucene<Document>();
    d1->add(newLucene<Field>(L"default", L"one two", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(d1);

    DocumentPtr d2 = newLucene<Document>();
    d2->add(newLucene<Field>(L"default", L"one three", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(d2);

    DocumentPtr d3 = newLucene<Document>();
    d2->add(newLucene<Field>(L"default", L"two four", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(d3);

    writer->close();

    IndexReaderPtr reader = newLucene<TestReader>(IndexReader::open(directory, true));

    EXPECT_TRUE(reader->isOptimized());

    TermEnumPtr terms = reader->terms();
    while (terms->next()) {
        EXPECT_NE(terms->term()->text().find(L'e'), String::npos);
    }
    terms->close();

    TermPositionsPtr positions = reader->termPositions(newLucene<Term>(L"default", L"one"));
    while (positions->next()) {
        EXPECT_TRUE((positions->doc() % 2) == 1);
    }

    int32_t NUM_DOCS = 3;

    TermDocsPtr td = reader->termDocs(TermPtr());
    for (int32_t i = 0; i < NUM_DOCS; ++i) {
        EXPECT_TRUE(td->next());
        EXPECT_EQ(i, td->doc());
        EXPECT_EQ(1, td->freq());
    }
    td->close();
    reader->close();
    directory->close();
}
