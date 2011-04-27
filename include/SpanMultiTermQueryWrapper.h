/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SPANMULTITERMQUERYWRAPPER_H
#define SPANMULTITERMQUERYWRAPPER_H

#include "SpanQuery.h"
#include "SpanOrQuery.h"
#include "MultiTermQuery.h"
#include "ScoringRewrite.h"
#include "TopTermsRewrite.h"

namespace Lucene
{
    /// Wraps any {@link MultiTermQuery} as a {@link SpanQuery}, so it can be 
    /// nested within other SpanQuery classes.
    ///
    /// The query is rewritten by default to a {@link SpanOrQuery} containing
    /// the expanded terms, but this can be customized. 
    ///
    /// Example:
    /// <pre>
    /// WildcardQueryPtr wildcard = newLucene<WildcardQuery>(newLucene<Term>(L"field", L"bro?n"));
    /// SpanQueryPtr spanWildcard = newLucene<SpanMultiTermQueryWrapper>(wildcard);
    ///     // do something with spanWildcard, such as use it in a SpanFirstQuery
    /// </pre>
    class LPPAPI SpanMultiTermQueryWrapper : public SpanQuery
    {
    public:
        /// Create a new SpanMultiTermQueryWrapper. 
        /// @param query Query to wrap.
        ///
        /// NOTE: This will call {@link MultiTermQuery#setRewriteMethod(MultiTermQuery.RewriteMethod)}
        /// on the wrapped query, changing its rewrite method to a suitable one for spans.
        /// Be sure to not change the rewrite method on the wrapped query afterwards! Doing so will
        /// In Lucene 3.x, MultiTermQuery allows queries to rewrite to different field names, but 
        /// SpanQuery needs a fixed field. The wrapped query must therefore support getField() or getTerm().
        SpanMultiTermQueryWrapper(MultiTermQueryPtr query);
        
        virtual ~SpanMultiTermQueryWrapper();
        
        LUCENE_CLASS(SpanMultiTermQueryWrapper);
    
    protected:
        MultiTermQueryPtr query;

    public:
        /// Returns the rewriteMethod
        SpanRewriteMethodPtr getRewriteMethod();
        
        /// Sets the rewrite method. This only makes sense to be a span rewrite method.
        void setRewriteMethod(SpanRewriteMethodPtr rewriteMethod);
        
        virtual SpansPtr getSpans(IndexReaderPtr reader);
        virtual String getField();
        
        using SpanQuery::toString;
        virtual String toString(const String& field);
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
        
        /// A rewrite method that first translates each term into a SpanTermQuery in a {@link 
        /// Occur#SHOULD} clause in a BooleanQuery, and keeps the scores as computed by the query.
        /// @see #setRewriteMethod
        static SpanRewriteMethodPtr SCORING_SPAN_QUERY_REWRITE();
    };

    /// Abstract class that defines how the query is rewritten.
    class LPPAPI SpanRewriteMethod : public RewriteMethod
    {
    public:
        SpanRewriteMethod();
        virtual ~SpanRewriteMethod();
        
        LUCENE_CLASS(SpanRewriteMethod);
    
    private:
        ScoringRewritePtr delegate;
    
    public:
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
    };
    
    class ScoringRewriteSpanOrQuery : public ScoringRewrite
    {
    public:
        virtual ~ScoringRewriteSpanOrQuery();
        LUCENE_CLASS(ScoringRewriteSpanOrQuery);
        
    protected:
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
    
    /// A rewrite method that first translates each term into a SpanTermQuery in a {@link 
    /// Occur#SHOULD} clause in a BooleanQuery, and keeps the scores as computed by the query.
    ///
    /// This rewrite method only uses the top scoring terms so it will not overflow the 
    /// boolean max clause count.
    ///
    /// @see #setRewriteMethod
    class LPPAPI TopTermsSpanBooleanQueryRewrite : public SpanRewriteMethod
    {
    public:
        TopTermsSpanBooleanQueryRewrite(int32_t size);
        virtual ~TopTermsSpanBooleanQueryRewrite();
        
        LUCENE_CLASS(TopTermsSpanBooleanQueryRewrite);

    public:
        TopTermsRewritePtr delegate;
    
    public:
        /// return the maximum priority queue size
        int32_t getSize();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
        
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);        
    };
    
    class TopTermsRewriteSpanOrQuery : public TopTermsRewrite
    {
    public:
        TopTermsRewriteSpanOrQuery(int32_t size);
        virtual ~TopTermsRewriteSpanOrQuery();
        
        LUCENE_CLASS(TopTermsRewriteSpanOrQuery);
        
    protected:
        virtual int32_t getMaxSize();
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
}

#endif
