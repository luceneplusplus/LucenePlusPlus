/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NGRAMTOKENFILTER_H
#define NGRAMTOKENFILTER_H

#include "TokenFilter.h"

namespace Lucene {

/// Tokenizes each input token into n-grams of the given size(s).
///
/// Offsets from the original token are preserved. All grams produced from the
/// same input token are emitted at the same position.
class LPPAPI NGramTokenFilter : public TokenFilter {
public:
    static const bool DEFAULT_PRESERVE_ORIGINAL;

public:
    NGramTokenFilter(const TokenStreamPtr& input, int32_t gramSize);
    NGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram);
    NGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram, bool preserveOriginal);

    virtual ~NGramTokenFilter();

    LUCENE_CLASS(NGramTokenFilter);

protected:
    int32_t minGram;
    int32_t maxGram;
    bool preserveOriginal;

    CharArray curTermBuffer;
    int32_t curTermLength;
    int32_t curGramSize;
    int32_t curPos;
    int32_t curPosIncr;
    AttributeSourceStatePtr state;

    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;

protected:
    void init(int32_t minGram, int32_t maxGram, bool preserveOriginal);

public:
    virtual bool incrementToken();
    virtual void reset();
    virtual void end();
};

}

#endif
