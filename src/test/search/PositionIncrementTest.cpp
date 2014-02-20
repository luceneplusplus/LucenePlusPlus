/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Analyzer.h"
#include "TokenStream.h"
#include "PositionIncrementAttribute.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "TermPositions.h"
#include "PhraseQuery.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "MultiPhraseQuery.h"
#include "QueryParser.h"
#include "WhitespaceAnalyzer.h"
#include "StopFilter.h"
#include "CharArraySet.h"
#include "LowerCaseTokenizer.h"
#include "PayloadAttribute.h"
#include "Payload.h"
#include "StringReader.h"
#include "SpanTermQuery.h"
#include "SpanQuery.h"
#include "SpanNearQuery.h"
#include "Spans.h"
#include "PayloadSpanUtil.h"
#include "Term.h"

using namespace Lucene;

typedef LuceneTestFixture PositionIncrementTest;

namespace TestSetPosition {

class SetPositionTokenStream : public TokenStream {
public:
    SetPositionTokenStream() {
        TOKENS = newCollection<String>(L"1", L"2", L"3", L"4", L"5");
        INCREMENTS = newCollection<int32_t>(0, 2, 1, 0, 1);
        i = 0;

        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        termAtt = addAttribute<TermAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
    }

    virtual ~SetPositionTokenStream() {
    }

protected:
    Collection<String> TOKENS;
    Collection<int32_t> INCREMENTS;
    int32_t i;

    PositionIncrementAttributePtr posIncrAtt;
    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken() {
        if (i == TOKENS.size()) {
            return false;
        }
        clearAttributes();
        termAtt->setTermBuffer(TOKENS[i]);
        offsetAtt->setOffset(i, i);
        posIncrAtt->setPositionIncrement(INCREMENTS[i]);
        ++i;
        return true;
    }
};

class SetPositionAnalyzer : public Analyzer {
public:
    virtual ~SetPositionAnalyzer() {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<SetPositionTokenStream>();
    }
};

class StopWhitespaceAnalyzer : public Analyzer {
public:
    StopWhitespaceAnalyzer(bool enablePositionIncrements) {
        this->enablePositionIncrements = enablePositionIncrements;
        this->a = newLucene<WhitespaceAnalyzer>();
    }

    virtual ~StopWhitespaceAnalyzer() {
    }

public:
    bool enablePositionIncrements;
    WhitespaceAnalyzerPtr a;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr ts = a->tokenStream(fieldName, reader);
        return newLucene<StopFilter>(enablePositionIncrements, ts, newLucene<CharArraySet>(newCollection<String>(L"stop"), true));
    }
};

}

TEST_F(PositionIncrementTest, testSetPosition) {
    AnalyzerPtr analyzer = newLucene<TestSetPosition::SetPositionAnalyzer>();
    DirectoryPtr store = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(store, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr d = newLucene<Document>();
    d->add(newLucene<Field>(L"field", L"bogus", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(d);
    writer->optimize();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(store, true);

    TermPositionsPtr pos = searcher->getIndexReader()->termPositions(newLucene<Term>(L"field", L"1"));
    pos->next();
    // first token should be at position 0
    EXPECT_EQ(0, pos->nextPosition());

    pos = searcher->getIndexReader()->termPositions(newLucene<Term>(L"field", L"2"));
    pos->next();
    // second token should be at position 2
    EXPECT_EQ(2, pos->nextPosition());

    PhraseQueryPtr q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"1"));
    q->add(newLucene<Term>(L"field", L"2"));
    Collection<ScoreDocPtr> hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // same as previous, just specify positions explicitely.
    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"1"), 0);
    q->add(newLucene<Term>(L"field", L"2"), 1);
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // specifying correct positions should find the phrase.
    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"1"), 0);
    q->add(newLucene<Term>(L"field", L"2"), 2);
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"2"));
    q->add(newLucene<Term>(L"field", L"3"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"3"));
    q->add(newLucene<Term>(L"field", L"4"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // phrase query would find it when correct positions are specified.
    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"3"), 0);
    q->add(newLucene<Term>(L"field", L"4"), 0);
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    // phrase query should fail for non existing searched term
    // even if there exist another searched terms in the same searched position.
    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"3"), 0);
    q->add(newLucene<Term>(L"field", L"9"), 0);
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // multi-phrase query should succeed for non existing searched term
    // because there exist another searched terms in the same searched position.
    MultiPhraseQueryPtr mq = newLucene<MultiPhraseQuery>();
    mq->add(newCollection<TermPtr>(newLucene<Term>(L"field", L"3"), newLucene<Term>(L"field", L"9")), 0);
    hits = searcher->search(mq, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"2"));
    q->add(newLucene<Term>(L"field", L"4"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"3"));
    q->add(newLucene<Term>(L"field", L"5"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"4"));
    q->add(newLucene<Term>(L"field", L"5"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());

    q = newLucene<PhraseQuery>();
    q->add(newLucene<Term>(L"field", L"2"));
    q->add(newLucene<Term>(L"field", L"5"));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // should not find "1 2" because there is a gap of 1 in the index
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<TestSetPosition::StopWhitespaceAnalyzer>(false));
    q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"1 2\""));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // omitted stop word cannot help because stop filter swallows the increments.
    q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"1 stop 2\""));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // query parser alone won't help, because stop filter swallows the increments.
    qp->setEnablePositionIncrements(true);
    q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"1 stop 2\""));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // stop filter alone won't help, because query parser swallows the increments.
    qp->setEnablePositionIncrements(false);
    q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"1 stop 2\""));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(0, hits.size());

    // when both qp and stopFilter propagate increments, we should find the doc.
    qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<TestSetPosition::StopWhitespaceAnalyzer>(true));
    qp->setEnablePositionIncrements(true);
    q = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(L"\"1 stop 2\""));
    hits = searcher->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
}

namespace TestPayloadsPos0 {

class TestPayloadFilter : public TokenFilter {
public:
    TestPayloadFilter(const TokenStreamPtr& input, const String& fieldName) : TokenFilter(input) {
        this->fieldName = fieldName;
        this->pos = 0;
        this->i = 0;
        this->posIncrAttr = input->addAttribute<PositionIncrementAttribute>();
        this->payloadAttr = input->addAttribute<PayloadAttribute>();
        this->termAttr = input->addAttribute<TermAttribute>();
    }

    virtual ~TestPayloadFilter() {
    }

public:
    String fieldName;
    int32_t pos;
    int32_t i;

    PositionIncrementAttributePtr posIncrAttr;
    PayloadAttributePtr payloadAttr;
    TermAttributePtr termAttr;

public:
    virtual bool incrementToken() {
        if (input->incrementToken()) {
            String payloadData = L"pos: " + StringUtils::toString(pos);
            ByteArray data = ByteArray::newInstance(payloadData.length() * sizeof(wchar_t));
            std::wcsncpy((wchar_t*)data.get(), payloadData.c_str(), payloadData.length());
            payloadAttr->setPayload(newLucene<Payload>(data));
            int32_t posIncr = i % 2 == 1 ? 1 : 0;
            posIncrAttr->setPositionIncrement(posIncr);
            pos += posIncr;
            ++i;
            return true;
        } else {
            return false;
        }
    }
};

class TestPayloadAnalyzer : public Analyzer {
public:
    virtual ~TestPayloadAnalyzer() {
    }

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr result = newLucene<LowerCaseTokenizer>(reader);
        return newLucene<TestPayloadFilter>(result, fieldName);
    }
};

}

TEST_F(PositionIncrementTest, testPayloadsPos0) {
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<TestPayloadsPos0::TestPayloadAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"content", newLucene<StringReader>(L"a a b c d e a f g h i j a b k k")));
    writer->addDocument(doc);

    IndexReaderPtr r = writer->getReader();

    TermPositionsPtr tp = r->termPositions(newLucene<Term>(L"content", L"a"));
    int32_t count = 0;
    EXPECT_TRUE(tp->next());
    // "a" occurs 4 times
    EXPECT_EQ(4, tp->freq());
    int32_t expected = 0;
    EXPECT_EQ(expected, tp->nextPosition());
    EXPECT_EQ(1, tp->nextPosition());
    EXPECT_EQ(3, tp->nextPosition());
    EXPECT_EQ(6, tp->nextPosition());

    // only one doc has "a"
    EXPECT_TRUE(!tp->next());

    IndexSearcherPtr is = newLucene<IndexSearcher>(r);

    SpanTermQueryPtr stq1 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"a"));
    SpanTermQueryPtr stq2 = newLucene<SpanTermQuery>(newLucene<Term>(L"content", L"k"));

    Collection<SpanQueryPtr> sqs = newCollection<SpanQueryPtr>(stq1, stq2);
    SpanNearQueryPtr snq = newLucene<SpanNearQuery>(sqs, 30, false);

    count = 0;
    bool sawZero = false;
    SpansPtr pspans = snq->getSpans(is->getIndexReader());
    while (pspans->next()) {
        Collection<ByteArray> payloads = pspans->getPayload();
        if (pspans->start() == 0) {
            sawZero = true;
        }
        count += payloads.size();
    }

    EXPECT_EQ(5, count);
    EXPECT_TRUE(sawZero);

    SpansPtr spans = snq->getSpans(is->getIndexReader());
    count = 0;
    sawZero = false;
    while (spans->next()) {
        ++count;
        if (spans->start() == 0) {
            sawZero = true;
        }
    }

    EXPECT_EQ(4, count);
    EXPECT_TRUE(sawZero);

    sawZero = false;
    PayloadSpanUtilPtr psu = newLucene<PayloadSpanUtil>(is->getIndexReader());
    Collection<ByteArray> pls = psu->getPayloadsForQuery(snq);
    count = pls.size();
    for (Collection<ByteArray>::iterator it = pls.begin(); it != pls.end(); ++it) {
        String s = String((wchar_t*)it->get(), it->size() / sizeof(wchar_t));
        if (s == L"pos: 0") {
            sawZero = true;
        }
    }

    EXPECT_EQ(5, count);
    EXPECT_TRUE(sawZero);

    writer->close();
    is->getIndexReader()->close();
    dir->close();
}
