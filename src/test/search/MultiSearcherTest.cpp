/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "Document.h"
#include "Field.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "QueryParser.h"
#include "Query.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MultiSearcher.h"
#include "Term.h"
#include "IndexReader.h"
#include "TermQuery.h"
#include "SetBasedFieldSelector.h"
#include "KeywordAnalyzer.h"
#include "Sort.h"
#include "DefaultSimilarity.h"
#include "TopFieldDocs.h"

using namespace Lucene;

typedef LuceneTestFixture MultiSearcherTest;

static MultiSearcherPtr getMultiSearcherInstance(Collection<SearchablePtr> searchers) {
    return newLucene<MultiSearcher>(searchers);
}

static DocumentPtr createDocument(const String& contents1, const String& contents2) {
    DocumentPtr document = newLucene<Document>();
    document->add(newLucene<Field>(L"contents", contents1, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    document->add(newLucene<Field>(L"other", L"other contents", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    if (!contents2.empty()) {
        document->add(newLucene<Field>(L"contents", contents2, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    }
    return document;
}

static void initIndex(const DirectoryPtr& directory, int32_t numDocs, bool create, const String& contents2) {
    IndexWriterPtr indexWriter = newLucene<IndexWriter>(directory, newLucene<KeywordAnalyzer>(), create, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < numDocs; ++i) {
        indexWriter->addDocument(createDocument(L"doc" + StringUtils::toString(i), contents2));
    }
    indexWriter->close();
}

TEST_F(MultiSearcherTest, testEmptyIndex) {
    // creating two directories for indices
    DirectoryPtr indexStoreA = newLucene<MockRAMDirectory>();
    DirectoryPtr indexStoreB = newLucene<MockRAMDirectory>();

    // creating a document to store
    DocumentPtr lDoc = newLucene<Document>();
    lDoc->add(newLucene<Field>(L"fulltext", L"Once upon a time.....", Field::STORE_YES, Field::INDEX_ANALYZED));
    lDoc->add(newLucene<Field>(L"id", L"doc1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    lDoc->add(newLucene<Field>(L"handle", L"1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // creating a document to store
    DocumentPtr lDoc2 = newLucene<Document>();
    lDoc2->add(newLucene<Field>(L"fulltext", L"in a galaxy far far away.....", Field::STORE_YES, Field::INDEX_ANALYZED));
    lDoc2->add(newLucene<Field>(L"id", L"doc2", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    lDoc2->add(newLucene<Field>(L"handle", L"1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // creating a document to store
    DocumentPtr lDoc3 = newLucene<Document>();
    lDoc3->add(newLucene<Field>(L"fulltext", L"a bizarre bug manifested itself....", Field::STORE_YES, Field::INDEX_ANALYZED));
    lDoc3->add(newLucene<Field>(L"id", L"doc3", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    lDoc3->add(newLucene<Field>(L"handle", L"1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // creating an index writer for the first index
    IndexWriterPtr writerA = newLucene<IndexWriter>(indexStoreA, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
    // creating an index writer for the second index, but writing nothing
    IndexWriterPtr writerB = newLucene<IndexWriter>(indexStoreB, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

    //--------------------------------------------------------------------
    // scenario 1
    //--------------------------------------------------------------------

    // writing the documents to the first index
    writerA->addDocument(lDoc);
    writerA->addDocument(lDoc2);
    writerA->addDocument(lDoc3);
    writerA->optimize();
    writerA->close();

    // closing the second index
    writerB->close();

    // creating the query
    QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"fulltext", newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    QueryPtr query = parser->parse(L"handle:1");

    // building the searchables
    Collection<SearchablePtr> searchers = newCollection<SearchablePtr>(newLucene<IndexSearcher>(indexStoreB, true), newLucene<IndexSearcher>(indexStoreA, true));
    // creating the multiSearcher
    SearcherPtr mSearcher = getMultiSearcherInstance(searchers);
    // performing the search
    Collection<ScoreDocPtr> hits = mSearcher->search(query, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(3, hits.size());

    // iterating over the hit documents
    for (int32_t i = 0; i < hits.size(); ++i) {
        mSearcher->doc(hits[i]->doc);
    }
    mSearcher->close();


    //--------------------------------------------------------------------
    // scenario 2
    //--------------------------------------------------------------------

    // adding one document to the empty index
    writerB = newLucene<IndexWriter>(indexStoreB, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writerB->addDocument(lDoc);
    writerB->optimize();
    writerB->close();

    // building the searchables
    Collection<SearchablePtr> searchers2 = newCollection<SearchablePtr>(newLucene<IndexSearcher>(indexStoreB, true), newLucene<IndexSearcher>(indexStoreA, true));
    // creating the mulitSearcher
    MultiSearcherPtr mSearcher2 = getMultiSearcherInstance(searchers2);
    // performing the same search
    Collection<ScoreDocPtr> hits2 = mSearcher2->search(query, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(4, hits2.size());

    // iterating over the hit documents
    for (int32_t i = 0; i < hits2.size(); ++i) {
        mSearcher2->doc(hits2[i]->doc);
    }

    // test the subSearcher() method
    QueryPtr subSearcherQuery = parser->parse(L"id:doc1");
    hits2 = mSearcher2->search(subSearcherQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits2.size());
    EXPECT_EQ(0, mSearcher2->subSearcher(hits2[0]->doc)); // hit from searchers2[0]
    EXPECT_EQ(1, mSearcher2->subSearcher(hits2[1]->doc)); // hit from searchers2[1]
    subSearcherQuery = parser->parse(L"id:doc2");
    hits2 = mSearcher2->search(subSearcherQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits2.size());
    EXPECT_EQ(1, mSearcher2->subSearcher(hits2[0]->doc));   // hit from searchers2[1]
    mSearcher2->close();

    //--------------------------------------------------------------------
    // scenario 3
    //--------------------------------------------------------------------

    // deleting the document just added, this will cause a different exception to take place
    TermPtr term = newLucene<Term>(L"id", L"doc1");
    IndexReaderPtr readerB = IndexReader::open(indexStoreB, false);
    readerB->deleteDocuments(term);
    readerB->close();

    // optimizing the index with the writer
    writerB = newLucene<IndexWriter>(indexStoreB, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), false, IndexWriter::MaxFieldLengthLIMITED);
    writerB->optimize();
    writerB->close();

    // building the searchables
    Collection<SearchablePtr> searchers3 = newCollection<SearchablePtr>(newLucene<IndexSearcher>(indexStoreB, true), newLucene<IndexSearcher>(indexStoreA, true));
    // creating the mulitSearcher
    SearcherPtr mSearcher3 = getMultiSearcherInstance(searchers3);
    // performing the same search
    Collection<ScoreDocPtr> hits3 = mSearcher3->search(query, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(3, hits3.size());


    // iterating over the hit documents
    for (int32_t i = 0; i < hits3.size(); ++i) {
        mSearcher3->doc(hits3[i]->doc);
    }

    mSearcher3->close();
    indexStoreA->close();
    indexStoreB->close();
}

TEST_F(MultiSearcherTest, testFieldSelector) {
    RAMDirectoryPtr ramDirectory1 = newLucene<RAMDirectory>();
    RAMDirectoryPtr ramDirectory2 = newLucene<RAMDirectory>();

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"contents", L"doc0"));

    // Now put the documents in a different index
    initIndex(ramDirectory1, 10, true, L""); // documents with a single token "doc0", "doc1", etc...
    initIndex(ramDirectory2, 10, true, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...

    IndexSearcherPtr indexSearcher1 = newLucene<IndexSearcher>(ramDirectory1, true);
    IndexSearcherPtr indexSearcher2 = newLucene<IndexSearcher>(ramDirectory2, true);

    MultiSearcherPtr searcher = getMultiSearcherInstance(newCollection<SearchablePtr>(indexSearcher1, indexSearcher2));
    EXPECT_TRUE(searcher);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits.size(), 2);
    DocumentPtr document = searcher->doc(hits[0]->doc);
    EXPECT_TRUE(document);
    EXPECT_EQ(document->getFields().size(), 2);
    // Should be one document from each directory they both have two fields, contents and other
    HashSet<String> ftl = HashSet<String>::newInstance();
    ftl.add(L"other");
    SetBasedFieldSelectorPtr fs = newLucene<SetBasedFieldSelector>(ftl, HashSet<String>::newInstance());
    document = searcher->doc(hits[0]->doc, fs);
    EXPECT_TRUE(document);
    EXPECT_EQ(document->getFields().size(), 1);
    String value = document->get(L"contents");
    EXPECT_TRUE(value.empty());
    value = document->get(L"other");
    EXPECT_TRUE(!value.empty());
    ftl.clear();
    ftl.add(L"contents");
    fs = newLucene<SetBasedFieldSelector>(ftl, HashSet<String>::newInstance());
    document = searcher->doc(hits[1]->doc, fs);
    value = document->get(L"contents");
    EXPECT_TRUE(!value.empty());
    value = document->get(L"other");
    EXPECT_TRUE(value.empty());
}

TEST_F(MultiSearcherTest, testNormalization) {
    int32_t numDocs = 10;

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"contents", L"doc0"));
    RAMDirectoryPtr ramDirectory1 = newLucene<MockRAMDirectory>();

    // First put the documents in the same index
    initIndex(ramDirectory1, numDocs, true, L""); // documents with a single token "doc0", "doc1", etc...
    initIndex(ramDirectory1, numDocs, false, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...

    IndexSearcherPtr indexSearcher1 = newLucene<IndexSearcher>(ramDirectory1, true);
    indexSearcher1->setDefaultFieldSortScoring(true, true);

    Collection<ScoreDocPtr> hits = indexSearcher1->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());

    // Store the scores for use later
    Collection<double> scores = newCollection<double>(hits[0]->score, hits[1]->score);
    EXPECT_TRUE(scores[0] > scores[1]);

    indexSearcher1->close();
    ramDirectory1->close();
    hits.clear();

    ramDirectory1 = newLucene<MockRAMDirectory>();
    RAMDirectoryPtr ramDirectory2 = newLucene<MockRAMDirectory>();

    // Now put the documents in a different index
    initIndex(ramDirectory1, numDocs, true, L""); // documents with a single token "doc0", "doc1", etc...
    initIndex(ramDirectory2, numDocs, true, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...

    indexSearcher1 = newLucene<IndexSearcher>(ramDirectory1, true);
    indexSearcher1->setDefaultFieldSortScoring(true, true);
    IndexSearcherPtr indexSearcher2 = newLucene<IndexSearcher>(ramDirectory2, true);
    indexSearcher2->setDefaultFieldSortScoring(true, true);

    SearcherPtr searcher = getMultiSearcherInstance(newCollection<SearchablePtr>(indexSearcher1, indexSearcher2));

    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;

    EXPECT_EQ(2, hits.size());

    // The scores should be the same (within reason)
    EXPECT_NEAR(scores[0], hits[0]->score, 1e-6); // This will a document from ramDirectory1
    EXPECT_NEAR(scores[1], hits[1]->score, 1e-6); // This will a document from ramDirectory2

    // Adding a Sort.RELEVANCE object should not change anything
    hits = searcher->search(query, FilterPtr(), 1000, Sort::RELEVANCE())->scoreDocs;

    EXPECT_EQ(2, hits.size());

    EXPECT_NEAR(scores[0], hits[0]->score, 1e-6); // This will a document from ramDirectory1
    EXPECT_NEAR(scores[1], hits[1]->score, 1e-6); // This will a document from ramDirectory2

    searcher->close();

    ramDirectory1->close();
    ramDirectory2->close();
}

namespace TestCustomSimilarity {

class CustomSimilarity : public DefaultSimilarity {
public:
    virtual ~CustomSimilarity() {
    }

public:
    virtual double idf(int32_t docFreq, int32_t numDocs) {
        return 100.0;
    }

    virtual double coord(int32_t overlap, int32_t maxOverlap) {
        return 1.0;
    }

    virtual double lengthNorm(const String& fieldName, int32_t numTokens) {
        return 1.0;
    }

    virtual double queryNorm(double sumOfSquaredWeights) {
        return 1.0;
    }

    virtual double sloppyFreq(int32_t distance) {
        return 1.0;
    }

    virtual double tf(double freq) {
        return 1.0;
    }
};

}

TEST_F(MultiSearcherTest, testCustomSimilarity) {
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    initIndex(dir, 10, true, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    MultiSearcherPtr msearcher = getMultiSearcherInstance(newCollection<SearchablePtr>(searcher));
    SimilarityPtr customSimilarity = newLucene<TestCustomSimilarity::CustomSimilarity>();

    searcher->setSimilarity(customSimilarity);
    msearcher->setSimilarity(customSimilarity);

    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"contents", L"doc0"));

    // Get a score from IndexSearcher
    TopDocsPtr topDocs = searcher->search(query, FilterPtr(), 1);
    double score1 = topDocs->maxScore;

    // Get the score from MultiSearcher
    topDocs = msearcher->search(query, FilterPtr(), 1);
    double scoreN = topDocs->maxScore;

    // The scores from the IndexSearcher and Multisearcher should be the same if the same similarity is used.
    EXPECT_NEAR(score1, scoreN, 1e-6);
}

TEST_F(MultiSearcherTest, testDocFreq) {
    RAMDirectoryPtr dir1 = newLucene<RAMDirectory>();
    RAMDirectoryPtr dir2 = newLucene<RAMDirectory>();

    initIndex(dir1, 10, true, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...
    initIndex(dir2, 5, true, L"x"); // documents with two tokens "doc0" and "x", "doc1" and x, etc...
    IndexSearcherPtr searcher1 = newLucene<IndexSearcher>(dir1, true);
    IndexSearcherPtr searcher2 = newLucene<IndexSearcher>(dir2, true);

    MultiSearcherPtr multiSearcher = getMultiSearcherInstance(newCollection<SearchablePtr>(searcher1, searcher2));
    EXPECT_EQ(15, multiSearcher->docFreq(newLucene<Term>(L"contents", L"x")));
}
