/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LOWERCASEFILTER_H
#define LOWERCASEFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// Normalizes token text to lower case.
    /// You must specify the required {@link Version} compatibility when creating 
    /// LowerCaseFilter:
    /// <ul>
    ///    <li> As of 3.1, supplementary characters are properly lowercased.
    /// </ul>
    class LPPAPI LowerCaseFilter : public TokenFilter
    {
    public:
        /// Create a new LowerCaseFilter, that normalizes token text to lower case.
        /// @param matchVersion Lucene version to match
        /// @param in TokenStream to filter
        LowerCaseFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input);
        
        /// @deprecated Use {@link #LowerCaseFilter(Version, TokenStream)} instead.
        LowerCaseFilter(TokenStreamPtr input);
        
        virtual ~LowerCaseFilter();
        
        LUCENE_CLASS(LowerCaseFilter);
    
    protected:
        CharTermAttributePtr termAtt;
    
    public:
        virtual bool incrementToken();
    };
}

#endif
