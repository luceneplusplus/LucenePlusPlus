/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NGramAnalyzer.h"
#include "NGramTokenizer.h"

namespace Lucene {

NGramAnalyzer::NGramAnalyzer() {
    minGram = NGramTokenizer::DEFAULT_MIN_NGRAM_SIZE;
    maxGram = NGramTokenizer::DEFAULT_MAX_NGRAM_SIZE;
}

NGramAnalyzer::NGramAnalyzer(int32_t minGram, int32_t maxGram) {
    if (minGram < 1) {
        boost::throw_exception(IllegalArgumentException(L"minGram must be greater than zero"));
    }
    if (minGram > maxGram) {
        boost::throw_exception(IllegalArgumentException(L"minGram must not be greater than maxGram"));
    }
    this->minGram = minGram;
    this->maxGram = maxGram;
}

NGramAnalyzer::~NGramAnalyzer() {
}

TokenStreamPtr NGramAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader) {
    return newLucene<NGramTokenizer>(reader, minGram, maxGram);
}

TokenStreamPtr NGramAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
    TokenizerPtr tokenizer(boost::dynamic_pointer_cast<Tokenizer>(getPreviousTokenStream()));
    if (!tokenizer) {
        tokenizer = newLucene<NGramTokenizer>(reader, minGram, maxGram);
        setPreviousTokenStream(tokenizer);
    } else {
        tokenizer->reset(reader);
    }
    return tokenizer;
}

}
