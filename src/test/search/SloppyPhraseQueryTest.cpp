/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Document.h"
#include "Field.h"
#include "PhraseQuery.h"
#include "Term.h"
#include "RAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "TopDocs.h"

using namespace Lucene;

class SloppyPhraseQueryTest : public LuceneTestFixture {
public:
    SloppyPhraseQueryTest() {
        S_1 = L"A A A";
        S_2 = L"A 1 2 3 A 4 5 6 A";

        DOC_1 = makeDocument(L"X " + S_1 + L" Y");
        DOC_2 = makeDocument(L"X " + S_2 + L" Y");
        DOC_3 = makeDocument(L"X " + S_1 + L" A Y");
        DOC_1_B = makeDocument(L"X " + S_1 + L" Y N N N N " + S_1 + L" Z");
        DOC_2_B = makeDocument(L"X " + S_2 + L" Y N N N N " + S_2 + L" Z");
        DOC_3_B = makeDocument(L"X " + S_1 + L" A Y N N N N " + S_1 + L" A Y");
        DOC_4 = makeDocument(L"A A X A X B A X B B A A X B A A");

        QUERY_1 = makePhraseQuery(S_1);
        QUERY_2 = makePhraseQuery(S_2);
        QUERY_4 = makePhraseQuery(L"X A A");
    }

    virtual ~SloppyPhraseQueryTest() {
    }

protected:
    String S_1;
    String S_2;

    DocumentPtr DOC_1;
    DocumentPtr DOC_2;
    DocumentPtr DOC_3;
    DocumentPtr DOC_1_B;
    DocumentPtr DOC_2_B;
    DocumentPtr DOC_3_B;
    DocumentPtr DOC_4;

    PhraseQueryPtr QUERY_1;
    PhraseQueryPtr QUERY_2;
    PhraseQueryPtr QUERY_4;

public:
    DocumentPtr makeDocument(const String& docText) {
        DocumentPtr doc = newLucene<Document>();
        FieldPtr f = newLucene<Field>(L"f", docText, Field::STORE_NO, Field::INDEX_ANALYZED);
        f->setOmitNorms(true);
        doc->add(f);
        return doc;
    }

    PhraseQueryPtr makePhraseQuery(const String& terms) {
        PhraseQueryPtr query = newLucene<PhraseQuery>();
        Collection<String> tokens = StringUtils::split(terms, L" +");
        for (int32_t i = 0; i < tokens.size(); ++i) {
            query->add(newLucene<Term>(L"f", tokens[i]));
        }
        return query;
    }

    double checkPhraseQuery(const DocumentPtr& doc, const PhraseQueryPtr& query, int32_t slop, int32_t expectedNumResults) {
        query->setSlop(slop);

        RAMDirectoryPtr ramDir = newLucene<RAMDirectory>();
        WhitespaceAnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
        IndexWriterPtr writer = newLucene<IndexWriter>(ramDir, analyzer, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->addDocument(doc);
        writer->close();

        IndexSearcherPtr searcher = newLucene<IndexSearcher>(ramDir, true);
        TopDocsPtr td = searcher->search(query, FilterPtr(), 10);
        EXPECT_EQ(expectedNumResults, td->totalHits);

        searcher->close();
        ramDir->close();

        return td->maxScore;
    }
};

/// Test DOC_4 and QUERY_4.
/// QUERY_4 has a fuzzy (len=1) match to DOC_4, so all slop values > 0 should succeed.
/// But only the 3rd sequence of A's in DOC_4 will do.
TEST_F(SloppyPhraseQueryTest, testDoc4Query4AllSlopsShouldMatch) {
    for (int32_t slop = 0; slop < 30; ++slop) {
        int32_t numResultsExpected = slop < 1 ? 0 : 1;
        checkPhraseQuery(DOC_4, QUERY_4, slop, numResultsExpected);
    }
}

/// Test DOC_1 and QUERY_1.
/// QUERY_1 has an exact match to DOC_1, so all slop values should succeed.
TEST_F(SloppyPhraseQueryTest, testDoc1Query1AllSlopsShouldMatch) {
    for (int32_t slop = 0; slop < 30; ++slop) {
        double score1 = checkPhraseQuery(DOC_1, QUERY_1, slop, 1);
        double score2 = checkPhraseQuery(DOC_1_B, QUERY_1, slop, 1);
        EXPECT_TRUE(score2 > score1);
    }
}

/// Test DOC_2 and QUERY_1.
/// 6 should be the minimum slop to make QUERY_1 match DOC_2.
TEST_F(SloppyPhraseQueryTest, testDoc2Query1Slop6OrMoreShouldMatch) {
    for (int32_t slop = 0; slop < 30; ++slop) {
        int32_t numResultsExpected = slop < 6 ? 0 : 1;
        double score1 = checkPhraseQuery(DOC_2, QUERY_1, slop, numResultsExpected);
        if (numResultsExpected > 0) {
            double score2 = checkPhraseQuery(DOC_2_B, QUERY_1, slop, 1);
            EXPECT_TRUE(score2 > score1);
        }
    }
}

/// Test DOC_2 and QUERY_2.
/// QUERY_2 has an exact match to DOC_2, so all slop values should succeed.
TEST_F(SloppyPhraseQueryTest, testDoc2Query2AllSlopsShouldMatch) {
    for (int32_t slop = 0; slop < 30; ++slop) {
        double score1 = checkPhraseQuery(DOC_2, QUERY_2, slop, 1);
        double score2 = checkPhraseQuery(DOC_2_B, QUERY_2, slop, 1);
        EXPECT_TRUE(score2 > score1);
    }
}

/// Test DOC_3 and QUERY_1.
/// QUERY_1 has an exact match to DOC_3, so all slop values should succeed.
TEST_F(SloppyPhraseQueryTest, testDoc3Query1AllSlopsShouldMatch) {
    for (int32_t slop = 0; slop < 30; ++slop) {
        double score1 = checkPhraseQuery(DOC_3, QUERY_1, slop, 1);
        double score2 = checkPhraseQuery(DOC_3_B, QUERY_1, slop, 1);
        EXPECT_TRUE(score2 > score1);
    }
}
