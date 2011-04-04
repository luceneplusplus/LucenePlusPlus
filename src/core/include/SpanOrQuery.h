/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANORQUERY_H
#define SPANORQUERY_H

#include "SpanQuery.h"

namespace Lucene
{
    /// Matches the union of its clauses.
    class LPPAPI SpanOrQuery : public SpanQuery
    {
    public:
        /// Construct a SpanOrQuery merging the provided clauses.
        SpanOrQuery(Collection<SpanQueryPtr> clauses);
        virtual ~SpanOrQuery();
        
        LUCENE_CLASS(SpanOrQuery);
    
    protected:
        Collection<SpanQueryPtr> clauses;
        String field;
    
    public:
        using SpanQuery::toString;
        
        /// Return the clauses whose spans are matched.
        Collection<SpanQueryPtr> getClauses();
    
        virtual String getField();
        virtual void extractTerms(SetTerm terms);
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        virtual String toString(const String& field);
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();        
        virtual SpansPtr getSpans(IndexReaderPtr reader);
        
        friend class OrSpans;
    };
}

#endif
