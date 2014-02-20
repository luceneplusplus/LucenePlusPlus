/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "PayloadHelper.h"
#include "TestUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "PayloadAttribute.h"
#include "TokenFilter.h"
#include "Payload.h"
#include "LowerCaseTokenizer.h"
#include "Analyzer.h"

namespace Lucene {

const String PayloadHelper::NO_PAYLOAD_FIELD = L"noPayloadField";
const String PayloadHelper::MULTI_FIELD = L"multiField";
const String PayloadHelper::FIELD = L"field";

DECLARE_SHARED_PTR(PayloadHelperAnalyzer)

class PayloadHelperFilter : public TokenFilter {
public:
    PayloadHelperFilter(const TokenStreamPtr& input, const String& fieldName) : TokenFilter(input) {
        this->numSeen = 0;
        this->fieldName = fieldName;
        this->payloadAtt = addAttribute<PayloadAttribute>();
    }

    virtual ~PayloadHelperFilter() {
    }

    LUCENE_CLASS(PayloadHelperFilter);

public:
    String fieldName;
    int32_t numSeen;
    PayloadAttributePtr payloadAtt;

public:
    virtual bool incrementToken() {
        if (input->incrementToken()) {
            if (fieldName == PayloadHelper::FIELD) {
                payloadAtt->setPayload(newLucene<Payload>(PayloadHelper::payloadField()));
            } else if (fieldName == PayloadHelper::MULTI_FIELD) {
                if (numSeen % 2 == 0) {
                    payloadAtt->setPayload(newLucene<Payload>(PayloadHelper::payloadMultiField1()));
                } else {
                    payloadAtt->setPayload(newLucene<Payload>(PayloadHelper::payloadMultiField2()));
                }
                ++numSeen;
            }
            return true;
        }
        return false;
    }
};

class PayloadHelperAnalyzer : public Analyzer {
public:
    virtual ~PayloadHelperAnalyzer() {
    }

    LUCENE_CLASS(PayloadHelperAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenStreamPtr result = newLucene<LowerCaseTokenizer>(reader);
        result = newLucene<PayloadHelperFilter>(result, fieldName);
        return result;
    }
};

PayloadHelper::~PayloadHelper() {
}

const ByteArray PayloadHelper::payloadField() {
    static ByteArray _payloadField;
    if (!_payloadField) {
        _payloadField = ByteArray::newInstance(1);
        _payloadField[0] = 1;
    }
    return _payloadField;
}

const ByteArray PayloadHelper::payloadMultiField1() {
    static ByteArray _payloadMultiField1;
    if (!_payloadMultiField1) {
        _payloadMultiField1 = ByteArray::newInstance(1);
        _payloadMultiField1[0] = 2;
    }
    return _payloadMultiField1;
}

const ByteArray PayloadHelper::payloadMultiField2() {
    static ByteArray _payloadMultiField2;
    if (!_payloadMultiField2) {
        _payloadMultiField2 = ByteArray::newInstance(1);
        _payloadMultiField2[0] = 4;
    }
    return _payloadMultiField2;
}

IndexSearcherPtr PayloadHelper::setUp(const SimilarityPtr& similarity, int32_t numDocs) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    PayloadHelperAnalyzerPtr analyzer = newLucene<PayloadHelperAnalyzer>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->setSimilarity(similarity);
    for (int32_t i = 0; i < numDocs; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD, intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(MULTI_FIELD, intToEnglish(i) + L"  " + intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
        doc->add(newLucene<Field>(NO_PAYLOAD_FIELD, intToEnglish(i), Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
    searcher->setSimilarity(similarity);
    return searcher;
}

}
