/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANNEARPAYLOADCHECKQUERY_H
#define SPANNEARPAYLOADCHECKQUERY_H

#include "SpanPositionCheckQuery.h"

namespace Lucene
{
    /// Only return those matches that have a specific payload at the given position.
    class LPPAPI SpanNearPayloadCheckQuery : public SpanPositionCheckQuery
    {
    public:
        SpanNearPayloadCheckQuery(SpanNearQueryPtr match, Collection<ByteArray> payloadToMatch);
        virtual ~SpanNearPayloadCheckQuery();
        
        LUCENE_CLASS(SpanNearPayloadCheckQuery);
    
    protected:
        Collection<ByteArray> payloadToMatch;
    
    public:
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
