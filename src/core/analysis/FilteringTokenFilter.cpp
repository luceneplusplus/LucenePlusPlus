/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FilteringTokenFilter.h"
#include "PositionIncrementAttribute.h"

namespace Lucene
{
    FilteringTokenFilter::FilteringTokenFilter(bool enablePositionIncrements, TokenStreamPtr input) : TokenFilter(input)
    {
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        this->enablePositionIncrements = enablePositionIncrements;
    }
    
    FilteringTokenFilter::~FilteringTokenFilter()
    {
    }
    
    bool FilteringTokenFilter::incrementToken()
    {
        if (enablePositionIncrements)
        {
            int32_t skippedPositions = 0;
            while (input->incrementToken())
            {
                if (accept())
                {
                    if (skippedPositions != 0)
                        posIncrAtt->setPositionIncrement(posIncrAtt->getPositionIncrement() + skippedPositions);
                    return true;
                }
                skippedPositions += posIncrAtt->getPositionIncrement();
            }
        }
        else
        {
            while (input->incrementToken())
            {
                if (accept())
                    return true;
            }
        }
        // reached EOS -- return false
        return false;
    }
    
    bool FilteringTokenFilter::getEnablePositionIncrements()
    {
        return enablePositionIncrements;
    }
    
    void FilteringTokenFilter::setEnablePositionIncrements(bool enable)
    {
        this->enablePositionIncrements = enable;
    }
}
