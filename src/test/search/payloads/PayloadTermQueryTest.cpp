/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "IndexSearcher.h"
#include "DefaultSimilarity.h"
#include "RAMDirectory.h"
#include "Analyzer.h"
#include "TokenFilter.h"
#include "LowerCaseTokenizer.h"
#include "PayloadAttribute.h"
#include "Payload.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "PayloadTermQuery.h"
#include "Term.h"
#include "MaxPayloadFunction.h"
#include "AveragePayloadFunction.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "CheckHits.h"
#include "TermSpans.h"
#include "SpanTermQuery.h"
#include "QueryUtils.h"
#include "BooleanClause.h"
#include "BooleanQuery.h"
#include "PayloadHelper.h"
#include "MiscUtils.h"

using namespace Lucene;

DECLARE_SHARED_PTR(BoostingTermSimilarity)
DECLARE_SHARED_PTR(PayloadTermAnalyzer)

class BoostingTermSimilarity : public DefaultSimilarity {
public:
    virtual ~BoostingTermSimilarity() {
    }

public:
    virtual double scorePayload(int32_t docId, const String& fieldName, int32_t start, int32_t end, ByteArray payload, int32_t offset, int32_t length) {
        // we know it is size 4 here, so ignore the offset/length
        return (double)payload[0];
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

    virtual double coord(int32_t overlap, int32_t maxOverlap) {
        return 1.0;
    }

    virtual double idf(int32_t docFreq, int32_t numDocs) {
        return 1.0;
    }

    virtual double tf(double freq) {
        return freq == 0.0 ? 0.0 : 1.0;
    }
};

class FullSimilarity : public DefaultSimilarity {
public:
    virtual ~FullSimilarity() {
    }

public:
    virtual double scorePayload(int32_t docId, const String& fieldName, int32_t start, int32_t end, ByteArray payload, int32_t offset, int32_t length) {
        // we know it is size 4 here, so ignore the offset/length
        return payload[0];
    }
};

class PayloadTermFilter : public TokenFilter {
public:
    PayloadTermFilter(ByteArray payloadField, ByteArray payloadMultiField1, ByteArray payloadMultiField2, const TokenStreamPtr& input, const String& fieldName) : TokenFilter(input) {
        this->payloadField = payloadField;
        this->payloadMultiField1 = payloadMultiField1;
        this->payloadMultiField2 = payloadMultiField2;
        this->numSeen = 0;
        this->fieldName = fieldName;
        this->payloadAtt = addAttribute<PayloadAttribute>();
    }

    virtual ~PayloadTermFilter() {
    }

    LUCENE_CLASS(PayloadTermFilter);

public:
    ByteArray payloadField;
    ByteArray payloadMultiField1;
    ByteArray payloadMultiField2;
    String fieldName;
    int32_t numSeen;
    PayloadAttributePtr payloadAtt;

public:
    virtual bool incrementToken() {
        bool hasNext = input->incrementToken();
        if (hasNext) {
            if (fieldName == L"field") {
                payloadAtt->setPayload(newLucene<Payload>(payloadField));
            } else if (fieldName == L"multiField") {
                if (numSeen % 2 == 0) {
                    payloadAtt->setPayload(newLucene<Payload>(payloadMultiField1));
                } else {
                    payloadAtt->setPayload(newLucene<Payload>(payloadMultiField2));
                }
                ++numSeen;
            }
            return true;
        } else {
            return false;
        }
    }
};

class PayloadTermAnalyzer : public Analyzer {
public:
    PayloadTermAnalyzer(ByteArray payloadField, ByteArray payloadMultiField1, ByteArray payloadMultiField2) {
        this->payloadField = payloadField;
        this->payloadMultiField1 = payloadMultiField1;
        this->payloadMultiField2 = payloadMultiField2;
    }

    virtual ~PayloadTermAnalyzer() {
    }

    LUCENE_CLASS(PayloadTermAnalyzer);

protected:
    ByteArray payloadField;
    ByteArray payloadMultiField1;
    ByteArray payloadMultiField2;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr result = newLucene<LowerCaseTokenizer>(reader);
        result = newLucene<PayloadTermFilter>(payloadField, payloadMultiField1, payloadMultiField2, result, fieldName);
        return result;
    }
};

class PayloadTermQueryTest : public LuceneTestFixture {
public:
    PayloadTermQueryTest() {
        similarity = newLucene<BoostingTermSimilarity>();
        payloadField = ByteArray::newInstance(1);
        payloadField[0] = 1;
        payloadMultiField1 = ByteArray::newInstance(1);
        payloadMultiField1[0] = 2;
        payloadMultiField2 = ByteArray::newInstance(1);
        payloadMultiField2[0] = 4;

        directory = newLucene<RAMDirectory>();
        PayloadTermAnalyzerPtr analyzer = newLucene<PayloadTermAnalyzer>(payloadField, payloadMultiField1, payloadMultiField2);
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setSimilarity(similarity);
        for (int32_t i = 0; i < 1000; ++i) {
            DocumentPtr doc = newLucene<Document>();
            FieldPtr noPayloadField = newLucene<Field>(PayloadHelper::NO_PAYLOAD_FIELD, intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED);
            doc->add(noPayloadField);
            doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            doc->add(newLucene<Field>(L"multiField", intToEnglish(i) + L"  " + intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->optimize();
        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
        searcher->setSimilarity(similarity);
    }

    virtual ~PayloadTermQueryTest() {
    }

protected:
    IndexSearcherPtr searcher;
    BoostingTermSimilarityPtr similarity;
    ByteArray payloadField;
    ByteArray payloadMultiField1;
    ByteArray payloadMultiField2;
    RAMDirectoryPtr directory;
};

TEST_F(PayloadTermQueryTest, testSetup) {
    PayloadTermQueryPtr query = newLucene<PayloadTermQuery>(newLucene<Term>(L"field", L"seventy"), newLucene<MaxPayloadFunction>());
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 100);

    // they should all have the exact same score, because they all contain seventy once, and we set all the other similarity factors to be 1
    EXPECT_EQ(hits->getMaxScore(), 1);
    for (int32_t i = 0; i < hits->scoreDocs.size(); ++i) {
        ScoreDocPtr doc = hits->scoreDocs[i];
        EXPECT_EQ(doc->score, 1);
    }
    CheckHits::checkExplanations(query, PayloadHelper::FIELD, searcher, true);
    SpansPtr spans = query->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    EXPECT_TRUE(MiscUtils::typeOf<TermSpans>(spans));
}

TEST_F(PayloadTermQueryTest, testQuery) {
    PayloadTermQueryPtr BoostingTermFuncTermQuery = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::MULTI_FIELD, L"seventy"), newLucene<MaxPayloadFunction>());
    QueryUtils::check(BoostingTermFuncTermQuery);

    SpanTermQueryPtr spanTermQuery = newLucene<SpanTermQuery>(newLucene<Term>(PayloadHelper::MULTI_FIELD, L"seventy"));
    EXPECT_TRUE(BoostingTermFuncTermQuery->equals(spanTermQuery) == spanTermQuery->equals(BoostingTermFuncTermQuery));

    PayloadTermQueryPtr BoostingTermFuncTermQuery2 = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::MULTI_FIELD, L"seventy"), newLucene<AveragePayloadFunction>());

    QueryUtils::checkUnequal(BoostingTermFuncTermQuery, BoostingTermFuncTermQuery2);
}

TEST_F(PayloadTermQueryTest, testMultipleMatchesPerDoc) {
    PayloadTermQueryPtr query = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::MULTI_FIELD, L"seventy"), newLucene<MaxPayloadFunction>());
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 100);

    // they should all have the exact same score, because they all contain seventy once, and we set all the other similarity factors to be 1
    EXPECT_EQ(hits->getMaxScore(), 4.0);

    // there should be exactly 10 items that score a 4, all the rest should score a 2
    // The 10 items are: 70 + i*100 where i in [0-9]
    int32_t numTens = 0;
    for (int32_t i = 0; i < hits->scoreDocs.size(); ++i) {
        ScoreDocPtr doc = hits->scoreDocs[i];
        if (doc->doc % 10 == 0) {
            ++numTens;
            EXPECT_EQ(doc->score, 4.0);
        } else {
            EXPECT_EQ(doc->score, 2.0);
        }
    }
    EXPECT_EQ(numTens, 10);
    CheckHits::checkExplanations(query, L"field", searcher, true);
    SpansPtr spans = query->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    EXPECT_TRUE(MiscUtils::typeOf<TermSpans>(spans));
    // should be two matches per document
    int32_t count = 0;
    // 100 hits times 2 matches per hit, we should have 200 in count
    while (spans->next()) {
        ++count;
    }
    EXPECT_EQ(count, 200);
}

TEST_F(PayloadTermQueryTest, testIgnoreSpanScorer) {
    PayloadTermQueryPtr query = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::MULTI_FIELD, L"seventy"), newLucene<MaxPayloadFunction>(), false);

    IndexSearcherPtr theSearcher = newLucene<IndexSearcher>(directory, true);
    theSearcher->setSimilarity(newLucene<FullSimilarity>());
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 100);

    // they should all have the exact same score, because they all contain seventy once, and we set all the other similarity factors to be 1
    EXPECT_EQ(hits->getMaxScore(), 4.0);

    // there should be exactly 10 items that score a 4, all the rest should score a 2
    // The 10 items are: 70 + i*100 where i in [0-9]
    int32_t numTens = 0;
    for (int32_t i = 0; i < hits->scoreDocs.size(); ++i) {
        ScoreDocPtr doc = hits->scoreDocs[i];
        if (doc->doc % 10 == 0) {
            ++numTens;
            EXPECT_EQ(doc->score, 4.0);
        } else {
            EXPECT_EQ(doc->score, 2.0);
        }
    }
    EXPECT_EQ(numTens, 10);
    CheckHits::checkExplanations(query, L"field", searcher, true);
    SpansPtr spans = query->getSpans(searcher->getIndexReader());
    EXPECT_TRUE(spans);
    EXPECT_TRUE(MiscUtils::typeOf<TermSpans>(spans));
    // should be two matches per document
    int32_t count = 0;
    // 100 hits times 2 matches per hit, we should have 200 in count
    while (spans->next()) {
        ++count;
    }
    EXPECT_EQ(count, 200);
}

TEST_F(PayloadTermQueryTest, testNoMatch) {
    PayloadTermQueryPtr query = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::FIELD, L"junk"), newLucene<MaxPayloadFunction>());
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 0);
}

TEST_F(PayloadTermQueryTest, testNoPayload) {
    PayloadTermQueryPtr q1 = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"zero"), newLucene<MaxPayloadFunction>());
    PayloadTermQueryPtr q2 = newLucene<PayloadTermQuery>(newLucene<Term>(PayloadHelper::NO_PAYLOAD_FIELD, L"foo"), newLucene<MaxPayloadFunction>());
    BooleanClausePtr c1 = newLucene<BooleanClause>(q1, BooleanClause::MUST);
    BooleanClausePtr c2 = newLucene<BooleanClause>(q2, BooleanClause::MUST_NOT);
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(c1);
    query->add(c2);
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 1);
    Collection<int32_t> results = newCollection<int32_t>(0);
    CheckHits::checkHitCollector(query, PayloadHelper::NO_PAYLOAD_FIELD, searcher, results);
}
