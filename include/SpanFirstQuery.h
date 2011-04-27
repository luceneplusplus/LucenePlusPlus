/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANFIRSTQUERY_H
#define SPANFIRSTQUERY_H

#include "SpanPositionRangeQuery.h"

namespace Lucene
{
    /// Matches spans near the beginning of a field.
    ///
    /// This class is a simple extension of {@link SpanPositionRangeQuery} in that it assumes the start to 
    /// be zero and only checks the end boundary.
    class LPPAPI SpanFirstQuery : public SpanPositionRangeQuery
    {
    public:
        /// Construct a SpanFirstQuery matching spans in match whose end position is less than or equal to end.
        SpanFirstQuery(SpanQueryPtr match, int32_t end);
        virtual ~SpanFirstQuery();
        
        LUCENE_CLASS(SpanFirstQuery);
    
    public:
        using SpanPositionRangeQuery::toString;
        virtual String toString(const String& field);
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
    
    protected:
        virtual AcceptStatus acceptPosition(SpansPtr spans);
    };
}

#endif
