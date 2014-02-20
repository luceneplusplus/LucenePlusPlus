/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "IndexSearcher.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "FieldMaskingSpanQuery.h"
#include "SpanTermQuery.h"
#include "SpanOrQuery.h"
#include "SpanNearQuery.h"
#include "Term.h"
#include "QueryUtils.h"
#include "CheckHits.h"
#include "Spans.h"

using namespace Lucene;

class FieldMaskingSpanQueryTest : public LuceneTestFixture {
public:
    FieldMaskingSpanQueryTest() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer= newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        writer->addDocument(doc(newCollection<FieldPtr>(
                                    field(L"id", L"0"),
                                    field(L"gender", L"male"),
                                    field(L"first", L"james"),
                                    field(L"last", L"jones")))
                           );

        writer->addDocument(doc(newCollection<FieldPtr>(
                                    field(L"id", L"1"),
                                    field(L"gender", L"male"),
                                    field(L"first", L"james"),
                                    field(L"last", L"smith"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"sally"),
                                    field(L"last", L"jones")))
                           );

        writer->addDocument(doc(newCollection<FieldPtr>(
                                    field(L"id", L"2"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"greta"),
                                    field(L"last", L"jones"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"sally"),
                                    field(L"last", L"smith"),
                                    field(L"gender", L"male"),
                                    field(L"first", L"james"),
                                    field(L"last", L"jones")))
                           );

        writer->addDocument(doc(newCollection<FieldPtr>(
                                    field(L"id", L"3"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"lisa"),
                                    field(L"last", L"jones"),
                                    field(L"gender", L"male"),
                                    field(L"first", L"bob"),
                                    field(L"last", L"costas")))
                           );

        writer->addDocument(doc(newCollection<FieldPtr>(
                                    field(L"id", L"4"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"sally"),
                                    field(L"last", L"smith"),
                                    field(L"gender", L"female"),
                                    field(L"first", L"linda"),
                                    field(L"last", L"dixit"),
                                    field(L"gender", L"male"),
                                    field(L"first", L"bubba"),
                                    field(L"last", L"jones")))
                           );

        writer->close();
        searcher = newLucene<IndexSearcher>(directory, true);
    }

    virtual ~FieldMaskingSpanQueryTest() {
        searcher->close();
    }

protected:
    IndexSearcherPtr searcher;

public:
    DocumentPtr doc(Collection<FieldPtr> fields) {
        DocumentPtr doc = newLucene<Document>();
        for (int32_t i = 0; i < fields.size(); ++i) {
            doc->add(fields[i]);
        }
        return doc;
    }

    FieldPtr field(const String& name, const String& value) {
        return newLucene<Field>(name, value, Field::STORE_NO, Field::INDEX_ANALYZED);
    }

    void check(const SpanQueryPtr& q, Collection<int32_t> docs) {
        CheckHits::checkHitCollector(q, L"", searcher, docs);
    }

    String str(const SpansPtr& span) {
        return str(span->doc(), span->start(), span->end());
    }

    String str(int32_t doc, int32_t start, int32_t end) {
        return L"s(" + StringUtils::toString(doc) + L"," + StringUtils::toString(start) + L"," + StringUtils::toString(end) + L")";
    }
};

TEST_F(FieldMaskingSpanQueryTest, testRewrite0) {
    SpanQueryPtr q = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    q->setBoost(8.7654321);
    SpanQueryPtr qr = boost::dynamic_pointer_cast<SpanQuery>(searcher->rewrite(q));

    QueryUtils::checkEqual(q, qr);

    SetTerm terms = SetTerm::newInstance();
    qr->extractTerms(terms);
    EXPECT_EQ(1, terms.size());
}

namespace TestRewrite {

class TestableFieldMaskingSpanQuery : public FieldMaskingSpanQuery {
public:
    TestableFieldMaskingSpanQuery(const SpanQueryPtr& maskedQuery, const String& maskedField) : FieldMaskingSpanQuery(maskedQuery, maskedField) {
    }

    virtual ~TestableFieldMaskingSpanQuery() {
    }

public:
    virtual QueryPtr rewrite(const IndexReaderPtr& reader) {
        return newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(
                                          newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"sally")),
                                          newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"james")))
                                     );
    }
};

}

TEST_F(FieldMaskingSpanQueryTest, testRewrite1) {
    // mask an anon SpanQuery class that rewrites to something else.
    SpanQueryPtr q = newLucene<TestRewrite::TestableFieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    SpanQueryPtr qr = boost::dynamic_pointer_cast<SpanQuery>(searcher->rewrite(q));

    QueryUtils::checkUnequal(q, qr);

    SetTerm terms = SetTerm::newInstance();
    qr->extractTerms(terms);
    EXPECT_EQ(2, terms.size());
}

TEST_F(FieldMaskingSpanQueryTest, testRewrite2) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"smith"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"jones"));
    SpanQueryPtr q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, newLucene<FieldMaskingSpanQuery>(q2, L"last")), 1, true);
    QueryPtr qr = searcher->rewrite(q);
    QueryUtils::checkEqual(q, qr);
    SetTerm terms = SetTerm::newInstance();
    qr->extractTerms(terms);
    EXPECT_EQ(2, terms.size());
}

TEST_F(FieldMaskingSpanQueryTest, testEquality1) {
    SpanQueryPtr q1 = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    SpanQueryPtr q2 = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    SpanQueryPtr q3 = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"XXXXX");
    SpanQueryPtr q4 = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"XXXXX")), L"first");
    SpanQueryPtr q5 = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"xXXX", L"sally")), L"first");
    QueryUtils::checkEqual(q1, q2);
    QueryUtils::checkUnequal(q1, q3);
    QueryUtils::checkUnequal(q1, q4);
    QueryUtils::checkUnequal(q1, q5);

    SpanQueryPtr qA = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    qA->setBoost(9.0);
    SpanQueryPtr qB = newLucene<FieldMaskingSpanQuery>(newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally")), L"first");
    QueryUtils::checkUnequal(qA, qB);
    qB->setBoost(9.0);
    QueryUtils::checkEqual(qA, qB);
}

TEST_F(FieldMaskingSpanQueryTest, testNoop0) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"sally"));
    SpanQueryPtr q = newLucene<FieldMaskingSpanQuery>(q1, L"first");
    check(q, Collection<int32_t>::newInstance());
}

TEST_F(FieldMaskingSpanQueryTest, testNoop1) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"smith"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"jones"));
    SpanQueryPtr q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, newLucene<FieldMaskingSpanQuery>(q2, L"last")), 0, true);
    check(q, newCollection<int32_t>(1, 2));
    q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<FieldMaskingSpanQuery>(q1, L"last"), newLucene<FieldMaskingSpanQuery>(q2, L"last")), 0, true);
    check(q, newCollection<int32_t>(1, 2));
}

TEST_F(FieldMaskingSpanQueryTest, testSimple1) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"james"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"jones"));
    SpanQueryPtr q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, newLucene<FieldMaskingSpanQuery>(q2, L"first")), -1, false);
    check(q, newCollection<int32_t>(0, 2));
    q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<FieldMaskingSpanQuery>(q2, L"first"), q1), -1, false);
    check(q, newCollection<int32_t>(0, 2));
    q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q2, newLucene<FieldMaskingSpanQuery>(q1, L"last")), -1, false);
    check(q, newCollection<int32_t>(0, 2));
    q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<FieldMaskingSpanQuery>(q1, L"last"), q2), -1, false);
    check(q, newCollection<int32_t>(0, 2));
}

TEST_F(FieldMaskingSpanQueryTest, testSimple2) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"gender", L"female"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"smith"));
    SpanQueryPtr q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(q1, newLucene<FieldMaskingSpanQuery>(q2, L"gender")), -1, false);
    check(q, newCollection<int32_t>(2, 4));
    q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<FieldMaskingSpanQuery>(q1, L"id"), newLucene<FieldMaskingSpanQuery>(q2, L"id")), -1, false);
    check(q, newCollection<int32_t>(2, 4));
}

TEST_F(FieldMaskingSpanQueryTest, testSpans0) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"gender", L"female"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"james"));
    SpanQueryPtr q = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(q1, newLucene<FieldMaskingSpanQuery>(q2, L"gender")));
    check(q, newCollection<int32_t>(0, 1, 2, 3, 4));

    SpansPtr span = q->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(0, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(1, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(1, 1, 2), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(2, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(2, 1, 2), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(2, 2, 3), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(3, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(4, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(4, 1, 2), str(span));

    EXPECT_TRUE(!span->next());
}

TEST_F(FieldMaskingSpanQueryTest, testSpans1) {
    SpanQueryPtr q1 = newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"sally"));
    SpanQueryPtr q2 = newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"james"));
    SpanQueryPtr qA = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(q1, q2));
    SpanQueryPtr qB = newLucene<FieldMaskingSpanQuery>(qA, L"id");

    check(qA, newCollection<int32_t>(0, 1, 2, 4));
    check(qB, newCollection<int32_t>(0, 1, 2, 4));

    SpansPtr spanA = qA->getSpans(searcher->getIndexReader());
    SpansPtr spanB = qB->getSpans(searcher->getIndexReader());

    while (spanA->next()) {
        EXPECT_TRUE(spanB->next());
        EXPECT_EQ(str(spanA), str(spanB));
    }
    EXPECT_TRUE(!(spanB->next()));
}

TEST_F(FieldMaskingSpanQueryTest, testSpans2) {
    SpanQueryPtr qA1 = newLucene<SpanTermQuery>(newLucene<Term>(L"gender", L"female"));
    SpanQueryPtr qA2 = newLucene<SpanTermQuery>(newLucene<Term>(L"first", L"james"));
    SpanQueryPtr qA = newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(qA1, newLucene<FieldMaskingSpanQuery>(qA2, L"gender")));
    SpanQueryPtr qB = newLucene<SpanTermQuery>(newLucene<Term>(L"last", L"jones"));
    SpanQueryPtr q = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(newLucene<FieldMaskingSpanQuery>(qA, L"id"), newLucene<FieldMaskingSpanQuery>(qB, L"id")), -1, false);
    check(q, newCollection<int32_t>(0, 1, 2, 3));

    SpansPtr span = q->getSpans(searcher->getIndexReader());

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(0, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(1, 1, 2), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(2, 0, 1), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(2, 2, 3), str(span));

    EXPECT_TRUE(span->next());
    EXPECT_EQ(str(3, 0, 1), str(span));

    EXPECT_TRUE(!span->next());
}
