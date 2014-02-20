/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "IndexSearcher.h"
#include "MultiSearcher.h"
#include "Document.h"
#include "Field.h"
#include "QueryParser.h"
#include "Query.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

class MultiSearcherRankingTest : public LuceneTestFixture {
public:
    MultiSearcherRankingTest() {
        // create MultiSearcher from two separate searchers
        DirectoryPtr d1 = newLucene<RAMDirectory>();
        IndexWriterPtr iw1 = newLucene<IndexWriter>(d1, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        addCollection1(iw1);
        iw1->close();
        DirectoryPtr d2 = newLucene<RAMDirectory>();
        IndexWriterPtr iw2 = newLucene<IndexWriter>(d2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        addCollection2(iw2);
        iw2->close();

        Collection<SearchablePtr> s = newCollection<SearchablePtr>(newLucene<IndexSearcher>(d1, true), newLucene<IndexSearcher>(d2, true));
        multiSearcher = newLucene<MultiSearcher>(s);

        // create IndexSearcher which contains all documents
        DirectoryPtr d = newLucene<RAMDirectory>();
        IndexWriterPtr iw = newLucene<IndexWriter>(d, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        addCollection1(iw);
        addCollection2(iw);
        iw->close();
        singleSearcher = newLucene<IndexSearcher>(d, true);
    }

    virtual ~MultiSearcherRankingTest() {
    }

protected:
    static const String FIELD_NAME;
    SearcherPtr multiSearcher;
    SearcherPtr singleSearcher;

public:
    void addCollection1(const IndexWriterPtr& iw) {
        add(L"one blah three", iw);
        add(L"one foo three multiOne", iw);
        add(L"one foobar three multiThree", iw);
        add(L"blueberry pie", iw);
        add(L"blueberry strudel", iw);
        add(L"blueberry pizza", iw);
    }

    void addCollection2(const IndexWriterPtr& iw) {
        add(L"two blah three", iw);
        add(L"two foo xxx multiTwo", iw);
        add(L"two foobar xxx multiThreee", iw);
        add(L"blueberry chewing gum", iw);
        add(L"bluebird pizza", iw);
        add(L"bluebird foobar pizza", iw);
        add(L"piccadilly circus", iw);
    }

    void add(const String& value, const IndexWriterPtr& iw) {
        DocumentPtr d = newLucene<Document>();
        d->add(newLucene<Field>(FIELD_NAME, value, Field::STORE_YES, Field::INDEX_ANALYZED));
        iw->addDocument(d);
    }

    /// checks if a query yields the same result when executed on a single IndexSearcher containing all
    /// documents and on MultiSearcher aggregating sub-searchers
    /// @param queryStr  the query to check.
    void checkQuery(const String& queryStr) {
        QueryParserPtr queryParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, FIELD_NAME, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
        QueryPtr query = queryParser->parse(queryStr);
        Collection<ScoreDocPtr> multiSearcherHits = multiSearcher->search(query, FilterPtr(), 1000)->scoreDocs;
        Collection<ScoreDocPtr> singleSearcherHits = singleSearcher->search(query, FilterPtr(), 1000)->scoreDocs;
        EXPECT_EQ(multiSearcherHits.size(), singleSearcherHits.size());
        for (int32_t i = 0; i < multiSearcherHits.size(); ++i) {
            DocumentPtr docMulti = multiSearcher->doc(multiSearcherHits[i]->doc);
            DocumentPtr docSingle = singleSearcher->doc(singleSearcherHits[i]->doc);
            EXPECT_NEAR(multiSearcherHits[i]->score, singleSearcherHits[i]->score, 0.001);
            EXPECT_EQ(docMulti->get(FIELD_NAME), docSingle->get(FIELD_NAME));
        }
    }
};

const String MultiSearcherRankingTest::FIELD_NAME = L"body";

TEST_F(MultiSearcherRankingTest, testOneTermQuery) {
    checkQuery(L"three");
}

TEST_F(MultiSearcherRankingTest, testTwoTermQuery) {
    checkQuery(L"three foo");
}

TEST_F(MultiSearcherRankingTest, testPrefixQuery) {
    checkQuery(L"multi*");
}

TEST_F(MultiSearcherRankingTest, testFuzzyQuery) {
    checkQuery(L"multiThree~");
}

TEST_F(MultiSearcherRankingTest, testRangeQuery) {
    checkQuery(L"{multiA TO multiP}");
}

TEST_F(MultiSearcherRankingTest, testMultiPhraseQuery) {
    checkQuery(L"\"blueberry pi*\"");
}

TEST_F(MultiSearcherRankingTest, testNoMatchQuery) {
    checkQuery(L"+three +nomatch");
}
