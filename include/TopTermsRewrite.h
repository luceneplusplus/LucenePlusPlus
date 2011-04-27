/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOPTERMSREWRITE_H
#define TOPTERMSREWRITE_H

#include "TermCollectingRewrite.h"

namespace Lucene
{
    /// Base rewrite method for collecting only the top terms via a priority queue.
    class LPPAPI TopTermsRewrite : public TermCollectingRewrite
    {
    public:
        /// Create a TopTermsBooleanQueryRewrite for at most size terms.
        ///
        /// NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than size, then 
        /// it will be used instead. 
        TopTermsRewrite(int32_t size);
        
        virtual ~TopTermsRewrite();
    
        LUCENE_CLASS(TopTermsRewrite);
    
    private:
        int32_t size;
    
    public:
        /// Return the maximum priority queue size
        virtual int32_t getSize();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
    
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
        
    protected:
        /// Return the maximum size of the priority queue (for boolean rewrites this 
        /// is BooleanQuery#getMaxClauseCount).
        virtual int32_t getMaxSize() = 0;
    };
    
    /// A rewrite method that first translates each term into {@link BooleanClause#SHOULD} 
    /// clause in a BooleanQuery, and keeps the scores as computed by the query.
    ///
    /// This rewrite method only uses the top scoring terms so it will not overflow the 
    /// boolean max clause count. It is the default rewrite method for {@link FuzzyQuery}.
    /// @see #setRewriteMethod
    class LPPAPI TopTermsScoringBooleanQueryRewrite : public TopTermsRewrite
    {
    public:
        /// Create a TopTermsScoringBooleanQueryRewrite for at most size terms.
        ///
        /// NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than size, then it 
        /// will be used instead. 
        TopTermsScoringBooleanQueryRewrite(int32_t size);
        virtual ~TopTermsScoringBooleanQueryRewrite();
        
        LUCENE_CLASS(TopTermsScoringBooleanQueryRewrite);
    
    protected:
        virtual int32_t getMaxSize();
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
    
    /// A rewrite method that first translates each term into {@link BooleanClause#SHOULD} 
    /// clause in a BooleanQuery, but the scores are only computed as the boost.
    ///
    /// This rewrite method only uses the top scoring terms so it will not overflow the 
    /// boolean max clause count.
    ///
    /// @see #setRewriteMethod
    class LPPAPI TopTermsBoostOnlyBooleanQueryRewrite : public TopTermsRewrite
    {
    public:
        /// Create a TopTermsBoostOnlyBooleanQueryRewrite for at most size terms.
        ///
        /// NOTE: if {@link BooleanQuery#getMaxClauseCount} is smaller than size, then it 
        /// will be used instead. 
        TopTermsBoostOnlyBooleanQueryRewrite(int32_t size);
        virtual ~TopTermsBoostOnlyBooleanQueryRewrite();
        
        LUCENE_CLASS(TopTermsBoostOnlyBooleanQueryRewrite);
    
    protected:
        virtual int32_t getMaxSize();
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
}

#endif
