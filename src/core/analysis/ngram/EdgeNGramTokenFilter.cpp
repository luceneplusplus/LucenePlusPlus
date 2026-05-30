/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "EdgeNGramTokenFilter.h"
#include "PositionIncrementAttribute.h"
#include "TermAttribute.h"

namespace Lucene {

const bool EdgeNGramTokenFilter::DEFAULT_PRESERVE_ORIGINAL = false;

EdgeNGramTokenFilter::EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t gramSize) : NGramTokenFilter(input, gramSize, gramSize, DEFAULT_PRESERVE_ORIGINAL) {
}

EdgeNGramTokenFilter::EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram) : NGramTokenFilter(input, minGram, maxGram, DEFAULT_PRESERVE_ORIGINAL) {
}

EdgeNGramTokenFilter::EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram, bool preserveOriginal)
    : NGramTokenFilter(input, minGram, maxGram, preserveOriginal) {
}

EdgeNGramTokenFilter::~EdgeNGramTokenFilter() {
}

bool EdgeNGramTokenFilter::incrementToken() {
    while (true) {
        if (!curTermBuffer) {
            if (!input->incrementToken()) {
                return false;
            }

            state = captureState();
            curTermLength = termAtt->termLength();
            curPosIncr += posIncrAtt->getPositionIncrement();

            if (preserveOriginal && curTermLength < minGram) {
                posIncrAtt->setPositionIncrement(curPosIncr);
                curPosIncr = 0;
                return true;
            }

            curTermBuffer = CharArray::newInstance(curTermLength);
            std::memcpy(curTermBuffer.get(), termAtt->termBufferArray(), curTermLength * sizeof(wchar_t));
            curGramSize = minGram;
        }

        if (curGramSize <= curTermLength) {
            if (curGramSize <= maxGram) {
                restoreState(state);
                termAtt->setTermBuffer(curTermBuffer.get(), 0, curGramSize);
                posIncrAtt->setPositionIncrement(curPosIncr);
                curPosIncr = 0;
                ++curGramSize;
                return true;
            } else if (preserveOriginal) {
                restoreState(state);
                termAtt->setTermBuffer(curTermBuffer.get(), 0, curTermLength);
                posIncrAtt->setPositionIncrement(0);
                curTermBuffer.reset();
                return true;
            }
        }

        curTermBuffer.reset();
    }
}

void EdgeNGramTokenFilter::reset() {
    NGramTokenFilter::reset();
}

void EdgeNGramTokenFilter::end() {
    NGramTokenFilter::end();
}

}
