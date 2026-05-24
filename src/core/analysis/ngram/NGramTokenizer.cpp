/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NGramTokenizer.h"
#include "CharReader.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "Reader.h"

namespace Lucene {

const int32_t NGramTokenizer::DEFAULT_MIN_NGRAM_SIZE = 1;
const int32_t NGramTokenizer::DEFAULT_MAX_NGRAM_SIZE = 2;

NGramTokenizer::NGramTokenizer(const ReaderPtr& input) : Tokenizer(input) {
    init(DEFAULT_MIN_NGRAM_SIZE, DEFAULT_MAX_NGRAM_SIZE);
}

NGramTokenizer::NGramTokenizer(const ReaderPtr& input, int32_t minGram, int32_t maxGram) : Tokenizer(input) {
    init(minGram, maxGram);
}

NGramTokenizer::NGramTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input, int32_t minGram, int32_t maxGram) : Tokenizer(source, input) {
    init(minGram, maxGram);
}

NGramTokenizer::NGramTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input, int32_t minGram, int32_t maxGram) : Tokenizer(factory, input) {
    init(minGram, maxGram);
}

NGramTokenizer::~NGramTokenizer() {
}

void NGramTokenizer::init(int32_t minGram, int32_t maxGram) {
    if (minGram < 1) {
        boost::throw_exception(IllegalArgumentException(L"minGram must be greater than zero"));
    }
    if (minGram > maxGram) {
        boost::throw_exception(IllegalArgumentException(L"minGram must not be greater than maxGram"));
    }

    this->minGram = minGram;
    this->maxGram = maxGram;
    buffer = CharArray::newInstance(2 * maxGram + 1024);

    termAtt = addAttribute<TermAttribute>();
    termAtt->resizeTermBuffer(maxGram);
    offsetAtt = addAttribute<OffsetAttribute>();
    posIncrAtt = addAttribute<PositionIncrementAttribute>();

    resetState();
}

void NGramTokenizer::resetState() {
    bufferStart = bufferEnd = buffer.size();
    lastNonTokenChar = lastCheckedChar = bufferStart - 1;
    offset = 0;
    gramSize = minGram;
    exhausted = false;
}

void NGramTokenizer::fillBuffer() {
    if (exhausted || bufferStart < bufferEnd - maxGram - 1) {
        return;
    }

    int32_t remaining = bufferEnd - bufferStart;
    if (remaining > 0 && bufferStart > 0) {
        std::memmove(buffer.get(), buffer.get() + bufferStart, remaining * sizeof(wchar_t));
    }
    bufferEnd = remaining;
    lastCheckedChar -= bufferStart;
    lastNonTokenChar -= bufferStart;
    bufferStart = 0;

    while (bufferEnd < buffer.size()) {
        int32_t length = input->read(buffer.get(), bufferEnd, buffer.size() - bufferEnd);
        if (length == -1) {
            exhausted = true;
            break;
        }
        bufferEnd += length;
    }
}

void NGramTokenizer::consume() {
    ++bufferStart;
    ++offset;
}

void NGramTokenizer::updateLastNonTokenChar() {
    int32_t termEnd = bufferStart + gramSize - 1;
    if (termEnd > lastCheckedChar) {
        for (int32_t i = termEnd; i > lastCheckedChar; --i) {
            if (!isTokenChar(buffer[i])) {
                lastNonTokenChar = i;
                break;
            }
        }
        lastCheckedChar = termEnd;
    }
}

bool NGramTokenizer::isTokenChar(wchar_t chr) {
    return true;
}

bool NGramTokenizer::incrementToken() {
    clearAttributes();

    while (true) {
        fillBuffer();

        if (gramSize > maxGram || bufferStart + gramSize > bufferEnd) {
            if (bufferStart + 1 + minGram > bufferEnd) {
                if (exhausted) {
                    return false;
                }
                continue;
            }
            consume();
            gramSize = minGram;
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

void NGramTokenizer::end() {
    int32_t endOffset = offset;
    for (int32_t i = bufferStart; i < bufferEnd; ++i) {
        ++endOffset;
    }
    endOffset = correctOffset(endOffset);
    offsetAtt->setOffset(endOffset, endOffset);
}

void NGramTokenizer::reset() {
    Tokenizer::reset(input);
    resetState();
}

void NGramTokenizer::reset(const ReaderPtr& input) {
    this->input = CharReader::get(input);
    this->charStream = boost::dynamic_pointer_cast<CharStream>(this->input);
    resetState();
}

}
