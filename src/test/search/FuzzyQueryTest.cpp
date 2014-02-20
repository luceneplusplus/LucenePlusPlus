/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "FuzzyQuery.h"
#include "Term.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "BooleanQuery.h"
#include "StandardAnalyzer.h"
#include "QueryParser.h"
#include "IndexReader.h"

using namespace Lucene;

typedef LuceneTestFixture FuzzyQueryTest;

static void addDoc(const String& text, const IndexWriterPtr& writer) {
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", text, Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
}

TEST_F(FuzzyQueryTest, testFuzziness) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(L"aaaaa", writer);
    addDoc(L"aaaab", writer);
    addDoc(L"aaabb", writer);
    addDoc(L"aabbb", writer);
    addDoc(L"abbbb", writer);
    addDoc(L"bbbbb", writer);
    addDoc(L"ddddd", writer);
    writer->optimize();
    writer->close();
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    FuzzyQueryPtr query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 0);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());

    // same with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 3);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 4);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 5);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 6);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // test scoring
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"bbbbb"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    Collection<String> order = newCollection<String>(L"bbbbb", L"abbbb", L"aabbb");
    for (int32_t i = 0; i < hits.size(); ++i) {
        String term = searcher->doc(hits[i]->doc)->get(L"field");
        EXPECT_EQ(order[i], term);
    }

    // test BooleanQuery.maxClauseCount
    int32_t savedClauseCount = BooleanQuery::getMaxClauseCount();

    BooleanQuery::setMaxClauseCount(2);
    // This query would normally return 3 documents, because 3 terms match (see above)
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"bbbbb"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    order = newCollection<String>(L"bbbbb", L"abbbb");

    for (int32_t i = 0; i < hits.size(); ++i) {
        String term = searcher->doc(hits[i]->doc)->get(L"field");
        EXPECT_EQ(order[i], term);
    }

    BooleanQuery::setMaxClauseCount(savedClauseCount);

    // not similar enough
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"xxxxx"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaccc"), FuzzyQuery::defaultMinSimilarity(), 0); // edit distance to "aaaaa" = 3
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // query identical to a word in the index
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaa"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    // default allows for up to two edits
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    EXPECT_EQ(searcher->doc(hits[2]->doc)->get(L"field"), L"aaabb");

    // query similar to a word in the index
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    EXPECT_EQ(searcher->doc(hits[2]->doc)->get(L"field"), L"aaabb");

    // now with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    EXPECT_EQ(searcher->doc(hits[2]->doc)->get(L"field"), L"aaabb");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    EXPECT_EQ(searcher->doc(hits[2]->doc)->get(L"field"), L"aaabb");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 3);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    EXPECT_EQ(searcher->doc(hits[2]->doc)->get(L"field"), L"aaabb");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 4);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaa");
    EXPECT_EQ(searcher->doc(hits[1]->doc)->get(L"field"), L"aaaab");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaac"), FuzzyQuery::defaultMinSimilarity(), 5);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"ddddd");

    // now with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"ddddd");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"ddddd");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 3);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"ddddd");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 4);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"ddddd");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 5);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // different field = no match
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"anotherfield", L"ddddX"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    searcher->close();
    directory->close();
}

TEST_F(FuzzyQueryTest, testFuzzinessLong) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(L"aaaaaaa", writer);
    addDoc(L"segment", writer);
    writer->optimize();
    writer->close();
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    // not similar enough
    FuzzyQueryPtr query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"xxxxx"), FuzzyQuery::defaultMinSimilarity(), 0);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    // edit distance to "aaaaaaa" = 3, this matches because the string is longer than
    // in testDefaultFuzziness so a bigger difference is allowed
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaccc"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaaaa");

    // now with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaccc"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaaaa");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaccc"), FuzzyQuery::defaultMinSimilarity(), 4);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(searcher->doc(hits[0]->doc)->get(L"field"), L"aaaaaaa");
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaaaccc"), FuzzyQuery::defaultMinSimilarity(), 5);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // no match, more than half of the characters is wrong
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaacccc"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // now with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"aaacccc"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // "student" and "stellent" are indeed similar to "segment" by default
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"stellent"), FuzzyQuery::defaultMinSimilarity(), 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // now with prefix
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"stellent"), FuzzyQuery::defaultMinSimilarity(), 1);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"stellent"), FuzzyQuery::defaultMinSimilarity(), 2);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // "student" doesn't match anymore thanks to increased minimum similarity
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), 0.6, 0);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    try {
        query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), 1.1);
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
    try {
        query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"student"), -0.1);
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }

    searcher->close();
    directory->close();
}

TEST_F(FuzzyQueryTest, testTokenLengthOpt) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDoc(L"12345678911", writer);
    addDoc(L"segment", writer);
    writer->optimize();
    writer->close();
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    // term not over 10 chars, so optimization shortcuts
    FuzzyQueryPtr query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"1234569"), 0.9);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // 10 chars, so no optimization
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"1234567891"), 0.9);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // over 10 chars, so no optimization
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"12345678911"), 0.9);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // over 10 chars, no match
    query = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"sdfsdfsdfsdf"), 0.9);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
}

TEST_F(FuzzyQueryTest, testGiga) {
    StandardAnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    DirectoryPtr index = newLucene<MockRAMDirectory>();
    IndexWriterPtr w = newLucene<IndexWriter>(index, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);

    addDoc(L"Lucene in Action", w);
    addDoc(L"Lucene for Dummies", w);

    addDoc(L"Giga byte", w);

    addDoc(L"ManagingGigabytesManagingGigabyte", w);
    addDoc(L"ManagingGigabytesManagingGigabytes", w);

    addDoc(L"The Art of Computer Science", w);
    addDoc(L"J. K. Rowling", w);
    addDoc(L"JK Rowling", w);
    addDoc(L"Joanne K Roling", w);
    addDoc(L"Bruce Willis", w);
    addDoc(L"Willis bruce", w);
    addDoc(L"Brute willis", w);
    addDoc(L"B. willis", w);
    IndexReaderPtr r = w->getReader();
    w->close();

    QueryPtr q = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", analyzer)->parse(L"giga~0.9");

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(r);
    Collection<ScoreDocPtr> hits = searcher->search(q, 10)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    EXPECT_EQ(L"Giga byte", searcher->doc(hits[0]->doc)->get(L"field"));
    r->close();
}
