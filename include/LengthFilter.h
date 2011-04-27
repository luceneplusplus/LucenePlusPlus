/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LENGTHFILTER_H
#define LENGTHFILTER_H

#include "FilteringTokenFilter.h"

namespace Lucene
{
    /// Removes words that are too long or too short from the stream.
    class LPPAPI LengthFilter : public FilteringTokenFilter
    {
    public:
        /// Build a filter that removes words that are too long or too short from the text.
        LengthFilter(bool enablePositionIncrements, TokenStreamPtr input, int32_t min, int32_t max);
        
        /// Build a filter that removes words that are too long or too short from the text.
        /// @deprecated Use {@link #LengthFilter(boolean, TokenStream, int, int)} instead.
        LengthFilter(TokenStreamPtr input, int32_t min, int32_t max);
        
        virtual ~LengthFilter();
        
        LUCENE_CLASS(LengthFilter);
    
    protected:
        int32_t min;
        int32_t max;
        CharTermAttributePtr termAtt;
    
    public:
        virtual bool accept();
    };
}

#endif
