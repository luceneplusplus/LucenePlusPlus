/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexReader.h"
#include "Document.h"
#include "Field.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "TermDocs.h"
#include "TermEnum.h"
#include "ParallelReader.h"
#include "Term.h"

using namespace Lucene;

class ParallelTermEnumTest : public LuceneTestFixture {
public:
    ParallelTermEnumTest() {
        RAMDirectoryPtr rd1 = newLucene<RAMDirectory>();
        IndexWriterPtr iw1 = newLucene<IndexWriter>(rd1, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field1", L"the quick brown fox jumps", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"field2", L"the quick brown fox jumps", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"field4", L"", Field::STORE_NO, Field::INDEX_ANALYZED));
        iw1->addDocument(doc);

        iw1->close();
        RAMDirectoryPtr rd2 = newLucene<RAMDirectory>();
        IndexWriterPtr iw2 = newLucene<IndexWriter>(rd2, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field0", L"", Field::STORE_NO, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"field1", L"the fox jumps over the lazy dog", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"field3", L"the fox jumps over the lazy dog", Field::STORE_YES, Field::INDEX_ANALYZED));
        iw2->addDocument(doc);

        iw2->close();

        this->ir1 = IndexReader::open(rd1, true);
        this->ir2 = IndexReader::open(rd2, true);
    }

    virtual ~ParallelTermEnumTest() {
        ir1->close();
        ir2->close();
    }

public:
    IndexReaderPtr ir1;
    IndexReaderPtr ir2;
};

TEST_F(ParallelTermEnumTest, testParallelTermEnum) {
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(ir1);
    pr->add(ir2);

    TermDocsPtr td = pr->termDocs();
    TermEnumPtr te = pr->terms();

    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field1:brown", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field1:fox", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field1:jumps", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field1:quick", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field1:the", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field2:brown", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field2:fox", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field2:jumps", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field2:quick", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field2:the", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:dog", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:fox", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:jumps", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:lazy", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:over", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(te->next());
    EXPECT_EQ(L"field3:the", te->term()->toString());
    td->seek(te->term());
    EXPECT_TRUE(td->next());
    EXPECT_EQ(0, td->doc());
    EXPECT_TRUE(!td->next());
    EXPECT_TRUE(!te->next());
}
