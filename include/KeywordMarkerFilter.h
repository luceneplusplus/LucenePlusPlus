/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef KEYWORDMARKERFILTER_H
#define KEYWORDMARKERFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// Marks terms as keywords via the {@link KeywordAttribute}. Each token contained 
    /// in the provided is marked as a keyword by setting {@link 
    /// KeywordAttribute#setKeyword(bool )} to true.
    /// @see KeywordAttribute
    class LPPAPI KeywordMarkerFilter : public TokenFilter
    {
    public:
        /// Create a new KeywordMarkerFilter, that marks the current token as a keyword 
        /// if the tokens term buffer is contained in the given set via the {@link 
        /// KeywordAttribute}.
        /// @param input TokenStream to filter
        /// @param keywordSet the keywords set to lookup the current termbuffer
        KeywordMarkerFilter(TokenStreamPtr input, CharArraySetPtr keywordSet);
        
        /// Create a new KeywordMarkerFilter, that marks the current token as a keyword 
        /// if the tokens term buffer is contained in the given set via the {@link 
        /// KeywordAttribute}.
        /// @param input TokenStream to filter
        /// @param keywordSet the keywords set to lookup the current termbuffer
        KeywordMarkerFilter(TokenStreamPtr input, HashSet<String> keywordSet);
        
        virtual ~KeywordMarkerFilter();
        
        LUCENE_CLASS(KeywordMarkerFilter);
    
    protected:
        KeywordAttributePtr keywordAttr;
        CharTermAttributePtr termAtt;
        CharArraySetPtr keywordSet;
    
    public:
        virtual bool incrementToken();
    };
}

#endif
