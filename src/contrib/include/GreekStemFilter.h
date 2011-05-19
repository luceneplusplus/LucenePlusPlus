/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GREEKSTEMFILTER_H
#define GREEKSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that applies {@link GreekStemmer} to stem Greek words.
    ///
    /// To prevent terms from being stemmed use an instance of {@link
    /// KeywordMarkerFilter} or a custom {@link TokenFilter} that sets the {@link
    /// KeywordAttribute} before this {@link TokenStream}.
    ///
    /// NOTE: Input is expected to be casefolded for Greek (including folding of
    /// final sigma to sigma), and with diacritics removed. This can be achieved by
    /// using {@link GreekLowerCaseFilter} before GreekStemFilter.
    class LPPCONTRIBAPI GreekStemFilter : public TokenFilter
    {
    public:
        GreekStemFilter(TokenStreamPtr input);
        virtual ~GreekStemFilter();

        LUCENE_CLASS(GreekStemFilter);

    protected:
        /// {@link GreekStemmer} in use by this filter.
        GreekStemmerPtr stemmer;

        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;

    public:
        virtual bool incrementToken();
    };
}

#endif

