/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CLASSICFILTER_H
#define CLASSICFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// Normalizes tokens extracted with {@link ClassicTokenizer}.
    class LPPAPI ClassicFilter : public TokenFilter
    {
    public:
        /// Construct filtering input.
        ClassicFilter(TokenStreamPtr input);
        virtual ~ClassicFilter();
        
        LUCENE_CLASS(ClassicFilter);
    
    protected:
        TypeAttributePtr typeAtt;
        CharTermAttributePtr termAtt;
    
    protected:
        static const String& APOSTROPHE_TYPE();
        static const String& ACRONYM_TYPE();
    
    public:
        /// Returns the next token in the stream, or null at EOS.
        ///
        /// Removes <tt>'s</tt> from the end of words.
        /// Removes dots from acronyms.
        virtual bool incrementToken();
    };
}

#endif
