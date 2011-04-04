/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANFIRSTQUERY_H
#define SPANFIRSTQUERY_H

#include "SpanQuery.h"
#include "Spans.h"

namespace Lucene
{
    /// Matches spans near the beginning of a field.
    class LPPAPI SpanFirstQuery : public SpanQuery
    {
    public:
        /// Construct a SpanFirstQuery matching spans in match whose end position is less than or equal to end.
        SpanFirstQuery(SpanQueryPtr match, int32_t end);
        virtual ~SpanFirstQuery();
        
        LUCENE_CLASS(SpanFirstQuery);
    
    protected:
        SpanQueryPtr match;
        int32_t end;
    
    public:
        using SpanQuery::toString;
        
        /// Return the SpanQuery whose matches are filtered.
        SpanQueryPtr getMatch();
        
        /// Return the maximum end position permitted in a match.
        int32_t getEnd();
        
        virtual String getField();
        virtual String toString(const String& field);
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual void extractTerms(SetTerm terms);
        virtual SpansPtr getSpans(IndexReaderPtr reader);
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        
        friend class FirstSpans;
    };
}

#endif
