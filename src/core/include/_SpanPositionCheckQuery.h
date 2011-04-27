/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SPANPOSITIONCHECKQUERY_H
#define _SPANPOSITIONCHECKQUERY_H

#include "Spans.h"

namespace Lucene
{
    class LPPAPI PositionCheckSpan : public Spans
    {
    public:
        PositionCheckSpan(SpanPositionCheckQueryPtr spanQuery, IndexReaderPtr reader);
        virtual ~PositionCheckSpan();
        
        LUCENE_CLASS(PositionCheckSpan);
    
    protected:
        SpanPositionCheckQueryWeakPtr _spanQuery;
        SpansPtr spans;
    
    public:
        virtual bool next();
        virtual bool skipTo(int32_t target);
        virtual int32_t doc();
        virtual int32_t start();
        virtual int32_t end();
        virtual Collection<ByteArray> getPayload();
        virtual bool isPayloadAvailable();
        virtual String toString();
    
    protected:
        virtual bool doNext();
    };
}

#endif
