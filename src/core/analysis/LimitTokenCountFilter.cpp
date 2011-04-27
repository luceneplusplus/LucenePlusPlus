/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LimitTokenCountFilter.h"

namespace Lucene
{
    LimitTokenCountFilter::LimitTokenCountFilter(TokenStreamPtr input, int32_t maxTokenCount) : TokenFilter(input)
    {
        this->maxTokenCount = maxTokenCount;
        this->tokenCount = 0;
    }
    
    LimitTokenCountFilter::~LimitTokenCountFilter()
    {
    }
    
    bool LimitTokenCountFilter::incrementToken()
    {
        if (tokenCount < maxTokenCount && input->incrementToken())
        {
            ++tokenCount;
            return true;
        }
        return false;
    }
    
    void LimitTokenCountFilter::reset()
    {
        TokenFilter::reset();
        tokenCount = 0;
    }
}
