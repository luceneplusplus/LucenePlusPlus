/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHSTEMFILTER_H
#define FRENCHSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that stems French words.
    ///
    /// The used stemmer can be changed at runtime after the filter object is
    /// created (as long as it is a {@link FrenchStemmer}).
    ///
    /// To prevent terms from being stemmed use an instance of {@link
    /// KeywordMarkerFilter} or a custom {@link TokenFilter} that sets the {@link
    /// KeywordAttribute} before this {@link TokenStream}.
    ///
    /// @see KeywordMarkerFilter
    /// @deprecated Use {@link SnowballFilter} with {@link FrenchStemmer}
    /// instead, which has the same functionality.
    class LPPCONTRIBAPI FrenchStemFilter : public TokenFilter
    {
    public:
        FrenchStemFilter(TokenStreamPtr input);

        /// Builds a FrenchStemFilter that uses an exclusion table.
        /// @param in the {@link TokenStream} to filter
        /// @param exclusiontable a set of terms not to be stemmed
        /// @deprecated use {@link KeywordAttribute} with {@link KeywordMarkerFilter}
        /// instead.
        FrenchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable);

        virtual ~FrenchStemFilter();

        LUCENE_CLASS(FrenchStemFilter);

    protected:
        /// {@link FrenchStemmer} in use by this filter.
        FrenchStemmerPtr stemmer;

        HashSet<String> exclusions;
        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;

    public:
        virtual bool incrementToken();

        /// Set a alternative/custom {@link FrenchStemmer} for this filter.
        void setStemmer(FrenchStemmerPtr stemmer);

        /// Set an alternative exclusion list for this filter.
        /// @deprecated use {@link KeywordAttribute} with {@link KeywordMarkerFilter}
        /// instead.
        void setExclusionSet(HashSet<String> exclusiontable);
    };
}

#endif

