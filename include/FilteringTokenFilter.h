/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FILTERINGTOKENFILTER_H
#define FILTERINGTOKENFILTER_H

#include "TokenFilter.h"

namespace Lucene
{
    /// Abstract base class for TokenFilters that may remove tokens. You have to implement 
    /// {@link #accept} and return a boolean if the current token should be preserved. 
    /// {@link #incrementToken} uses this method to decide if a token should be passed to 
    /// the caller.
    class LPPAPI FilteringTokenFilter : public TokenFilter
    {
    public:
        FilteringTokenFilter(bool enablePositionIncrements, TokenStreamPtr input);
        virtual ~FilteringTokenFilter();
        
        LUCENE_CLASS(FilteringTokenFilter);
    
    protected:
        PositionIncrementAttributePtr posIncrAtt;
        bool enablePositionIncrements;
    
    protected:
        /// Override this method and return if the current input token should be returned 
        /// by {@link #incrementToken}.
        virtual bool accept() = 0;

    public:
        virtual bool incrementToken();
        
        /// @see #setEnablePositionIncrements(bool)
        virtual bool getEnablePositionIncrements();
        
        /// If true, this TokenFilter will preserve positions of the incoming tokens (ie, 
        /// accumulate and set position increments of the removed tokens). Generally, true 
        /// is best as it does not lose information (positions of the original tokens) during 
        /// indexing.
        ///
        /// When set, when a token is stopped (omitted), the position increment of the following
        /// token is incremented.
        ///
        /// NOTE: be sure to also set {@link QueryParser#setEnablePositionIncrements} if you use 
        /// QueryParser to create queries.
        virtual void setEnablePositionIncrements(bool enable);
    };
}

#endif
