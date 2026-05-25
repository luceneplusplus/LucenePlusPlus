/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NGRAMTOKENIZER_H
#define NGRAMTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene {

/// Tokenizes the input into n-grams of the given size(s).
///
/// This tokenizer emits tokens by increasing start offset, then increasing
/// length. Offsets identify the exact characters in the original input that
/// produced each term.
class LPPAPI NGramTokenizer : public Tokenizer {
public:
    static const int32_t DEFAULT_MIN_NGRAM_SIZE;
    static const int32_t DEFAULT_MAX_NGRAM_SIZE;

public:
    NGramTokenizer(const ReaderPtr& input);
    NGramTokenizer(const ReaderPtr& input, int32_t minGram, int32_t maxGram);
    NGramTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input, int32_t minGram, int32_t maxGram);
    NGramTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input, int32_t minGram, int32_t maxGram);

    virtual ~NGramTokenizer();

    LUCENE_CLASS(NGramTokenizer);

protected:
    CharArray buffer;
    int32_t bufferStart;
    int32_t bufferEnd;
    int32_t offset;
    int32_t gramSize;
    int32_t minGram;
    int32_t maxGram;
    bool exhausted;
    int32_t lastCheckedChar;
    int32_t lastNonTokenChar;

    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;
    PositionIncrementAttributePtr posIncrAtt;

protected:
    void init(int32_t minGram, int32_t maxGram);
    void resetState();
    void fillBuffer();
    void consume();
    void updateLastNonTokenChar();

    /// Only collect characters which satisfy this condition.
    virtual bool isTokenChar(wchar_t chr);

public:
    virtual bool incrementToken();
    virtual void end();
    virtual void reset();
    virtual void reset(const ReaderPtr& input);
};

}

#endif
