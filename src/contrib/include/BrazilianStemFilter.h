/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BRAZILIANSTEMFILTER_H
#define BRAZILIANSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that applies {@link BrazilianStemmer}.
class LPPCONTRIBAPI BrazilianStemFilter : public TokenFilter {
public:
    BrazilianStemFilter(const TokenStreamPtr& input);
    BrazilianStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable);

    virtual ~BrazilianStemFilter();

    LUCENE_CLASS(BrazilianStemFilter);

protected:
    /// {@link BrazilianStemmer} in use by this filter.
    BrazilianStemmerPtr stemmer;

    HashSet<String> exclusions;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();
};

}

#endif
