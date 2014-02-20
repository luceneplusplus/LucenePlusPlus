/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GERMANSTEMFILTER_H
#define GERMANSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that stems German words.
///
/// It supports a table of words that should not be stemmed at all.  The stemmer used can
/// be changed at runtime after the filter object is created (as long as it is a
/// {@link GermanStemmer}).
class LPPCONTRIBAPI GermanStemFilter : public TokenFilter {
public:
    GermanStemFilter(const TokenStreamPtr& input);

    /// Builds a GermanStemFilter that uses an exclusion table.
    GermanStemFilter(const TokenStreamPtr& input, HashSet<String> exclusionSet);

    virtual ~GermanStemFilter();

    LUCENE_CLASS(GermanStemFilter);

protected:
    /// {@link GermanStemmer} in use by this filter.
    GermanStemmerPtr stemmer;

    HashSet<String> exclusionSet;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();

    /// Set a alternative/custom {@link GermanStemmer} for this filter.
    void setStemmer(const GermanStemmerPtr& stemmer);

    /// Set an alternative exclusion list for this filter.
    void setExclusionSet(HashSet<String> exclusionSet);
};

}

#endif
