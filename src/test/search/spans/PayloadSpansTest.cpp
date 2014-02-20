/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "DefaultSimilarity.h"
#include "PayloadHelper.h"
#include "SpanTermQuery.h"
#include "SpanFirstQuery.h"
#include "Term.h"
#include "Spans.h"
#include "SpanNearQuery.h"
#include "SpanNotQuery.h"
#include "RAMDirectory.h"
#include "PayloadAttribute.h"
#include "TokenFilter.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "Payload.h"
#include "LowerCaseTokenizer.h"
#include "Analyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "StringReader.h"
#include "TopDocs.h"
#include "PayloadSpanUtil.h"
#include "TermQuery.h"

using namespace Lucene;

DECLARE_SHARED_PTR(PayloadSpansAnalyzer)

class PayloadSpansFilter : public TokenFilter {
public:
    PayloadSpansFilter(const TokenStreamPtr& input, const String& fieldName) : TokenFilter(input) {
        this->fieldName = fieldName;
        this->pos = 0;
        this->entities = HashSet<String>::newInstance();
        this->entities.add(L"xx");
        this->entities.add(L"one");
        this->nopayload = HashSet<String>::newInstance();
        this->nopayload.add(L"nopayload");
        this->nopayload.add(L"np");
        this->termAtt = addAttribute<TermAttribute>();
        this->posIncrAtt = addAttribute<PositionIncrementAttribute>();
        this->payloadAtt = addAttribute<PayloadAttribute>();
    }

    virtual ~PayloadSpansFilter() {
    }

    LUCENE_CLASS(PayloadSpansFilter);

public:
    String fieldName;
    HashSet<String> entities;
    HashSet<String> nopayload;
    int32_t pos;
    PayloadAttributePtr payloadAtt;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;

public:
    virtual bool incrementToken() {
        if (input->incrementToken()) {
            String token(termAtt->termBuffer().get(), termAtt->termLength());

            if (!nopayload.contains(token)) {
                StringStream buf;
                buf << token;
                if (entities.contains(token)) {
                    buf << L":Entity:";
                } else {
                    buf << L":Noise:";
                }
                buf << pos;
                ByteArray data = ByteArray::newInstance(buf.str().length() * sizeof(wchar_t));
                std::wcsncpy((wchar_t*)data.get(), buf.str().c_str(), buf.str().length());
                payloadAtt->setPayload(newLucene<Payload>(data));
            }
            pos += posIncrAtt->getPositionIncrement();
            return true;
        } else {
            return false;
        }
    }
};

class PayloadSpansAnalyzer : public Analyzer {
public:
    virtual ~PayloadSpansAnalyzer() {
    }

    LUCENE_CLASS(PayloadSpansAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr result = newLucene<LowerCaseTokenizer>(reader);
        result = newLucene<PayloadSpansFilter>(result, fieldName);
        return result;
    }
};

class PayloadSpansTest : public LuceneTestFixture {
public:
    PayloadSpansTest() {
        similarity = newLucene<DefaultSimilarity>();
        searcher = PayloadHelper::setUp(similarity, 1000);
        indexReader = searcher->getIndexReader();
    }

    virtual ~PayloadSpansTest() {
    }

protected:
    IndexSearcherPtr searcher;
    SimilarityPtr similarity;
    IndexReaderPtr indexReader;

public:
    void checkSpans(const SpansPtr& spans, int32_t expectedNumSpans, int32_t expectedNumPayloads, int32_t expectedPayloadLength, int32_t expectedFirstByte) {
        EXPECT_TRUE(spans);
        int32_t seen = 0;
        while (spans->next()) {
            // if we expect payloads, then isPayloadAvailable should be true
            if (expectedNumPayloads > 0) {
                EXPECT_TRUE(spans->isPayloadAvailable());
            } else {
                EXPECT_TRUE(!spans->isPayloadAvailable());
            }
            // See payload helper, for the PayloadHelper::FIELD field, there is a single byte payload at every token
            if (spans->isPayloadAvailable()) {
                Collection<ByteArray> payload = spans->getPayload();
                EXPECT_EQ(payload.size(), expectedNumPayloads);
                for (Collection<ByteArray>::iterator thePayload = payload.begin(); thePayload != payload.end(); ++thePayload) {
                    EXPECT_EQ(thePayload->size(), expectedPayloadLength);
                    EXPECT_EQ((*thePayload)[0], expectedFirstByte);
                }
            }
            ++seen;
        }
        EXPECT_EQ(seen, expectedNumSpans);
    }

    void checkSpans(const SpansPtr& spans, int32_t numSpans, Collection<int32_t> numPayloads) {
        int32_t cnt = 0;
        while (spans->next()) {
            if (spans->isPayloadAvailable()) {
                Collection<ByteArray> payload = spans->getPayload();
                EXPECT_EQ(numPayloads[cnt], payload.size());
            } else {
                EXPECT_TRUE(numPayloads.size() <= 0 || numPayloads[cnt] <= 0);
            }
        }
        ++cnt;
    }

    IndexSearcherPtr getSpanNotSearcher() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        PayloadSpansAnalyzerPtr analyzer = newLucene<PayloadSpansAnalyzer>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setSimilarity(similarity);

        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(PayloadHelper::FIELD, L"one two three one four three", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
        writer->close();

        IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
        searcher->setSimilarity(similarity);
        return searcher;
    }

    IndexSearcherPtr getSearcher() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        PayloadSpansAnalyzerPtr analyzer = newLucene<PayloadSpansAnalyzer>();
        Collection<String> docs = newCollection<String>(
                                      L"xx rr yy mm  pp", L"xx yy mm rr pp", L"nopayload qq ss pp np",
                                      L"one two three four five six seven eight nine ten eleven",
                                      L"nine one two three four five six seven eight eleven ten"
                                  );
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
        writer->setSimilarity(similarity);

        for (int32_t i = 0; i < docs.size(); ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(PayloadHelper::FIELD, docs[i], Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }

        writer->close();

        IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
        return searcher;
    }
};

TEST_F(PayloadSpansTest, testSpanTermQuery) {
    SpanTermQueryPtr stq = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"seventy"));
    SpansPtr spans = stq->getSpans(indexReader);
    EXPECT_TRUE(spans);
    checkSpans(spans, 100, 1, 1, 1);

    stq = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"seventy"));
    spans = stq->getSpans(indexReader);
    EXPECT_TRUE(spans);
    checkSpans(spans, 100, 0, 0, 0);
}

TEST_F(PayloadSpansTest, testSpanFirst) {
    SpanQueryPtr match = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"one"));
    SpanFirstQueryPtr sfq = newLucene<SpanFirstQuery>(match, 2);
    SpansPtr spans = sfq->getSpans(indexReader);
    checkSpans(spans, 109, 1, 1, 1);
    // Test more complicated subclause
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"one")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"hundred"))
                                       );
    match = newLucene<SpanNearQuery>(clauses, 0, true);
    sfq = newLucene<SpanFirstQuery>(match, 2);
    checkSpans(sfq->getSpans(indexReader), 100, 2, 1, 1);

    match = newLucene<SpanNearQuery>(clauses, 0, false);
    sfq = newLucene<SpanFirstQuery>(match, 2);
    checkSpans(sfq->getSpans(indexReader), 100, 2, 1, 1);
}

TEST_F(PayloadSpansTest, testSpanNot) {
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"one")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"three"))
                                       );
    SpanQueryPtr spq = newLucene<SpanNearQuery>(clauses, 5, true);
    SpanNotQueryPtr snq = newLucene<SpanNotQuery>(spq, newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"two")));
    checkSpans(snq->getSpans(getSpanNotSearcher()->getIndexReader()), 1, newCollection<int32_t>(2));
}

TEST_F(PayloadSpansTest, testNestedSpans) {
    IndexSearcherPtr searcher = getSearcher();
    SpanTermQueryPtr stq = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"mark"));
    SpansPtr spans = stq->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 0, Collection<int32_t>());

    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"rr")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"yy")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"xx"))
                                       );
    SpanNearQueryPtr spanNearQuery = newLucene<SpanNearQuery>(clauses, 12, false);

    spans = spanNearQuery->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 2, newCollection<int32_t>(3, 3));

    clauses[0] = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"xx"));
    clauses[1] = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"rr"));
    clauses[2] = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"yy"));

    spanNearQuery = newLucene<SpanNearQuery>(clauses, 6, true);

    spans = spanNearQuery->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 1, newCollection<int32_t>(3));

    clauses = newCollection<SpanQueryPtr>(
                  newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"xx")),
                  newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"rr"))
              );

    spanNearQuery = newLucene<SpanNearQuery>(clauses, 6, true);

    // xx within 6 of rr
    Collection<SpanQueryPtr> clauses2 = newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"yy")),
                                            spanNearQuery
                                        );

    SpanNearQueryPtr nestedSpanNearQuery = newLucene<SpanNearQuery>(clauses2, 6, false);

    // yy within 6 of xx within 6 of rr
    spans = nestedSpanNearQuery->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 2, newCollection<int32_t>(3, 3));
}

TEST_F(PayloadSpansTest, testFirstClauseWithoutPayload) {
    IndexSearcherPtr searcher = getSearcher();
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"nopayload")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"qq")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"ss"))
                                       );
    SpanNearQueryPtr spanNearQuery = newLucene<SpanNearQuery>(clauses, 6, true);

    Collection<SpanQueryPtr> clauses2 = newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"pp")),
                                            spanNearQuery
                                        );

    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(clauses2, 6, false);

    Collection<SpanQueryPtr> clauses3 = newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"np")),
                                            snq
                                        );

    SpanNearQueryPtr nestedSpanNearQuery = newLucene<SpanNearQuery>(clauses3, 6, false);

    SpansPtr spans = nestedSpanNearQuery->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 1, newCollection<int32_t>(3));
}

TEST_F(PayloadSpansTest, testHeavilyNestedSpanQuery) {
    IndexSearcherPtr searcher = getSearcher();
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"one")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"two")),
                                           newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"three"))
                                       );
    SpanNearQueryPtr spanNearQuery = newLucene<SpanNearQuery>(clauses, 5, true);

    clauses[0] = spanNearQuery;
    clauses[1] = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"five"));
    clauses[2] = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"six"));

    SpanNearQueryPtr spanNearQuery2 = newLucene<SpanNearQuery>(clauses, 6, true);

    Collection<SpanQueryPtr> clauses2 = newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"eleven")),
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"ten"))
                                        );
    SpanNearQueryPtr spanNearQuery3 = newLucene<SpanNearQuery>(clauses2, 2, false);

    Collection<SpanQueryPtr> clauses3 = newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"nine")),
                                            spanNearQuery2,
                                            spanNearQuery3
                                        );
    SpanNearQueryPtr nestedSpanNearQuery = newLucene<SpanNearQuery>(clauses3, 6, false);

    SpansPtr spans = nestedSpanNearQuery->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    checkSpans(spans, 2, newCollection<int32_t>(8, 8));
}

TEST_F(PayloadSpansTest, testShrinkToAfterShortestMatch) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<PayloadSpansAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", newLucene<StringReader>(L"a b c d e f g h i j a k")));
    writer->addDocument(doc);
    writer->close();

    IndexSearcherPtr is = newLucene<IndexSearcher>(directory, true);

    SpanTermQueryPtr stq1 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"a"));
    SpanTermQueryPtr stq2 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"k"));
    Collection<SpanQueryPtr> sqs = newCollection<SpanQueryPtr>(stq1, stq2);
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(sqs, 1, true);
    SpansPtr spans = snq->getSpans(is->getIndexReader());

    TopDocsPtr topDocs = is->search(snq, 1);
    HashSet<String> payloadSet = HashSet<String>::newInstance();
    for (int32_t i = 0; i < topDocs->scoreDocs.size(); ++i) {
        while (spans->next()) {
            Collection<ByteArray> payloads = spans->getPayload();
            for (Collection<ByteArray>::iterator it = payloads.begin(); it != payloads.end(); ++it) {
                payloadSet.add(String((wchar_t*)it->get(), it->size() / sizeof(wchar_t)));
            }
        }
    }
    EXPECT_EQ(2, payloadSet.size());
    EXPECT_TRUE(payloadSet.contains(L"a:Noise:10"));
    EXPECT_TRUE(payloadSet.contains(L"k:Noise:11"));
}

TEST_F(PayloadSpansTest, testShrinkToAfterShortestMatch2) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<PayloadSpansAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", newLucene<StringReader>(L"a b a d k f a h i k a k")));
    writer->addDocument(doc);
    writer->close();

    IndexSearcherPtr is = newLucene<IndexSearcher>(directory, true);

    SpanTermQueryPtr stq1 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"a"));
    SpanTermQueryPtr stq2 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"k"));
    Collection<SpanQueryPtr> sqs = newCollection<SpanQueryPtr>(stq1, stq2);
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(sqs, 0, true);
    SpansPtr spans = snq->getSpans(is->getIndexReader());

    TopDocsPtr topDocs = is->search(snq, 1);
    HashSet<String> payloadSet = HashSet<String>::newInstance();
    for (int32_t i = 0; i < topDocs->scoreDocs.size(); ++i) {
        while (spans->next()) {
            Collection<ByteArray> payloads = spans->getPayload();
            for (Collection<ByteArray>::iterator it = payloads.begin(); it != payloads.end(); ++it) {
                payloadSet.add(String((wchar_t*)it->get(), it->size() / sizeof(wchar_t)));
            }
        }
    }
    EXPECT_EQ(2, payloadSet.size());
    EXPECT_TRUE(payloadSet.contains(L"a:Noise:10"));
    EXPECT_TRUE(payloadSet.contains(L"k:Noise:11"));
}

TEST_F(PayloadSpansTest, testShrinkToAfterShortestMatch3) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<PayloadSpansAnalyzer>(), IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", newLucene<StringReader>(L"j k a l f k k p a t a k l k t a")));
    writer->addDocument(doc);
    writer->close();

    IndexSearcherPtr is = newLucene<IndexSearcher>(directory, true);

    SpanTermQueryPtr stq1 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"a"));
    SpanTermQueryPtr stq2 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"k"));
    Collection<SpanQueryPtr> sqs = newCollection<SpanQueryPtr>(stq1, stq2);
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(sqs, 0, true);
    SpansPtr spans = snq->getSpans(is->getIndexReader());

    TopDocsPtr topDocs = is->search(snq, 1);
    HashSet<String> payloadSet = HashSet<String>::newInstance();
    for (int32_t i = 0; i < topDocs->scoreDocs.size(); ++i) {
        while (spans->next()) {
            Collection<ByteArray> payloads = spans->getPayload();
            for (Collection<ByteArray>::iterator it = payloads.begin(); it != payloads.end(); ++it) {
                payloadSet.add(String((wchar_t*)it->get(), it->size() / sizeof(wchar_t)));
            }
        }
    }
    EXPECT_EQ(2, payloadSet.size());
    EXPECT_TRUE(payloadSet.contains(L"a:Noise:10"));
    EXPECT_TRUE(payloadSet.contains(L"k:Noise:11"));
}

TEST_F(PayloadSpansTest, testPayloadSpanUtil) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    PayloadSpansAnalyzerPtr analyzer = newLucene<PayloadSpansAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setSimilarity(similarity);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(PayloadHelper::FIELD, L"xx rr yy mm  pp", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    IndexReaderPtr reader = searcher->getIndexReader();
    PayloadSpanUtilPtr psu = newLucene<PayloadSpanUtil>(reader);

    Collection<ByteArray> payloads = psu->getPayloadsForQuery(newLucene<TermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"rr")));
    EXPECT_EQ(1, payloads.size());
    EXPECT_EQ(String((wchar_t*)(payloads[0].get()), payloads[0].size() / sizeof(wchar_t)), L"rr:Noise:1");
}
