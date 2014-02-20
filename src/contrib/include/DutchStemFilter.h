/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DUTCHSTEMFILTER_H
#define DUTCHSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that stems Dutch words.
///
/// It supports a table of words that should not be stemmed at all.  The stemmer used can
/// be changed at runtime after the filter object is created (as long as it is a
/// {@link DutchStemmer}).
///
/// NOTE: This stemmer does not implement the Snowball algorithm correctly, specifically
/// doubled consonants. It is recommended that you consider using the "Dutch" stemmer in
/// the snowball package instead. This stemmer will likely be deprecated in a future release.
class LPPCONTRIBAPI DutchStemFilter : public TokenFilter {
public:
    DutchStemFilter(const TokenStreamPtr& input);

    /// Builds a DutchStemFilter that uses an exclusion table.
    DutchStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable);

    /// Builds a DutchStemFilter that uses an exclusion table and dictionary of word stem
    /// pairs, that overrule the algorithm.
    DutchStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable, MapStringString stemdictionary);

    virtual ~DutchStemFilter();

    LUCENE_CLASS(DutchStemFilter);

protected:
    /// {@link DutchStemmer} in use by this filter.
    DutchStemmerPtr stemmer;

    HashSet<String> exclusions;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();

    /// Set a alternative/custom {@link DutchStemmer} for this filter.
    void setStemmer(const DutchStemmerPtr& stemmer);

    /// Set an alternative exclusion list for this filter.
    void setExclusionSet(HashSet<String> exclusiontable);

    /// Set dictionary for stemming, this dictionary overrules the algorithm, so you can
    /// correct for a particular unwanted word-stem pair.
    void setStemDictionary(MapStringString dict);
};

}

#endif
