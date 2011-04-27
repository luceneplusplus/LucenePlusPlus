/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LIMITTOKENCOUNTFILTER_H
#define LIMITTOKENCOUNTFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// This TokenFilter limits the number of tokens while indexing. It is a 
    /// replacement for the maximum field length setting inside {@link IndexWriter}.
    class LPPAPI LimitTokenCountFilter : public TokenFilter
    {
    public:
        /// Build a filter that only accepts tokens up to a maximum number.
        LimitTokenCountFilter(TokenStreamPtr input, int32_t maxTokenCount);
        
        virtual ~LimitTokenCountFilter();
        
        LUCENE_CLASS(LimitTokenCountFilter);
    
    private:
        int32_t maxTokenCount;
        int32_t tokenCount;
    
    public:
        virtual bool incrementToken();
        virtual void reset();
    };
}

#endif
