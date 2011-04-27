/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANPOSITIONRANGEQUERY_H
#define SPANPOSITIONRANGEQUERY_H

#include "SpanPositionCheckQuery.h"

namespace Lucene
{
    /// Checks to see if the {@link #getMatch()} lies between a start and end position
    /// @see SpanFirstQuery for a derivation that is optimized for the case where start position is 0
    class LPPAPI SpanPositionRangeQuery : public SpanPositionCheckQuery
    {
    public:
        SpanPositionRangeQuery(SpanQueryPtr match, int32_t start, int32_t end);
        virtual ~SpanPositionRangeQuery();
        
        LUCENE_CLASS(SpanPositionRangeQuery);
    
    protected:
        int32_t start;
        int32_t end;
    
    public:
        /// @return The minimum position permitted in a match
        virtual int32_t getStart();
        
        /// @return the maximum end position permitted in a match.
        virtual int32_t getEnd();
        
        using SpanPositionCheckQuery::toString;
        virtual String toString(const String& field);
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        
    protected:
        virtual AcceptStatus acceptPosition(SpansPtr spans);
    };
}

#endif
