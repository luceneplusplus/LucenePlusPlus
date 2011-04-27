/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SCORINGREWRITE_H
#define SCORINGREWRITE_H

#include "TermCollectingRewrite.h"

namespace Lucene
{
    /// Only public to be accessible by spans package.
    class LPPAPI ScoringRewrite : public TermCollectingRewrite
    {
    public:
        virtual ~ScoringRewrite();
        LUCENE_CLASS(ScoringRewrite);

    public:
        /// A rewrite method that first translates each term into {@link BooleanClause#SHOULD} clause in a
        /// BooleanQuery, and keeps the scores as computed by the query.  Note that typically such scores 
        /// are meaningless to the user, and require non-trivial CPU to compute, so it's almost always 
        /// better to use {@link MultiTermQuery#CONSTANT_SCORE_AUTO_REWRITE_DEFAULT} instead.
        ///
        /// NOTE: This rewrite method will hit {@link BooleanQuery.TooManyClauses} if the number of terms
        /// exceeds {@link BooleanQuery#getMaxClauseCount}.
        /// @see #setRewriteMethod
        static ScoringRewritePtr SCORING_BOOLEAN_QUERY_REWRITE();
        
        /// Like {@link #SCORING_BOOLEAN_QUERY_REWRITE} except scores are not computed.  Instead, each 
        /// matching document receives a constant score equal to the query's boost.
        ///
        /// NOTE: This rewrite method will hit {@link BooleanQuery.TooManyClauses} if the number of terms
        /// exceeds {@link BooleanQuery#getMaxClauseCount}.
        /// @see #setRewriteMethod
        static RewriteMethodPtr CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
    };
}

#endif
