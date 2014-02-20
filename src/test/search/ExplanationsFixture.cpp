/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "ExplanationsFixture.h"
#include "SpanTermQuery.h"
#include "Term.h"
#include "CheckHits.h"
#include "RAMDirectory.h"
#include "WhitespaceAnalyzer.h"
#include "QueryParser.h"
#include "Document.h"
#include "Field.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "SpanFirstQuery.h"
#include "SpanOrQuery.h"
#include "SpanNearQuery.h"
#include "SpanNotQuery.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

namespace Lucene {

const String ExplanationsFixture::KEY = L"KEY";
const String ExplanationsFixture::FIELD = L"field";

ExplanationsFixture::ExplanationsFixture() {
    qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, FIELD, newLucene<WhitespaceAnalyzer>());
    docFields = newCollection<String>(L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3 zz", L"w1 xx w2 yy w3", L"w1 w3 xx w2 yy w3 zz");

    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer= newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

    for (int32_t i = 0; i < docFields.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(KEY, StringUtils::toString(i), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(FIELD, docFields[i], Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();
    searcher = newLucene<IndexSearcher>(directory, true);
}

ExplanationsFixture::~ExplanationsFixture() {
    searcher->close();
}

SpanTermQueryPtr ExplanationsFixture::st(const String& s) {
    return newLucene<SpanTermQuery>(newLucene<Term>(FIELD, s));
}

SpanFirstQueryPtr ExplanationsFixture::sf(const String& s, int32_t b) {
    return newLucene<SpanFirstQuery>(st(s), b);
}

SpanNotQueryPtr ExplanationsFixture::snot(const SpanQueryPtr& i, const SpanQueryPtr& e) {
    return newLucene<SpanNotQuery>(i, e);
}

SpanOrQueryPtr ExplanationsFixture::sor(const String& s, const String& e) {
    return sor(st(s), st(e));
}

SpanOrQueryPtr ExplanationsFixture::sor(const SpanQueryPtr& s, const SpanQueryPtr& e) {
    return newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(s, e));
}

SpanOrQueryPtr ExplanationsFixture::sor(const String& s, const String& m, const String& e) {
    return sor(st(s), st(m), st(e));
}

SpanOrQueryPtr ExplanationsFixture::sor(const SpanQueryPtr& s, const SpanQueryPtr& m, const SpanQueryPtr& e) {
    return newLucene<SpanOrQuery>(newCollection<SpanQueryPtr>(s, m, e));
}

SpanNearQueryPtr ExplanationsFixture::snear(const String& s, const String& e, int32_t slop, bool inOrder) {
    return snear(st(s), st(e), slop, inOrder);
}

SpanNearQueryPtr ExplanationsFixture::snear(const SpanQueryPtr& s, const SpanQueryPtr& e, int32_t slop, bool inOrder) {
    return newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(s, e), slop, inOrder);
}

SpanNearQueryPtr ExplanationsFixture::snear(const String& s, const String& m, const String& e, int32_t slop, bool inOrder) {
    return snear(st(s), st(m), st(e), slop, inOrder);
}

SpanNearQueryPtr ExplanationsFixture::snear(const SpanQueryPtr& s, const SpanQueryPtr& m, const SpanQueryPtr& e, int32_t slop, bool inOrder) {
    return newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(s, m, e), slop, inOrder);
}

QueryPtr ExplanationsFixture::optB(const String& q) {
    return optB(makeQuery(q));
}

QueryPtr ExplanationsFixture::optB(const QueryPtr& q) {
    BooleanQueryPtr bq = newLucene<BooleanQuery>(true);
    bq->add(q, BooleanClause::SHOULD);
    bq->add(newLucene<TermQuery>(newLucene<Term>(L"NEVER", L"MATCH")), BooleanClause::MUST_NOT);
    return bq;
}

QueryPtr ExplanationsFixture::reqB(const String& q) {
    return reqB(makeQuery(q));
}

QueryPtr ExplanationsFixture::reqB(const QueryPtr& q) {
    BooleanQueryPtr bq = newLucene<BooleanQuery>(true);
    bq->add(q, BooleanClause::MUST);
    bq->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"w1")), BooleanClause::SHOULD);
    return bq;
}

Collection<TermPtr> ExplanationsFixture::ta(Collection<String> s) {
    Collection<TermPtr> t = Collection<TermPtr>::newInstance(s.size());
    for (int32_t i = 0; i < s.size(); ++i) {
        t[i] = newLucene<Term>(FIELD, s[i]);
    }
    return t;
}

void ExplanationsFixture::qtest(const String& queryText, Collection<int32_t> expDocNrs) {
    qtest(makeQuery(queryText), expDocNrs);
}

void ExplanationsFixture::qtest(const QueryPtr& q, Collection<int32_t> expDocNrs) {
    CheckHits::checkHitCollector(q, FIELD, searcher, expDocNrs);
}

void ExplanationsFixture::bqtest(const QueryPtr& q, Collection<int32_t> expDocNrs) {
    qtest(reqB(q), expDocNrs);
    qtest(optB(q), expDocNrs);
}

void ExplanationsFixture::bqtest(const String& queryText, Collection<int32_t> expDocNrs) {
    bqtest(makeQuery(queryText), expDocNrs);
}

QueryPtr ExplanationsFixture::makeQuery(const String& queryText) {
    return qp->parse(queryText);
}

}
