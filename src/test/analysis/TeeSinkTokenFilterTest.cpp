/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "BaseTokenStreamFixture.h"
#include "TestUtils.h"
#include "TeeSinkTokenFilter.h"
#include "WhitespaceTokenizer.h"
#include "TokenStream.h"
#include "TermAttribute.h"
#include "StringReader.h"
#include "CachingTokenFilter.h"
#include "LowerCaseFilter.h"
#include "StandardFilter.h"
#include "StandardTokenizer.h"
#include "PositionIncrementAttribute.h"
#include "MiscUtils.h"

using namespace Lucene;

class TheSinkFilter : public SinkFilter {
public:
    virtual ~TheSinkFilter() {
    }

public:
    virtual bool accept(const AttributeSourcePtr& source) {
        TermAttributePtr termAtt = source->getAttribute<TermAttribute>();
        return boost::iequals(termAtt->term(), L"The");
    }
};

class DogSinkFilter : public SinkFilter {
public:
    virtual ~DogSinkFilter() {
    }

public:
    virtual bool accept(const AttributeSourcePtr& source) {
        TermAttributePtr termAtt = source->getAttribute<TermAttribute>();
        return boost::iequals(termAtt->term(), L"Dogs");
    }
};

class TeeSinkTokenFilterTest : public BaseTokenStreamFixture {
public:
    TeeSinkTokenFilterTest() {
        tokens1 = newCollection<String>(L"The", L"quick", L"Burgundy", L"Fox", L"jumped", L"over", L"the", L"lazy", L"Red", L"Dogs");
        tokens2 = newCollection<String>(L"The", L"Lazy", L"Dogs", L"should", L"stay", L"on", L"the", L"porch");

        for (int32_t i = 0; i < tokens1.size(); ++i) {
            buffer1 << tokens1[i] << L" ";
        }
        for (int32_t i = 0; i < tokens2.size(); ++i) {
            buffer2 << tokens2[i] << L" ";
        }

        theFilter = newLucene<TheSinkFilter>();
        dogFilter = newLucene<DogSinkFilter>();
    }

    virtual ~TeeSinkTokenFilterTest() {
    }

protected:
    StringStream buffer1;
    StringStream buffer2;
    Collection<String> tokens1;
    Collection<String> tokens2;

    SinkFilterPtr theFilter;
    SinkFilterPtr dogFilter;
};

TEST_F(TeeSinkTokenFilterTest, testGeneral) {
    TeeSinkTokenFilterPtr source = newLucene<TeeSinkTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(buffer1.str())));
    TokenStreamPtr sink1 = source->newSinkTokenStream();
    TokenStreamPtr sink2 = source->newSinkTokenStream(theFilter);

    source->addAttribute<CheckClearAttributesAttribute>();
    sink1->addAttribute<CheckClearAttributesAttribute>();
    sink2->addAttribute<CheckClearAttributesAttribute>();

    checkTokenStreamContents(source, tokens1);
    checkTokenStreamContents(sink1, tokens1);
    checkTokenStreamContents(sink2, newCollection<String>(L"The", L"the"));
}

TEST_F(TeeSinkTokenFilterTest, testMultipleSources) {
    TeeSinkTokenFilterPtr tee1 = newLucene<TeeSinkTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(buffer1.str())));
    SinkTokenStreamPtr dogDetector = tee1->newSinkTokenStream(dogFilter);
    SinkTokenStreamPtr theDetector = tee1->newSinkTokenStream(theFilter);
    TokenStreamPtr source1 = newLucene<CachingTokenFilter>(tee1);

    tee1->addAttribute<CheckClearAttributesAttribute>();
    dogDetector->addAttribute<CheckClearAttributesAttribute>();
    theDetector->addAttribute<CheckClearAttributesAttribute>();

    TeeSinkTokenFilterPtr tee2 = newLucene<TeeSinkTokenFilter>(newLucene<WhitespaceTokenizer>(newLucene<StringReader>(buffer2.str())));
    tee2->addSinkTokenStream(dogDetector);
    tee2->addSinkTokenStream(theDetector);
    TokenStreamPtr source2 = tee2;

    checkTokenStreamContents(source1, tokens1);
    checkTokenStreamContents(source2, tokens2);

    checkTokenStreamContents(theDetector, newCollection<String>(L"The", L"the", L"The", L"the"));
    checkTokenStreamContents(dogDetector, newCollection<String>(L"Dogs", L"Dogs"));

    source1->reset();
    TokenStreamPtr lowerCasing = newLucene<LowerCaseFilter>(source1);
    Collection<String> lowerCaseTokens = Collection<String>::newInstance(tokens1.size());
    for (int32_t i = 0; i < tokens1.size(); ++i) {
        lowerCaseTokens[i] = StringUtils::toLower((const String&)tokens1[i]);
    }
    checkTokenStreamContents(lowerCasing, lowerCaseTokens);
}

namespace TestPerformance {

class ModuloTokenFilter : public TokenFilter {
public:
    ModuloTokenFilter(const TokenStreamPtr& input, int32_t mc) : TokenFilter(input) {
        modCount = mc;
        count = 0;
    }

    virtual ~ModuloTokenFilter() {
    }

public:
    int32_t modCount;
    int32_t count;

public:
    // return every 100 tokens
    virtual bool incrementToken() {
        bool hasNext = false;
        for (hasNext = input->incrementToken(); hasNext && count % modCount != 0; hasNext = input->incrementToken()) {
            ++count;
        }
        ++count;
        return hasNext;
    }
};

class ModuloSinkFilter : public SinkFilter {
public:
    ModuloSinkFilter(int32_t mc) {
        modCount = mc;
        count = 0;
    }

    virtual ~ModuloSinkFilter() {
    }

public:
    int32_t modCount;
    int32_t count;

public:
    virtual bool accept(const AttributeSourcePtr& source) {
        bool b = (source && count % modCount == 0);
        ++count;
        return b;
    }
};

}

/// Not an explicit test, just useful to print out some info on performance
TEST_F(TeeSinkTokenFilterTest, testPerformance) {
    Collection<int32_t> tokCount = newCollection<int32_t>(100, 500, 1000, 2000, 5000, 10000);
    Collection<int32_t> modCounts = newCollection<int32_t>(1, 2, 5, 10, 20, 50, 100, 200, 500);
    for (int32_t k = 0; k < tokCount.size(); ++k) {
        StringStream buffer;
        // std::cout << "-----Tokens: " << tokCount[k] << "-----";
        for (int32_t i = 0; i < tokCount[k]; ++i) {
            buffer << StringUtils::toUpper(intToEnglish(i)) << L" ";
        }
        // make sure we produce the same tokens
        TeeSinkTokenFilterPtr teeStream = newLucene<TeeSinkTokenFilter>(newLucene<StandardFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(buffer.str()))));
        TokenStreamPtr sink = teeStream->newSinkTokenStream(newLucene<TestPerformance::ModuloSinkFilter>(100));
        teeStream->consumeAllTokens();
        TokenStreamPtr stream = newLucene<TestPerformance::ModuloTokenFilter>(newLucene<StandardFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(buffer.str()))), 100);
        TermAttributePtr tfTok = stream->addAttribute<TermAttribute>();
        TermAttributePtr sinkTok = sink->addAttribute<TermAttribute>();
        for (int32_t i = 0; stream->incrementToken(); ++i) {
            EXPECT_TRUE(sink->incrementToken());
            EXPECT_TRUE(tfTok->equals(sinkTok));
        }

        // simulate two fields, each being analyzed once, for 20 documents
        for (int32_t j = 0; j < modCounts.size(); ++j) {
            int32_t tfPos = 0;
            int64_t start = MiscUtils::currentTimeMillis();
            for (int32_t i = 0; i < 20; ++i) {
                stream = newLucene<StandardFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(buffer.str())));
                PositionIncrementAttributePtr posIncrAtt = stream->getAttribute<PositionIncrementAttribute>();
                while (stream->incrementToken()) {
                    tfPos += posIncrAtt->getPositionIncrement();
                }
                stream = newLucene<TestPerformance::ModuloTokenFilter>(newLucene<StandardFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(buffer.str()))), modCounts[j]);
                posIncrAtt = stream->getAttribute<PositionIncrementAttribute>();
                while (stream->incrementToken()) {
                    tfPos += posIncrAtt->getPositionIncrement();
                }
            }
            int64_t finish = MiscUtils::currentTimeMillis();
            // std::cout << "ModCount: " << modCounts[j] << " Two fields took " << (finish - start) << " ms";
            int32_t sinkPos = 0;
            // simulate one field with one sink
            start = MiscUtils::currentTimeMillis();
            for (int32_t i = 0; i < 20; ++i) {
                teeStream = newLucene<TeeSinkTokenFilter>(newLucene<StandardFilter>(newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, newLucene<StringReader>(buffer.str()))));
                sink = teeStream->newSinkTokenStream(newLucene<TestPerformance::ModuloSinkFilter>(modCounts[j]));
                PositionIncrementAttributePtr posIncrAtt = teeStream->getAttribute<PositionIncrementAttribute>();
                while (teeStream->incrementToken()) {
                    sinkPos += posIncrAtt->getPositionIncrement();
                }
                posIncrAtt = sink->getAttribute<PositionIncrementAttribute>();
                while (sink->incrementToken()) {
                    sinkPos += posIncrAtt->getPositionIncrement();
                }
            }
            finish = MiscUtils::currentTimeMillis();
            // std::cout << "ModCount: " << modCounts[j] << " Tee fields took " << (finish - start) << " ms";
            EXPECT_EQ(sinkPos, tfPos);
        }
        // std::cout << "- End Tokens: " << tokCount[k] << "-----";
    }
}
