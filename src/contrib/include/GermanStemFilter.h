/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GERMANSTEMFILTER_H
#define GERMANSTEMFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// A {@link TokenFilter} that stems German words. 
    ///
    /// It supports a table of words that should not be stemmed at all.  The stemmer used 
    /// can be changed at runtime after the filter object is created (as long as it is a 
    /// {@link GermanStemmer}).
    ///
    /// To prevent terms from being stemmed use an instance of {@link KeywordMarkerFilter}
    /// or a custom {@link TokenFilter} that sets the {@link KeywordAttribute} before this 
    /// {@link TokenStream}.
    /// @see KeywordMarkerFilter
    class LPPCONTRIBAPI GermanStemFilter : public TokenFilter
    {
    public:
        /// Creates a {@link GermanStemFilter} instance
        /// @param in the source {@link TokenStream}
        GermanStemFilter(TokenStreamPtr input);
        
        /// Builds a GermanStemFilter that uses an exclusion table.
        /// @deprecated use {@link KeywordAttribute} with {@link KeywordMarkerFilter} instead.
        GermanStemFilter(TokenStreamPtr input, HashSet<String> exclusionSet);
        
        virtual ~GermanStemFilter();
        
        LUCENE_CLASS(GermanStemFilter);
    
    protected:
        /// {@link GermanStemmer} in use by this filter.
        GermanStemmerPtr stemmer;
        
        HashSet<String> exclusionSet;
        CharTermAttributePtr termAtt;
        KeywordAttributePtr keywordAttr;
    
    public:
        virtual bool incrementToken();
        
        /// Set a alternative/custom {@link GermanStemmer} for this filter.
        void setStemmer(GermanStemmerPtr stemmer);
        
        /// Set an alternative exclusion list for this filter.
        /// @deprecated use {@link KeywordAttribute} with {@link KeywordMarkerFilter} instead.
        void setExclusionSet(HashSet<String> exclusionSet);
    };
}

#endif
