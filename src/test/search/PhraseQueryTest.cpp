/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "Analyzer.h"
#include "WhitespaceTokenizer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "PhraseQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "QueryUtils.h"
#include "Term.h"
#include "StopAnalyzer.h"
#include "WhitespaceAnalyzer.h"
#include "TermQuery.h"
#include "BooleanQuery.h"
#include "QueryParser.h"

using namespace Lucene;

class PhraseQueryAnalyzer : public Analyzer {
public:
    virtual ~PhraseQueryAnalyzer() {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<WhitespaceTokenizer>(reader);
    }

    virtual int32_t getPositionIncrementGap(const String& fieldName) {
        return 100;
    }
};

class PhraseQueryTest : public LuceneTestFixture {
public:
    PhraseQueryTest() {
        directory = newLucene<RAMDirectory>();
        AnalyzerPtr analyzer = newLucene<PhraseQueryAnalyzer>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"field", L"one two three four five", Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(L"repeated", L"this is a repeated field - first part", Field::STORE_YES, Field::INDEX_ANALYZED));
        FieldablePtr repeatedField = newLucene<Field>(L"repeated", L"second part of a repeated field", Field::STORE_YES, Field::INDEX_ANALYZED);
        doc->add(repeatedField);
        doc->add(newLucene<Field>(L"palindrome", L"one two three two one", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"nonexist", L"phrase exist notexist exist found", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"nonexist", L"phrase exist notexist exist found", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        writer->optimize();
        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
        query = newLucene<PhraseQuery>();
    }

    virtual ~PhraseQueryTest() {
        searcher->close();
        directory->close();
    }

public:
    // threshold for comparing floats
    static const double SCORE_COMP_THRESH;

protected:
    IndexSearcherPtr searcher;
    PhraseQueryPtr query;
    RAMDirectoryPtr directory;
};

const double PhraseQueryTest::SCORE_COMP_THRESH = 1e-6f;

TEST_F(PhraseQueryTest, testNotCloseEnough) {
    query->setSlop(2);
    query->add(newLucene<Term>(L"field", L"one"));
    query->add(newLucene<Term>(L"field", L"five"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

TEST_F(PhraseQueryTest, testBarelyCloseEnough) {
    query->setSlop(3);
    query->add(newLucene<Term>(L"field", L"one"));
    query->add(newLucene<Term>(L"field", L"five"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);
}

/// Ensures slop of 0 works for exact matches, but not reversed
TEST_F(PhraseQueryTest, testExact) {
    // slop is zero by default
    query->add(newLucene<Term>(L"field", L"four"));
    query->add(newLucene<Term>(L"field", L"five"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"two"));
    query->add(newLucene<Term>(L"field", L"one"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

TEST_F(PhraseQueryTest, testSlop1) {
    // Ensures slop of 1 works with terms in order.
    query->setSlop(1);
    query->add(newLucene<Term>(L"field", L"one"));
    query->add(newLucene<Term>(L"field", L"two"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    // Ensures slop of 1 does not work for phrases out of order; must be at least 2
    query = newLucene<PhraseQuery>();
    query->setSlop(1);
    query->add(newLucene<Term>(L"field", L"two"));
    query->add(newLucene<Term>(L"field", L"one"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

/// As long as slop is at least 2, terms can be reversed
TEST_F(PhraseQueryTest, testOrderDoesntMatter) {
    // must be at least two for reverse order match
    query->setSlop(2);
    query->add(newLucene<Term>(L"field", L"two"));
    query->add(newLucene<Term>(L"field", L"one"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    query = newLucene<PhraseQuery>();
    query->setSlop(2);
    query->add(newLucene<Term>(L"field", L"three"));
    query->add(newLucene<Term>(L"field", L"one"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

/// Slop is the total number of positional moves allowed to line up a phrase
TEST_F(PhraseQueryTest, testMulipleTerms) {
    query->setSlop(2);
    query->add(newLucene<Term>(L"field", L"one"));
    query->add(newLucene<Term>(L"field", L"three"));
    query->add(newLucene<Term>(L"field", L"five"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    query = newLucene<PhraseQuery>();
    query->setSlop(5); // it takes six moves to match this phrase
    query->add(newLucene<Term>(L"field", L"five"));
    query->add(newLucene<Term>(L"field", L"three"));
    query->add(newLucene<Term>(L"field", L"one"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);

    query->setSlop(6);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);
}

TEST_F(PhraseQueryTest, testPhraseQueryWithStopAnalyzer) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    StopAnalyzerPtr stopAnalyzer = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_24);
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, stopAnalyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"the stop words are here", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    // valid exact phrase query
    PhraseQueryPtr query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"stop"));
    query->add(newLucene<Term>(L"field", L"words"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    // StopAnalyzer as of 2.4 does not leave "holes", so this matches.
    query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"words"));
    query->add(newLucene<Term>(L"field", L"here"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    searcher->close();
}

TEST_F(PhraseQueryTest, testPhraseQueryInConjunctionScorer) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"source", L"marketing info", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"contents", L"foobar", Field::STORE_YES, Field::INDEX_ANALYZED));
    doc->add(newLucene<Field>(L"source", L"marketing info", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    PhraseQueryPtr phraseQuery = newLucene<PhraseQuery>();
    phraseQuery->add(newLucene<Term>(L"source", L"marketing"));
    phraseQuery->add(newLucene<Term>(L"source", L"info"));
    Collection<ScoreDocPtr> hits = searcher->search(phraseQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    QueryUtils::check(phraseQuery, searcher);

    TermQueryPtr termQuery = newLucene<TermQuery>(newLucene<Term>(L"contents", L"foobar"));
    BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(termQuery, BooleanClause::MUST);
    booleanQuery->add(phraseQuery, BooleanClause::MUST);
    hits = searcher->search(booleanQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(termQuery, searcher);

    searcher->close();

    writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"contents", L"map entry woo", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"contents", L"woo map entry", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"contents", L"map foobarword entry woo", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    writer->optimize();
    writer->close();

    searcher = newLucene<IndexSearcher>(directory, true);

    termQuery = newLucene<TermQuery>(newLucene<Term>(L"contents", L"woo"));
    phraseQuery = newLucene<PhraseQuery>();
    phraseQuery->add(newLucene<Term>(L"contents", L"map"));
    phraseQuery->add(newLucene<Term>(L"contents", L"entry"));

    hits = searcher->search(termQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    hits = searcher->search(phraseQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());

    booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(termQuery, BooleanClause::MUST);
    booleanQuery->add(phraseQuery, BooleanClause::MUST);
    hits = searcher->search(booleanQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());

    booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(phraseQuery, BooleanClause::MUST);
    booleanQuery->add(termQuery, BooleanClause::MUST);
    hits = searcher->search(booleanQuery, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    QueryUtils::check(booleanQuery, searcher);

    searcher->close();
    directory->close();
}

TEST_F(PhraseQueryTest, testSlopScoring) {
    DirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"field", L"foo firstname lastname foo", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);

    DocumentPtr doc2 = newLucene<Document>();
    doc2->add(newLucene<Field>(L"field", L"foo firstname xxx lastname foo", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc2);

    DocumentPtr doc3 = newLucene<Document>();
    doc3->add(newLucene<Field>(L"field", L"foo firstname xxx yyy lastname foo", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc3);

    writer->optimize();
    writer->close();

    SearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
    PhraseQueryPtr query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"field", L"firstname"));
    query->add(newLucene<Term>(L"field", L"lastname"));
    query->setSlop(INT_MAX);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    // Make sure that those matches where the terms appear closer to each other get a higher score
    EXPECT_NEAR(0.71, hits[0]->score, 0.01);
    EXPECT_EQ(0, hits[0]->doc);
    EXPECT_NEAR(0.44, hits[1]->score, 0.01);
    EXPECT_EQ(1, hits[1]->doc);
    EXPECT_NEAR(0.31, hits[2]->score, 0.01);
    EXPECT_EQ(2, hits[2]->doc);
    QueryUtils::check(query, searcher);
}

TEST_F(PhraseQueryTest, testToString) {
    StopAnalyzerPtr analyzer = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", analyzer);
    qp->setEnablePositionIncrements(true);
    PhraseQueryPtr q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"this hi this is a test is\""));
    EXPECT_EQ(L"field:\"? hi ? ? ? test\"", q->toString());
    q->add(newLucene<Term>(L"field", L"hello"), 1);
    EXPECT_EQ(L"field:\"? hi|hello ? ? ? test\"", q->toString());
}

TEST_F(PhraseQueryTest, testWrappedPhrase) {
    query->add(newLucene<Term>(L"repeated", L"first"));
    query->add(newLucene<Term>(L"repeated", L"part"));
    query->add(newLucene<Term>(L"repeated", L"second"));
    query->add(newLucene<Term>(L"repeated", L"part"));
    query->setSlop(100);

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    query->setSlop(99);

    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

/// work on two docs like this: "phrase exist notexist exist found"
TEST_F(PhraseQueryTest, testNonExistingPhrase) {
    // phrase without repetitions that exists in 2 docs
    query->add(newLucene<Term>(L"nonexist", L"phrase"));
    query->add(newLucene<Term>(L"nonexist", L"notexist"));
    query->add(newLucene<Term>(L"nonexist", L"found"));
    query->setSlop(2); // would be found this way

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    QueryUtils::check(query, searcher);

    // phrase with repetitions that exists in 2 docs
    query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"nonexist", L"phrase"));
    query->add(newLucene<Term>(L"nonexist", L"exist"));
    query->add(newLucene<Term>(L"nonexist", L"exist"));
    query->setSlop(1); // would be found

    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    QueryUtils::check(query, searcher);

    // phrase I with repetitions that does not exist in any doc
    query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"nonexist", L"phrase"));
    query->add(newLucene<Term>(L"nonexist", L"notexist"));
    query->add(newLucene<Term>(L"nonexist", L"phrase"));
    query->setSlop(1000); // would not be found no matter how high the slop is

    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);

    // phrase II with repetitions that does not exist in any doc
    query = newLucene<PhraseQuery>();
    query->add(newLucene<Term>(L"nonexist", L"phrase"));
    query->add(newLucene<Term>(L"nonexist", L"exist"));
    query->add(newLucene<Term>(L"nonexist", L"exist"));
    query->add(newLucene<Term>(L"nonexist", L"exist"));
    query->setSlop(1000); // would not be found no matter how high the slop is

    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());
    QueryUtils::check(query, searcher);
}

/// Working on a 2 fields like this:
///    Field(L"field", L"one two three four five")
///    Field(L"palindrome", L"one two three two one")
/// Phrase of size 2 occurring twice, once in order and once in reverse, because doc is a palindrome, is counted twice.
/// Also, in this case order in query does not matter.  Also, when an exact match is found, both sloppy scorer and
/// exact scorer scores the same.
TEST_F(PhraseQueryTest, testPalindrome2) {
    // search on non palindrome, find phrase with no slop, using exact phrase scorer
    query->setSlop(0); // to use exact phrase scorer
    query->add(newLucene<Term>(L"field", L"two"));
    query->add(newLucene<Term>(L"field", L"three"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    double score0 = hits[0]->score;
    QueryUtils::check(query, searcher);

    // search on non palindrome, find phrase with slop 2, though no slop required here.
    query->setSlop(2); // to use sloppy scorer
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    double score1 = hits[0]->score;
    EXPECT_NEAR(score0, score1, SCORE_COMP_THRESH);
    QueryUtils::check(query, searcher);

    // search ordered in palindrome, find it twice
    query = newLucene<PhraseQuery>();
    query->setSlop(2); // must be at least two for both ordered and reversed to match
    query->add(newLucene<Term>(L"palindrome", L"two"));
    query->add(newLucene<Term>(L"palindrome", L"three"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    // search reversed in palindrome, find it twice
    query = newLucene<PhraseQuery>();
    query->setSlop(2); // must be at least two for both ordered and reversed to match
    query->add(newLucene<Term>(L"palindrome", L"three"));
    query->add(newLucene<Term>(L"palindrome", L"two"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);
}

/// Working on a 2 fields like this:
///    Field(L"field", L"one two three four five")
///    Field(L"palindrome", L"one two three two one")
/// Phrase of size 3 occurring twice, once in order and once in reverse, because doc is a palindrome, is counted twice.
/// Also, in this case order in query does not matter.  Also, when an exact match is found, both sloppy scorer and exact
/// scorer scores the same.
TEST_F(PhraseQueryTest, testPalindrome3) {
    // search on non palindrome, find phrase with no slop, using exact phrase scorer
    query->setSlop(0); // to use exact phrase scorer
    query->add(newLucene<Term>(L"field", L"one"));
    query->add(newLucene<Term>(L"field", L"two"));
    query->add(newLucene<Term>(L"field", L"three"));
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    double score0 = hits[0]->score;
    QueryUtils::check(query, searcher);

    // search on non palindrome, find phrase with slop 3, though no slop required here.
    query->setSlop(4); // to use sloppy scorer
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    double score1 = hits[0]->score;
    EXPECT_NEAR(score0, score1, SCORE_COMP_THRESH);
    QueryUtils::check(query, searcher);

    // search ordered in palindrome, find it twice
    query = newLucene<PhraseQuery>();
    query->setSlop(4); // must be at least four for both ordered and reversed to match
    query->add(newLucene<Term>(L"palindrome", L"one"));
    query->add(newLucene<Term>(L"palindrome", L"two"));
    query->add(newLucene<Term>(L"palindrome", L"three"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);

    // search reversed in palindrome, find it twice
    query = newLucene<PhraseQuery>();
    query->setSlop(4); // must be at least four for both ordered and reversed to match
    query->add(newLucene<Term>(L"palindrome", L"three"));
    query->add(newLucene<Term>(L"palindrome", L"two"));
    query->add(newLucene<Term>(L"palindrome", L"one"));
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    QueryUtils::check(query, searcher);
}

TEST_F(PhraseQueryTest, testEmptyPhraseQuery) {
    BooleanQueryPtr q2 = newLucene<BooleanQuery>();
    q2->add(newLucene<PhraseQuery>(), BooleanClause::MUST);
    EXPECT_EQ(q2->toString(), L"+\"?\"");
}
