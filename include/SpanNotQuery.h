/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANNOTQUERY_H
#define SPANNOTQUERY_H

#include "SpanQuery.h"
#include "Spans.h"

namespace Lucene
{
    /// Removes matches which overlap with another SpanQuery.
    class LPPAPI SpanNotQuery : public SpanQuery
    {
    public:
        /// Construct a SpanNotQuery matching spans from include which have no overlap with spans from exclude.
        SpanNotQuery(SpanQueryPtr include, SpanQueryPtr exclude);
        virtual ~SpanNotQuery();
        
        LUCENE_CLASS(SpanNotQuery);
    
    protected:
        SpanQueryPtr include;
        SpanQueryPtr exclude;
    
    public:
        using SpanQuery::toString;
        
        /// Return the SpanQuery whose matches are filtered.
        SpanQueryPtr getInclude();
        
        /// Return the SpanQuery whose matches must not overlap those returned.
        SpanQueryPtr getExclude();
        
        virtual String getField();
        virtual void extractTerms(SetTerm terms);
        virtual String toString(const String& field);
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual SpansPtr getSpans(IndexReaderPtr reader);
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
    };
    
    class LPPAPI NotSpans : public Spans
    {
    public:
        NotSpans(SpanNotQueryPtr query, SpansPtr includeSpans, SpansPtr excludeSpans);
        virtual ~NotSpans();
        
        LUCENE_CLASS(NotSpans);
    
    protected:
        SpanNotQueryPtr query;
        SpansPtr includeSpans;
        bool moreInclude;
        SpansPtr excludeSpans;
        bool moreExclude;
    
    public:
        virtual bool next();
        virtual bool skipTo(int32_t target);
        virtual int32_t doc();
        virtual int32_t start();
        virtual int32_t end();
        virtual Collection<ByteArray> getPayload();
        virtual bool isPayloadAvailable();
        virtual String toString();
    };
}

#endif
