/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LengthFilter.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    LengthFilter::LengthFilter(bool enablePositionIncrements, TokenStreamPtr input, int32_t min, int32_t max) : FilteringTokenFilter(enablePositionIncrements, input)
    {
        this->min = min;
        this->max = max;
        this->termAtt = addAttribute<CharTermAttribute>();
    }
    
    LengthFilter::LengthFilter(TokenStreamPtr input, int32_t min, int32_t max) : FilteringTokenFilter(false, input)
    {
        this->min = min;
        this->max = max;
        this->termAtt = addAttribute<CharTermAttribute>();
    }
    
    LengthFilter::~LengthFilter()
    {
    }
    
    bool LengthFilter::accept()
    {
        int32_t len = termAtt->length();
        return (len >= min && len <= max);
    }
}
