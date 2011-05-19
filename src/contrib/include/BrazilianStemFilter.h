/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BRAZILIANSTEMFILTER_H
#define BRAZILIANSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that applies {@link BrazilianStemmer}.
    ///
    /// To prevent terms from being stemmed use an instance of {@link KeywordMarkerFilter}
    /// or a custom {@link TokenFilter} that sets the {@link KeywordAttribute} before this
    /// {@link TokenStream}.
    ///
    /// @see KeywordMarkerFilter
    class LPPCONTRIBAPI BrazilianStemFilter : public TokenFilter
    {
    public:
        /// Creates a new BrazilianStemFilter
        /// @param input the source {@link TokenStream}
        BrazilianStemFilter(TokenStreamPtr input);

        /// Creates a new BrazilianStemFilter
        /// @param input the source {@link TokenStream}
        /// @param exclusiontable a set of terms that should be prevented from being stemmed.
        /// @deprecated use {@link KeywordAttribute} with {@link KeywordMarkerFilter} instead.
        BrazilianStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable);

        virtual ~BrazilianStemFilter();

        LUCENE_CLASS(BrazilianStemFilter);

    protected:
        /// {@link BrazilianStemmer} in use by this filter.
        BrazilianStemmerPtr stemmer;

        HashSet<String> exclusions;
        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;

    public:
        virtual bool incrementToken();
    };
}

#endif

