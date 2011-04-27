/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCORINGREWRITE_H
#define _SCORINGREWRITE_H

#include "ScoringRewrite.h"
#include "TermCollectingRewrite.h"

namespace Lucene
{
    /// A rewrite method that first translates each term into {@link BooleanClause#SHOULD} clause in a
    /// BooleanQuery, and keeps the scores as computed by the query.  Note that typically such scores 
    /// are meaningless to the user, and require non-trivial CPU to compute, so it's almost always 
    /// better to use {@link MultiTermQuery#CONSTANT_SCORE_AUTO_REWRITE_DEFAULT} instead.
    ///
    /// NOTE: This rewrite method will hit {@link BooleanQuery.TooManyClauses} if the number of terms
    /// exceeds {@link BooleanQuery#getMaxClauseCount}.
    /// @see #setRewriteMethod
    class ScoringRewriteBooleanQuery : public ScoringRewrite
    {
    public:
        virtual ~ScoringRewriteBooleanQuery();
        LUCENE_CLASS(ScoringRewriteBooleanQuery);

    protected:
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
    
    /// Like {@link #SCORING_BOOLEAN_QUERY_REWRITE} except scores are not computed.  Instead, each 
    /// matching document receives a constant score equal to the query's boost.
    ///
    /// NOTE: This rewrite method will hit {@link BooleanQuery.TooManyClauses} if the number of terms
    /// exceeds {@link BooleanQuery#getMaxClauseCount}.
    /// @see #setRewriteMethod
    class ConstantScoreBooleanQueryRewrite : public RewriteMethod
    {
    public:
        virtual ~ConstantScoreBooleanQueryRewrite();
        LUCENE_CLASS(ConstantScoreBooleanQueryRewrite);

    public:
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
    };
    
    class ScoringRewriteTermCollector : public TermCollector
    {
    public:
        ScoringRewriteTermCollector(ScoringRewritePtr query, MultiTermQueryPtr multiQuery, QueryPtr result, IntArray size);
        virtual ~ScoringRewriteTermCollector();
        
        LUCENE_CLASS(ScoringRewriteTermCollector);
    
    protected:
        ScoringRewriteWeakPtr _query;
        MultiTermQueryPtr multiQuery;
        QueryPtr result;
        IntArray size;
    
    protected:
        virtual bool collect(TermPtr t, double boost);
    };
}

#endif
