/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHSTEMFILTER_H
#define FRENCHSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that stems French words.
///
/// It supports a table of words that should not be stemmed at all.  The stemmer used can
/// be changed at runtime after the filter object is created (as long as it is a
/// {@link FrenchStemmer}).
///
/// NOTE: This stemmer does not implement the Snowball algorithm correctly, especially
/// involving case problems. It is recommended that you consider using the "French" stemmer
/// in the snowball package instead. This stemmer will likely be deprecated in a future release.
class LPPCONTRIBAPI FrenchStemFilter : public TokenFilter {
public:
    FrenchStemFilter(const TokenStreamPtr& input);

    /// Builds a FrenchStemFilter that uses an exclusion table.
    FrenchStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable);

    virtual ~FrenchStemFilter();

    LUCENE_CLASS(FrenchStemFilter);

protected:
    /// {@link FrenchStemmer} in use by this filter.
    FrenchStemmerPtr stemmer;

    HashSet<String> exclusions;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();

    /// Set a alternative/custom {@link FrenchStemmer} for this filter.
    void setStemmer(const FrenchStemmerPtr& stemmer);

    /// Set an alternative exclusion list for this filter.
    void setExclusionSet(HashSet<String> exclusiontable);
};

}

#endif
