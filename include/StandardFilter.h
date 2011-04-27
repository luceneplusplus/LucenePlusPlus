/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STANDARDFILTER_H
#define STANDARDFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// Normalizes tokens extracted with {@link StandardTokenizer}.
    class LPPAPI StandardFilter : public TokenFilter
    {
    public:
        /// @deprecated Use {@link #StandardFilter(Version, TokenStream)} instead.
        StandardFilter(TokenStreamPtr input);
        
        StandardFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input);
        
        virtual ~StandardFilter();
        
        LUCENE_CLASS(StandardFilter);
    
    protected:
        LuceneVersion::Version matchVersion;
        TypeAttributePtr typeAtt;
        CharTermAttributePtr termAtt;
    
    protected:
        static const String& APOSTROPHE_TYPE();
        static const String& ACRONYM_TYPE();
    
    public:
        virtual bool incrementToken();
        virtual bool incrementTokenClassic();
    };
}

#endif
