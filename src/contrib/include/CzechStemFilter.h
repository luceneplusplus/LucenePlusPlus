/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CZECHSTEMFILTER_H
#define CZECHSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that applies {@link CzechStemmer} to stem Czech words.
    ///
    /// To prevent terms from being stemmed use an instance of {@link 
    /// KeywordMarkerFilter} or a custom {@link TokenFilter} that sets the {@link 
    /// KeywordAttribute} before this {@link TokenStream}.
    ///
    /// NOTE: Input is expected to be in lowercase, but with diacritical marks
    /// @see KeywordMarkerFilter
    class LPPCONTRIBAPI CzechStemFilter : public TokenFilter
    {
    public:
        /// Creates a new CzechStemFilter 
        /// @param input the source {@link TokenStream} 
        CzechStemFilter(TokenStreamPtr input);
        
        virtual ~CzechStemFilter();
        
        LUCENE_CLASS(CzechStemFilter);
    
    private:
        /// {@link BrazilianStemmer} in use by this filter.
        CzechStemmerPtr stemmer;
        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;
    
    public:
        virtual bool incrementToken();
    };
}

#endif
