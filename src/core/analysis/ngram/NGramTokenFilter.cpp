/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NGramTokenFilter.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"

namespace Lucene {

const bool NGramTokenFilter::DEFAULT_PRESERVE_ORIGINAL = false;

NGramTokenFilter::NGramTokenFilter(const TokenStreamPtr& input, int32_t gramSize) : TokenFilter(input) {
    init(gramSize, gramSize, DEFAULT_PRESERVE_ORIGINAL);
}

NGramTokenFilter::NGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram) : TokenFilter(input) {
    init(minGram, maxGram, DEFAULT_PRESERVE_ORIGINAL);
}

NGramTokenFilter::NGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram, bool preserveOriginal) : TokenFilter(input) {
    init(minGram, maxGram, preserveOriginal);
}

NGramTokenFilter::~NGramTokenFilter() {
}

void NGramTokenFilter::init(int32_t minGram, int32_t maxGram, bool preserveOriginal) {
    if (minGram < 1) {
        boost::throw_exception(IllegalArgumentException(L"minGram must be greater than zero"));
    }
    if (minGram > maxGram) {
        boost::throw_exception(IllegalArgumentException(L"minGram must not be greater than maxGram"));
    }

    this->minGram = minGram;
    this->maxGram = maxGram;
    this->preserveOriginal = preserveOriginal;

    curTermLength = 0;
    curGramSize = minGram;
    curPos = 0;
    curPosIncr = 0;

    termAtt = addAttribute<TermAttribute>();
    posIncrAtt = addAttribute<PositionIncrementAttribute>();
}

bool NGramTokenFilter::incrementToken() {
    while (true) {
        if (!curTermBuffer) {
            if (!input->incrementToken()) {
                return false;
            }

            state = captureState();
            curTermLength = termAtt->termLength();
            curPosIncr += posIncrAtt->getPositionIncrement();
            curPos = 0;

            if (preserveOriginal && curTermLength < minGram) {
                posIncrAtt->setPositionIncrement(curPosIncr);
                curPosIncr = 0;
                return true;
            }

            curTermBuffer = CharArray::newInstance(curTermLength);
            std::memcpy(curTermBuffer.get(), termAtt->termBufferArray(), curTermLength * sizeof(wchar_t));
            curGramSize = minGram;
        }

        if (curGramSize > maxGram || curPos + curGramSize > curTermLength) {
            ++curPos;
            curGramSize = minGram;
        }

        if (curPos + curGramSize <= curTermLength) {
            restoreState(state);
            termAtt->setTermBuffer(curTermBuffer.get(), curPos, curGramSize);
            posIncrAtt->setPositionIncrement(curPosIncr);
            curPosIncr = 0;
            ++curGramSize;
            return true;
        } else if (preserveOriginal && curTermLength > maxGram) {
            restoreState(state);
            termAtt->setTermBuffer(curTermBuffer.get(), 0, curTermLength);
            posIncrAtt->setPositionIncrement(0);
            curTermBuffer.reset();
            return true;
        }

        curTermBuffer.reset();
    }
}

void NGramTokenFilter::reset() {
    TokenFilter::reset();
    curTermBuffer.reset();
    curPosIncr = 0;
}

void NGramTokenFilter::end() {
    TokenFilter::end();
    posIncrAtt->setPositionIncrement(curPosIncr);
}

}
