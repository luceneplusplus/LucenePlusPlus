/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/regex.hpp>
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DefaultSimilarity.h"
#include "RAMDirectory.h"
#include "PayloadAttribute.h"
#include "TokenFilter.h"
#include "Payload.h"
#include "LowerCaseTokenizer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "PayloadNearQuery.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "QueryUtils.h"
#include "Analyzer.h"
#include "SpanQuery.h"
#include "PayloadTermQuery.h"
#include "Term.h"
#include "AveragePayloadFunction.h"

using namespace Lucene;

DECLARE_SHARED_PTR(BoostingNearSimilarity)
DECLARE_SHARED_PTR(PayloadNearAnalyzer)

class BoostingNearIDFExplanation : public IDFExplanation {
public:
    virtual ~BoostingNearIDFExplanation() {
    }

public:
    virtual double getIdf() {
        return 1.0;
    }

    virtual String explain() {
        return L"Inexplicable";
    }
};

class BoostingNearSimilarity : public DefaultSimilarity {
public:
    virtual ~BoostingNearSimilarity() {
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

    virtual double tf(double freq) {
        return 1.0;
    }

    virtual IDFExplanationPtr idfExplain(Collection<TermPtr> terms, const SearcherPtr& searcher) {
        return newLucene<BoostingNearIDFExplanation>();
    }
};

class PayloadNearFilter : public TokenFilter {
public:
    PayloadNearFilter(ByteArray payload2, ByteArray payload4, const TokenStreamPtr& input, const String& fieldName) : TokenFilter(input) {
        this->payload2 = payload2;
        this->payload4 = payload4;
        this->numSeen = 0;
        this->fieldName = fieldName;
        this->payAtt = addAttribute<PayloadAttribute>();
    }

    virtual ~PayloadNearFilter() {
    }

    LUCENE_CLASS(PayloadNearFilter);

public:
    ByteArray payload2;
    ByteArray payload4;
    String fieldName;
    int32_t numSeen;
    PayloadAttributePtr payAtt;

public:
    virtual bool incrementToken() {
        bool result = false;
        if (input->incrementToken()) {
            if (numSeen % 2 == 0) {
                payAtt->setPayload(newLucene<Payload>(payload2));
            } else {
                payAtt->setPayload(newLucene<Payload>(payload4));
            }
            ++numSeen;
            result = true;
        }
        return result;
    }
};

class PayloadNearAnalyzer : public Analyzer {
public:
    PayloadNearAnalyzer(ByteArray payload2, ByteArray payload4) {
        this->payload2 = payload2;
        this->payload4 = payload4;
    }

    virtual ~PayloadNearAnalyzer() {
    }

    LUCENE_CLASS(PayloadNearAnalyzer);

protected:
    ByteArray payload2;
    ByteArray payload4;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr result = newLucene<LowerCaseTokenizer>(reader);
        result = newLucene<PayloadNearFilter>(payload2, payload4, result, fieldName);
        return result;
    }
};

class PayloadNearQueryTest : public LuceneTestFixture {
public:
    PayloadNearQueryTest() {
        similarity = newLucene<BoostingNearSimilarity>();
        payload2 = ByteArray::newInstance(1);
        payload2[0] = 2;
        payload4 = ByteArray::newInstance(1);
        payload4[0] = 4;

        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        PayloadNearAnalyzerPtr analyzer = newLucene<PayloadNearAnalyzer>(payload2, payload4);
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setSimilarity(similarity);
        for (int32_t i = 0; i < 1000; ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"field", intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
            String txt = intToEnglish(i) + L" " + intToEnglish(i + 1);
            doc->add(newLucene<Field>(L"field2", txt, Field::STORE_YES, Field::INDEX_ANALYZED));
            writer->addDocument(doc);
        }
        writer->optimize();
        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
        searcher->setSimilarity(similarity);
    }

    virtual ~PayloadNearQueryTest() {
    }

protected:
    IndexSearcherPtr searcher;
    BoostingNearSimilarityPtr similarity;
    ByteArray payload2;
    ByteArray payload4;

public:
    PayloadNearQueryPtr newPhraseQuery(const String& fieldName, const String& phrase, bool inOrder) {
        std::wstring phraseClauses(phrase.c_str());
        Collection<SpanQueryPtr> clauses = Collection<SpanQueryPtr>::newInstance();
        boost::wsregex_token_iterator tokenIterator(phraseClauses.begin(), phraseClauses.end(), boost::wregex(L"[\\s]+"), -1);
        boost::wsregex_token_iterator endToken;
        while (tokenIterator != endToken) {
            clauses.add(newLucene<PayloadTermQuery>(newLucene<Term>(fieldName, *tokenIterator), newLucene<AveragePayloadFunction>()));
            ++tokenIterator;
        }
        return newLucene<PayloadNearQuery>(clauses, 0, inOrder);
    }

    SpanNearQueryPtr spanNearQuery(const String& fieldName, const String& words) {
        std::wstring phraseClauses(words.c_str());
        Collection<SpanQueryPtr> clauses = Collection<SpanQueryPtr>::newInstance();
        boost::wsregex_token_iterator tokenIterator(phraseClauses.begin(), phraseClauses.end(), boost::wregex(L"[\\s]+"), -1);
        boost::wsregex_token_iterator endToken;
        while (tokenIterator != endToken) {
            clauses.add(newLucene<PayloadTermQuery>(newLucene<Term>(fieldName, *tokenIterator), newLucene<AveragePayloadFunction>()));
            ++tokenIterator;
        }
        return newLucene<SpanNearQuery>(clauses, 10000, false);
    }
};

TEST_F(PayloadNearQueryTest, testSetup) {
    PayloadNearQueryPtr query = newPhraseQuery(L"field", L"twenty two", true);
    QueryUtils::check(query);

    // all 10 hits should have score = 3 because adjacent terms have payloads of 2, 4 and all the similarity factors are set to 1
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    EXPECT_EQ(hits->totalHits, 10);
    for (int32_t j = 0; j < hits->scoreDocs.size(); ++j) {
        ScoreDocPtr doc = hits->scoreDocs[j];
        EXPECT_EQ(doc->score, 3);
    }
    for (int32_t i = 1; i < 10; ++i) {
        query = newPhraseQuery(L"field", intToEnglish(i) + L" hundred", true);
        // all should have score = 3 because adjacent terms have payloads of 2, 4 and all the similarity factors are set to 1
        hits = searcher->search(query, FilterPtr(), 100);
        EXPECT_TRUE(hits);
        EXPECT_EQ(hits->totalHits, 100);
        for (int32_t j = 0; j < hits->scoreDocs.size(); ++j) {
            ScoreDocPtr doc = hits->scoreDocs[j];
            EXPECT_EQ(doc->score, 3);
        }
    }
}

TEST_F(PayloadNearQueryTest, testPayloadNear) {
    SpanNearQueryPtr q1 = spanNearQuery(L"field2", L"twenty two");
    SpanNearQueryPtr q2 = spanNearQuery(L"field2", L"twenty three");
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(q1, q2);
    PayloadNearQueryPtr query = newLucene<PayloadNearQuery>(clauses, 10, false);
    EXPECT_EQ(12, searcher->search(query, FilterPtr(), 100)->totalHits);
}

TEST_F(PayloadNearQueryTest, testLongerSpan) {
    SpanNearQueryPtr query = newPhraseQuery(L"field", L"nine hundred ninety nine", true);
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    ScoreDocPtr doc = hits->scoreDocs[0];
    EXPECT_EQ(hits->totalHits, 1);
    // should have score = 3 because adjacent terms have payloads of 2,4
    EXPECT_EQ(doc->score, 3);
}

TEST_F(PayloadNearQueryTest, testComplexNested) {
    // combine ordered and unordered spans with some nesting to make sure all payloads are counted
    SpanQueryPtr q1 = newPhraseQuery(L"field", L"nine hundred", true);
    SpanQueryPtr q2 = newPhraseQuery(L"field", L"ninety nine", true);
    SpanQueryPtr q3 = newPhraseQuery(L"field", L"nine ninety", false);
    SpanQueryPtr q4 = newPhraseQuery(L"field", L"hundred nine", false);
    Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                           newLucene<PayloadNearQuery>(newCollection<SpanQueryPtr>(q1, q2), 0, true),
                                           newLucene<PayloadNearQuery>(newCollection<SpanQueryPtr>(q3, q4), 0, false)
                                       );
    PayloadNearQueryPtr query = newLucene<PayloadNearQuery>(clauses, 0, false);
    TopDocsPtr hits = searcher->search(query, FilterPtr(), 100);
    EXPECT_TRUE(hits);
    // should be only 1 hit - doc 999
    EXPECT_EQ(hits->scoreDocs.size(), 1);
    // the score should be 3 - the average of all the underlying payloads
    ScoreDocPtr doc = hits->scoreDocs[0];
    EXPECT_EQ(doc->score, 3);
}
