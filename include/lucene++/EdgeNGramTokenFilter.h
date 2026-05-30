/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef EDGENGRAMTOKENFILTER_H
#define EDGENGRAMTOKENFILTER_H

#include "NGramTokenFilter.h"

namespace Lucene {

/// Tokenizes each input token from the beginning edge into n-grams of the given size(s).
class LPPAPI EdgeNGramTokenFilter : public NGramTokenFilter {
public:
    static const bool DEFAULT_PRESERVE_ORIGINAL;

public:
    EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t gramSize);
    EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram);
    EdgeNGramTokenFilter(const TokenStreamPtr& input, int32_t minGram, int32_t maxGram, bool preserveOriginal);

    virtual ~EdgeNGramTokenFilter();

    LUCENE_CLASS(EdgeNGramTokenFilter);

public:
    virtual bool incrementToken();
    virtual void reset();
    virtual void end();
};

}

#endif
