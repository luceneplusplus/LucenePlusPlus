/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexSearcher.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "ParallelReader.h"
#include "IndexReader.h"
#include "TermQuery.h"
#include "Term.h"
#include "BooleanQuery.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "MapFieldSelector.h"
#include "TermDocs.h"

using namespace Lucene;

class ParallelReaderTest : public LuceneTestFixture {
public:
    ParallelReaderTest() {
        single = createSingle();
        parallel = createParallel();
    }

    virtual ~ParallelReaderTest() {
    }

public:
    SearcherPtr parallel;
    SearcherPtr single;

public:
    /// Fields 1-4 indexed together
    SearcherPtr createSingle() {
        DirectoryPtr dir = newLucene<MockRAMDirectory>();
        IndexWriterPtr w = newLucene<IndexWriter>(dir, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr d1 = newLucene<Document>();
        d1->add(newLucene<Field>(L"f1", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        d1->add(newLucene<Field>(L"f2", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        d1->add(newLucene<Field>(L"f3", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        d1->add(newLucene<Field>(L"f4", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        w->addDocument(d1);
        DocumentPtr d2 = newLucene<Document>();
        d2->add(newLucene<Field>(L"f1", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        d2->add(newLucene<Field>(L"f2", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        d2->add(newLucene<Field>(L"f3", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        d2->add(newLucene<Field>(L"f4", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        w->addDocument(d2);
        w->close();
        return newLucene<IndexSearcher>(dir, false);
    }

    /// Fields 1 & 2 in one index, 3 & 4 in other, with ParallelReader
    SearcherPtr createParallel() {
        DirectoryPtr dir1 = getDir1();
        DirectoryPtr dir2 = getDir2();
        ParallelReaderPtr pr = newLucene<ParallelReader>();
        pr->add(IndexReader::open(dir1, false));
        pr->add(IndexReader::open(dir2, false));
        return newLucene<IndexSearcher>(pr);
    }

    DirectoryPtr getDir1() {
        DirectoryPtr dir1 = newLucene<MockRAMDirectory>();
        IndexWriterPtr w1 = newLucene<IndexWriter>(dir1, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr d1 = newLucene<Document>();
        d1->add(newLucene<Field>(L"f1", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        d1->add(newLucene<Field>(L"f2", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        w1->addDocument(d1);
        DocumentPtr d2 = newLucene<Document>();
        d2->add(newLucene<Field>(L"f1", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        d2->add(newLucene<Field>(L"f2", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        w1->addDocument(d2);
        w1->close();
        return dir1;
    }

    DirectoryPtr getDir2() {
        DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
        IndexWriterPtr w2 = newLucene<IndexWriter>(dir2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr d3 = newLucene<Document>();
        d3->add(newLucene<Field>(L"f3", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        d3->add(newLucene<Field>(L"f4", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
        w2->addDocument(d3);
        DocumentPtr d4 = newLucene<Document>();
        d4->add(newLucene<Field>(L"f3", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        d4->add(newLucene<Field>(L"f4", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
        w2->addDocument(d4);
        w2->close();
        return dir2;
    }

    void queryTest(const QueryPtr& query) {
        Collection<ScoreDocPtr> parallelHits = parallel->search(query, FilterPtr(), 1000)->scoreDocs;
        Collection<ScoreDocPtr> singleHits = single->search(query, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(parallelHits.size(), singleHits.size());
        for (int32_t i = 0; i < parallelHits.size(); ++i) {
            EXPECT_NEAR(parallelHits[i]->score, singleHits[i]->score, 0.001);
            DocumentPtr docParallel = parallel->doc(parallelHits[i]->doc);
            DocumentPtr docSingle = single->doc(singleHits[i]->doc);
            EXPECT_EQ(docParallel->get(L"f1"), docSingle->get(L"f1"));
            EXPECT_EQ(docParallel->get(L"f2"), docSingle->get(L"f2"));
            EXPECT_EQ(docParallel->get(L"f3"), docSingle->get(L"f3"));
            EXPECT_EQ(docParallel->get(L"f4"), docSingle->get(L"f4"));
        }
    }
};

TEST_F(ParallelReaderTest, testQueries) {
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f1", L"v1")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f2", L"v1")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f2", L"v2")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f3", L"v1")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f3", L"v2")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f4", L"v1")));
    queryTest(newLucene<TermQuery>(newLucene<Term>(L"f4", L"v2")));

    BooleanQueryPtr bq1 = newLucene<BooleanQuery>();
    bq1->add(newLucene<TermQuery>(newLucene<Term>(L"f1", L"v1")), BooleanClause::MUST);
    bq1->add(newLucene<TermQuery>(newLucene<Term>(L"f4", L"v1")), BooleanClause::MUST);
    queryTest(bq1);
}

TEST_F(ParallelReaderTest, testFieldNames) {
    DirectoryPtr dir1 = getDir1();
    DirectoryPtr dir2 = getDir2();
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));
    HashSet<String> fieldNames = pr->getFieldNames(IndexReader::FIELD_OPTION_ALL);
    EXPECT_EQ(4, fieldNames.size());
    EXPECT_TRUE(fieldNames.contains(L"f1"));
    EXPECT_TRUE(fieldNames.contains(L"f2"));
    EXPECT_TRUE(fieldNames.contains(L"f3"));
    EXPECT_TRUE(fieldNames.contains(L"f4"));
}

TEST_F(ParallelReaderTest, testDocument) {
    DirectoryPtr dir1 = getDir1();
    DirectoryPtr dir2 = getDir2();
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));

    Collection<String> fields1 = newCollection<String>(L"f1");
    Collection<String> fields2 = newCollection<String>(L"f4");
    Collection<String> fields3 = newCollection<String>(L"f2", L"f3");

    DocumentPtr doc11 = pr->document(0, newLucene<MapFieldSelector>(fields1));
    DocumentPtr doc24 = pr->document(1, newLucene<MapFieldSelector>(fields2));
    DocumentPtr doc223 = pr->document(1, newLucene<MapFieldSelector>(fields3));

    EXPECT_EQ(1, doc11->getFields().size());
    EXPECT_EQ(1, doc24->getFields().size());
    EXPECT_EQ(2, doc223->getFields().size());

    EXPECT_EQ(L"v1", doc11->get(L"f1"));
    EXPECT_EQ(L"v2", doc24->get(L"f4"));
    EXPECT_EQ(L"v2", doc223->get(L"f2"));
    EXPECT_EQ(L"v2", doc223->get(L"f3"));
}

TEST_F(ParallelReaderTest, testIncompatibleIndexes) {
    // two documents
    DirectoryPtr dir1 = getDir1();

    // one document only
    DirectoryPtr dir2 = newLucene<MockRAMDirectory>();
    IndexWriterPtr w2 = newLucene<IndexWriter>(dir2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr d3 = newLucene<Document>();
    d3->add(newLucene<Field>(L"f3", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
    w2->addDocument(d3);
    w2->close();

    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));

    try {
        pr->add(IndexReader::open(dir2, false));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
}

TEST_F(ParallelReaderTest, testIsCurrent) {
    DirectoryPtr dir1 = getDir1();
    DirectoryPtr dir2 = getDir2();
    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));

    EXPECT_TRUE(pr->isCurrent());
    IndexReaderPtr modifier = IndexReader::open(dir1, false);
    modifier->setNorm(0, L"f1", (uint8_t)100);
    modifier->close();

    // one of the two IndexReaders which ParallelReader is using is not current anymore
    EXPECT_TRUE(!pr->isCurrent());

    modifier = IndexReader::open(dir2, false);
    modifier->setNorm(0, L"f3", (uint8_t)100);
    modifier->close();

    // now both are not current anymore
    EXPECT_TRUE(!pr->isCurrent());
}

TEST_F(ParallelReaderTest, testIsOptimized) {
    DirectoryPtr dir1 = getDir1();
    DirectoryPtr dir2 = getDir2();

    // add another document to ensure that the indexes are not optimized
    IndexWriterPtr modifier = newLucene<IndexWriter>(dir1, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr d = newLucene<Document>();
    d->add(newLucene<Field>(L"f1", L"v1", Field::STORE_YES, Field::INDEX_ANALYZED));
    modifier->addDocument(d);
    modifier->close();

    modifier = newLucene<IndexWriter>(dir2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    d = newLucene<Document>();
    d->add(newLucene<Field>(L"f2", L"v2", Field::STORE_YES, Field::INDEX_ANALYZED));
    modifier->addDocument(d);
    modifier->close();

    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));
    EXPECT_TRUE(!pr->isOptimized());
    pr->close();

    modifier = newLucene<IndexWriter>(dir1, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    modifier->optimize();
    modifier->close();

    pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));
    // just one of the two indexes are optimized
    EXPECT_TRUE(!pr->isOptimized());
    pr->close();

    modifier = newLucene<IndexWriter>(dir2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), IndexWriter::MaxFieldLengthLIMITED);
    modifier->optimize();
    modifier->close();

    pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));
    // now both indexes are optimized
    EXPECT_TRUE(pr->isOptimized());
    pr->close();
}

TEST_F(ParallelReaderTest, testAllTermDocs) {
    DirectoryPtr dir1 = getDir1();
    DirectoryPtr dir2 = getDir2();

    ParallelReaderPtr pr = newLucene<ParallelReader>();
    pr->add(IndexReader::open(dir1, false));
    pr->add(IndexReader::open(dir2, false));
    int32_t NUM_DOCS = 2;
    TermDocsPtr td = pr->termDocs(TermPtr());
    for (int32_t i = 0; i < NUM_DOCS; ++i) {
        EXPECT_TRUE(td->next());
        EXPECT_EQ(i, td->doc());
        EXPECT_EQ(1, td->freq());
    }
    td->close();
    pr->close();
    dir1->close();
    dir2->close();
}
