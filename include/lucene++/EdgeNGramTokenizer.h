/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef EDGENGRAMTOKENIZER_H
#define EDGENGRAMTOKENIZER_H

#include "NGramTokenizer.h"

namespace Lucene {

/// Tokenizes the input from the beginning edge into n-grams of the given size(s).
class LPPAPI EdgeNGramTokenizer : public NGramTokenizer {
public:
    static const int32_t DEFAULT_MIN_NGRAM_SIZE;
    static const int32_t DEFAULT_MAX_NGRAM_SIZE;

public:
    EdgeNGramTokenizer(const ReaderPtr& input);
    EdgeNGramTokenizer(const ReaderPtr& input, int32_t minGram, int32_t maxGram);
    EdgeNGramTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input, int32_t minGram, int32_t maxGram);
    EdgeNGramTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input, int32_t minGram, int32_t maxGram);

    virtual ~EdgeNGramTokenizer();

    LUCENE_CLASS(EdgeNGramTokenizer);

public:
    virtual bool incrementToken();
};

}

#endif
