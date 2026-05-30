/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "EdgeNGramTokenizer.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "TermAttribute.h"

namespace Lucene {

const int32_t EdgeNGramTokenizer::DEFAULT_MIN_NGRAM_SIZE = 1;
const int32_t EdgeNGramTokenizer::DEFAULT_MAX_NGRAM_SIZE = 1;

EdgeNGramTokenizer::EdgeNGramTokenizer(const ReaderPtr& input) : NGramTokenizer(input, DEFAULT_MIN_NGRAM_SIZE, DEFAULT_MAX_NGRAM_SIZE) {
}

EdgeNGramTokenizer::EdgeNGramTokenizer(const ReaderPtr& input, int32_t minGram, int32_t maxGram) : NGramTokenizer(input, minGram, maxGram) {
}

EdgeNGramTokenizer::EdgeNGramTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input, int32_t minGram, int32_t maxGram)
    : NGramTokenizer(source, input, minGram, maxGram) {
}

EdgeNGramTokenizer::EdgeNGramTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input, int32_t minGram, int32_t maxGram)
    : NGramTokenizer(factory, input, minGram, maxGram) {
}

EdgeNGramTokenizer::~EdgeNGramTokenizer() {
}

bool EdgeNGramTokenizer::incrementToken() {
    clearAttributes();

    while (true) {
        fillBuffer();

        if (gramSize > maxGram || bufferStart + gramSize > bufferEnd) {
            if (exhausted) {
                return false;
            }
            continue;
        }

        updateLastNonTokenChar();
        if (lastNonTokenChar >= bufferStart && lastNonTokenChar < bufferStart + gramSize) {
            consume();
            gramSize = minGram;
            continue;
        }

        termAtt->setTermBuffer(buffer.get(), bufferStart, gramSize);
        posIncrAtt->setPositionIncrement(1);
        offsetAtt->setOffset(correctOffset(offset), correctOffset(offset + gramSize));
        ++gramSize;
        return true;
    }
}

}
