/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANSTEMFILTER_H
#define RUSSIANSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that stems Russian words.
///
/// The implementation was inspired by GermanStemFilter.
///
/// The input should be filtered by {@link LowerCaseFilter} before passing it to RussianStemFilter,
/// because RussianStemFilter only works with lowercase characters.
class LPPCONTRIBAPI RussianStemFilter : public TokenFilter {
public:
    RussianStemFilter(const TokenStreamPtr& input);

    virtual ~RussianStemFilter();

    LUCENE_CLASS(RussianStemFilter);

protected:
    /// {@link RussianStemmer} in use by this filter.
    RussianStemmerPtr stemmer;

    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();

    /// Set a alternative/custom {@link RussianStemmer} for this filter.
    void setStemmer(const RussianStemmerPtr& stemmer);
};

}

#endif
