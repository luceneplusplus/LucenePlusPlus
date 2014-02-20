/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "WildcardQuery.h"
#include "Term.h"
#include "FuzzyQuery.h"
#include "IndexSearcher.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TermQuery.h"
#include "ConstantScoreQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "BooleanQuery.h"
#include "PrefixQuery.h"
#include "QueryParser.h"
#include "WhitespaceAnalyzer.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture WildcardTest;

static RAMDirectoryPtr getIndexStore(const String& field, Collection<String> contents) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < contents.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(field, contents[i], Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->optimize();
    writer->close();

    return indexStore;
}

static void checkMatches(const IndexSearcherPtr& searcher, const QueryPtr& q, int32_t expectedMatches) {
    Collection<ScoreDocPtr> result = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(expectedMatches, result.size());
}

TEST_F(WildcardTest, testEquals) {
    WildcardQueryPtr wq1 = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"b*a"));
    WildcardQueryPtr wq2 = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"b*a"));
    WildcardQueryPtr wq3 = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"b*a"));

    // reflexive?
    EXPECT_TRUE(wq1->equals(wq2));
    EXPECT_TRUE(wq2->equals(wq1));

    // transitive?
    EXPECT_TRUE(wq2->equals(wq3));
    EXPECT_TRUE(wq1->equals(wq3));

    EXPECT_TRUE(!wq1->equals(WildcardQueryPtr()));

    FuzzyQueryPtr fq = newLucene<FuzzyQuery>(newLucene<Term>(L"field", L"b*a"));
    EXPECT_TRUE(!wq1->equals(fq));
    EXPECT_TRUE(!fq->equals(wq1));
}

/// Tests if a WildcardQuery that has no wildcard in the term is rewritten to a single TermQuery.
/// The boost should be preserved, and the rewrite should return a ConstantScoreQuery if the
/// WildcardQuery had a ConstantScore rewriteMethod.
TEST_F(WildcardTest, testTermWithoutWildcard) {
    RAMDirectoryPtr indexStore = getIndexStore(L"field", newCollection<String>(L"nowildcard", L"nowildcardx"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    MultiTermQueryPtr wq = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"nowildcard"));
    checkMatches(searcher, wq, 1);

    wq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
    wq->setBoost(0.1);
    QueryPtr q = searcher->rewrite(wq);
    EXPECT_TRUE(MiscUtils::typeOf<TermQuery>(q));
    EXPECT_EQ(q->getBoost(), wq->getBoost());

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
    wq->setBoost(0.2);
    q = searcher->rewrite(wq);
    EXPECT_TRUE(MiscUtils::typeOf<ConstantScoreQuery>(q));
    EXPECT_EQ(q->getBoost(), wq->getBoost());

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT());
    wq->setBoost(0.3);
    q = searcher->rewrite(wq);
    EXPECT_TRUE(MiscUtils::typeOf<ConstantScoreQuery>(q));
    EXPECT_EQ(q->getBoost(), wq->getBoost());

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE());
    wq->setBoost(0.4);
    q = searcher->rewrite(wq);
    EXPECT_TRUE(MiscUtils::typeOf<ConstantScoreQuery>(q));
    EXPECT_EQ(q->getBoost(), wq->getBoost());
}

/// Tests if a WildcardQuery with an empty term is rewritten to an empty BooleanQuery
TEST_F(WildcardTest, testEmptyTerm) {
    RAMDirectoryPtr indexStore = getIndexStore(L"field", newCollection<String>(L"nowildcard", L"nowildcardx"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    MultiTermQueryPtr wq = newLucene<WildcardQuery>(newLucene<Term>(L"field", L""));
    wq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
    checkMatches(searcher, wq, 0);
    BooleanQueryPtr expected = newLucene<BooleanQuery>(true);
    EXPECT_TRUE(searcher->rewrite(expected)->equals(searcher->rewrite(wq)));
}

/// Tests if a WildcardQuery that has only a trailing * in the term is rewritten to a
/// single PrefixQuery. The boost and rewriteMethod should be preserved.
TEST_F(WildcardTest, testPrefixTerm) {
    RAMDirectoryPtr indexStore = getIndexStore(L"field", newCollection<String>(L"prefix", L"prefixx"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);

    MultiTermQueryPtr wq = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"prefix*"));
    checkMatches(searcher, wq, 2);

    MultiTermQueryPtr expected = newLucene<PrefixQuery>(newLucene<Term>(L"field", L"prefix"));
    wq->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
    wq->setBoost(0.1);
    expected->setRewriteMethod(wq->getRewriteMethod());
    expected->setBoost(wq->getBoost());
    EXPECT_TRUE(searcher->rewrite(expected)->equals(searcher->rewrite(wq)));

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
    wq->setBoost(0.2);
    expected->setRewriteMethod(wq->getRewriteMethod());
    expected->setBoost(wq->getBoost());
    EXPECT_TRUE(searcher->rewrite(expected)->equals(searcher->rewrite(wq)));

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT());
    wq->setBoost(0.3);
    expected->setRewriteMethod(wq->getRewriteMethod());
    expected->setBoost(wq->getBoost());
    EXPECT_TRUE(searcher->rewrite(expected)->equals(searcher->rewrite(wq)));

    wq->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE());
    wq->setBoost(0.4);
    expected->setRewriteMethod(wq->getRewriteMethod());
    expected->setBoost(wq->getBoost());
    EXPECT_TRUE(searcher->rewrite(expected)->equals(searcher->rewrite(wq)));
}

TEST_F(WildcardTest, testAsterisk) {
    RAMDirectoryPtr indexStore = getIndexStore(L"body", newCollection<String>(L"metal", L"metals"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);
    QueryPtr query1 = newLucene<TermQuery>(newLucene<Term>(L"body", L"metal"));
    QueryPtr query2 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"metal*"));
    QueryPtr query3 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"m*tal"));
    QueryPtr query4 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"m*tal*"));
    QueryPtr query5 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"m*tals"));

    BooleanQueryPtr query6 = newLucene<BooleanQuery>();
    query6->add(query5, BooleanClause::SHOULD);

    BooleanQueryPtr query7 = newLucene<BooleanQuery>();
    query7->add(query3, BooleanClause::SHOULD);
    query7->add(query5, BooleanClause::SHOULD);

    // Queries do not automatically lower-case search terms:
    QueryPtr query8 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"M*tal*"));

    checkMatches(searcher, query1, 1);
    checkMatches(searcher, query2, 2);
    checkMatches(searcher, query3, 1);
    checkMatches(searcher, query4, 2);
    checkMatches(searcher, query5, 1);
    checkMatches(searcher, query6, 1);
    checkMatches(searcher, query7, 2);
    checkMatches(searcher, query8, 0);
    checkMatches(searcher, newLucene<WildcardQuery>(newLucene<Term>(L"body", L"*tall")), 0);
    checkMatches(searcher, newLucene<WildcardQuery>(newLucene<Term>(L"body", L"*tal")), 1);
    checkMatches(searcher, newLucene<WildcardQuery>(newLucene<Term>(L"body", L"*tal*")), 2);
}

TEST_F(WildcardTest, testLotsOfAsterisks) {
    RAMDirectoryPtr indexStore = getIndexStore(L"body", newCollection<String>(L"metal", L"metals"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);
    StringStream term;
    term << L"m";
    for (int32_t i = 0; i < 512; ++i) {
        term << L"*";
    }
    term << L"tal";
    QueryPtr query3 = newLucene<WildcardQuery>(newLucene<Term>(L"body", term.str()));

    checkMatches(searcher, query3, 1);
    searcher->close();
    indexStore->close();
}

TEST_F(WildcardTest, testQuestionmark) {
    RAMDirectoryPtr indexStore = getIndexStore(L"body", newCollection<String>(L"metal", L"metals", L"mXtals", L"mXtXls"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(indexStore, true);
    QueryPtr query1 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"m?tal"));
    QueryPtr query2 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"metal?"));
    QueryPtr query3 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"metals?"));
    QueryPtr query4 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"m?t?ls"));
    QueryPtr query5 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"M?t?ls"));
    QueryPtr query6 = newLucene<WildcardQuery>(newLucene<Term>(L"body", L"meta??"));

    checkMatches(searcher, query1, 1);
    checkMatches(searcher, query2, 1);
    checkMatches(searcher, query3, 0);
    checkMatches(searcher, query4, 3);
    checkMatches(searcher, query5, 0);
    checkMatches(searcher, query6, 1); // Query: 'meta??' matches 'metals' not 'metal'
}

/// Test that wild card queries are parsed to the correct type and are searched correctly.
/// This test looks at both parsing and execution of wildcard queries.  Although placed
/// here, it also tests prefix queries, verifying that prefix queries are not parsed into
/// wild card queries, and vice-versa.
TEST_F(WildcardTest, testParsingAndSearching) {
    String field = L"content";
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, field, newLucene<WhitespaceAnalyzer>());
    qp->setAllowLeadingWildcard(true);
    Collection<String> docs = newCollection<String>(L"\\ abcdefg1", L"\\79 hijklmn1", L"\\\\ opqrstu1");

    // queries that should find all docs
    Collection<String> matchAll = newCollection<String>(L"*", L"*1", L"**1", L"*?", L"*?1", L"?*1", L"**", L"***", L"\\\\*");

    // queries that should find no docs
    Collection<String> matchNone = newCollection<String>(L"a*h", L"a?h", L"*a*h", L"?a", L"a?");

    // queries that should be parsed to prefix queries
    Collection< Collection<String> > matchOneDocPrefix = newCollection< Collection<String> >(
                newCollection<String>(L"a*", L"ab*", L"abc*"), // these should find only doc 0
                newCollection<String>(L"h*", L"hi*", L"hij*", L"\\\\7*"), // these should find only doc 1
                newCollection<String>(L"o*", L"op*", L"opq*", L"\\\\\\\\*") // these should find only doc 2
            );

    // queries that should be parsed to wildcard queries
    Collection< Collection<String> > matchOneDocWild = newCollection< Collection<String> >(
                newCollection<String>(L"*a*", L"*ab*", L"*abc**", L"ab*e*", L"*g?", L"*f?1", L"abc**"), // these should find only doc 0
                newCollection<String>(L"*h*", L"*hi*", L"*hij**", L"hi*k*", L"*n?", L"*m?1", L"hij**"), // these should find only doc 1
                newCollection<String>(L"*o*", L"*op*", L"*opq**", L"op*q*", L"*u?", L"*t?1", L"opq**") // these should find only doc 2
            );

    // prepare the index
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < docs.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(field, docs[i], Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);

    // test queries that must find all
    for (int32_t i = 0; i < matchAll.size(); ++i) {
        String qtxt = matchAll[i];
        QueryPtr q = qp->parse(qtxt);
        Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(docs.size(), hits.size());
    }

    // test queries that must find none
    for (int32_t i = 0; i < matchNone.size(); ++i) {
        String qtxt = matchNone[i];
        QueryPtr q = qp->parse(qtxt);
        Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(0, hits.size());
    }

    // test queries that must be prefix queries and must find only one doc
    for (int32_t i = 0; i < matchOneDocPrefix.size(); ++i) {
        for (int32_t j = 0; j < matchOneDocPrefix[i].size(); ++j) {
            String qtxt = matchOneDocPrefix[i][j];
            QueryPtr q = qp->parse(qtxt);
            EXPECT_TRUE(MiscUtils::typeOf<PrefixQuery>(q));
            Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
            EXPECT_EQ(1, hits.size());
            EXPECT_EQ(i, hits[0]->doc);
        }
    }

    // test queries that must be wildcard queries and must find only one doc
    for (int32_t i = 0; i < matchOneDocPrefix.size(); ++i) {
        for (int32_t j = 0; j < matchOneDocWild[i].size(); ++j) {
            String qtxt = matchOneDocWild[i][j];
            QueryPtr q = qp->parse(qtxt);
            EXPECT_TRUE(MiscUtils::typeOf<WildcardQuery>(q));
            Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
            EXPECT_EQ(1, hits.size());
            EXPECT_EQ(i, hits[0]->doc);
        }
    }

    searcher->close();
}
