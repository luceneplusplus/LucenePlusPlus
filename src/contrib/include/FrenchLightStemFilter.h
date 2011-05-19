/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHLIGHTSTEMFILTER_H
#define FRENCHLIGHTSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that applies {@link FrenchLightStemmer} to stem French words.
	///
	/// To prevent terms from being stemmed use an instance of {@link KeywordMarkerFilter}
	/// or a custom {@link TokenFilter} that sets the {@link KeywordAttribute} before this
	/// {@link TokenStream}.
    class LPPCONTRIBAPI FrenchLightStemFilter : public TokenFilter
    {
    public:
        FrenchLightStemFilter(TokenStreamPtr input);
        virtual ~FrenchLightStemFilter();

        LUCENE_CLASS(FrenchLightStemFilter);

    protected:
        FrenchLightStemmerPtr stemmer;
        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;

    public:
        virtual bool incrementToken();
    };
}

#endif

