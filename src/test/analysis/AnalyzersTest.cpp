/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "SimpleAnalyzer.h"
#include "WhitespaceAnalyzer.h"
#include "StopAnalyzer.h"
#include "TokenFilter.h"
#include "WhitespaceTokenizer.h"
#include "StringReader.h"
#include "PayloadAttribute.h"
#include "Payload.h"

using namespace Lucene;

typedef BaseTokenStreamFixture AnalyzersTest;

static void verifyPayload(const TokenStreamPtr& ts) {
    PayloadAttributePtr payloadAtt = ts->getAttribute<PayloadAttribute>();
    for (uint8_t b = 1; ; ++b) {
        bool hasNext = ts->incrementToken();
        if (!hasNext) {
            break;
        }
        EXPECT_EQ(b, payloadAtt->getPayload()->toByteArray()[0]);
    }
}

TEST_F(AnalyzersTest, testSimple) {
    AnalyzerPtr a = newLucene<SimpleAnalyzer>();
    checkAnalyzesTo(a, L"foo bar FOO BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(a, L"foo      bar .  FOO <> BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(a, L"foo.bar.FOO.BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(a, L"U.S.A.", newCollection<String>(L"u", L"s", L"a"));
    checkAnalyzesTo(a, L"C++", newCollection<String>(L"c"));
    checkAnalyzesTo(a, L"B2B", newCollection<String>(L"b", L"b"));
    checkAnalyzesTo(a, L"2B", newCollection<String>(L"b"));
    checkAnalyzesTo(a, L"\"QUOTED\" word", newCollection<String>(L"quoted", L"word"));
}

TEST_F(AnalyzersTest, testNull) {
    AnalyzerPtr a = newLucene<WhitespaceAnalyzer>();
    checkAnalyzesTo(a, L"foo bar FOO BAR", newCollection<String>(L"foo", L"bar", L"FOO", L"BAR"));
    checkAnalyzesTo(a, L"foo      bar .  FOO <> BAR", newCollection<String>(L"foo", L"bar", L".", L"FOO", L"<>", L"BAR"));
    checkAnalyzesTo(a, L"foo.bar.FOO.BAR", newCollection<String>(L"foo.bar.FOO.BAR"));
    checkAnalyzesTo(a, L"U.S.A.", newCollection<String>(L"U.S.A."));
    checkAnalyzesTo(a, L"C++", newCollection<String>(L"C++"));
    checkAnalyzesTo(a, L"B2B", newCollection<String>(L"B2B"));
    checkAnalyzesTo(a, L"2B", newCollection<String>(L"2B"));
    checkAnalyzesTo(a, L"\"QUOTED\" word", newCollection<String>(L"\"QUOTED\"", L"word"));
}

TEST_F(AnalyzersTest, testStop) {
    AnalyzerPtr a = newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(a, L"foo bar FOO BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(a, L"foo a bar such FOO THESE BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
}

namespace TestPayloadCopy {

DECLARE_SHARED_PTR(PayloadSetter)

class PayloadSetter : public TokenFilter {
public:
    PayloadSetter(const TokenStreamPtr& input) : TokenFilter(input) {
        payloadAtt = addAttribute<PayloadAttribute>();
        data = ByteArray::newInstance(1);
        data[0] = 0;
        p = newLucene<Payload>(data, 0, 1);
    }

    virtual ~PayloadSetter() {
    }

public:
    PayloadAttributePtr payloadAtt;
    ByteArray data;
    PayloadPtr p;

public:
    virtual bool incrementToken() {
        bool hasNext = input->incrementToken();
        if (!hasNext) {
            return false;
        }
        payloadAtt->setPayload(p); // reuse the payload / byte[]
        data[0]++;
        return true;
    }
};

}

/// Make sure old style next() calls result in a new copy of payloads
TEST_F(AnalyzersTest, testPayloadCopy) {
    String s = L"how now brown cow";
    TokenStreamPtr ts = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(s));
    ts = newLucene<TestPayloadCopy::PayloadSetter>(ts);
    verifyPayload(ts);

    ts = newLucene<WhitespaceTokenizer>(newLucene<StringReader>(s));
    ts = newLucene<TestPayloadCopy::PayloadSetter>(ts);
    verifyPayload(ts);
}
