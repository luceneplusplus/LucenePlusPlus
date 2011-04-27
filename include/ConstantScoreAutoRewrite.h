/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CONSTANTSCOREAUTOREWRITE_H
#define CONSTANTSCOREAUTOREWRITE_H

#include "TermCollectingRewrite.h"

namespace Lucene
{
    class LPPAPI ConstantScoreAutoRewrite : public TermCollectingRewrite
    {
    public:
        ConstantScoreAutoRewrite();
        virtual ~ConstantScoreAutoRewrite();
        
        LUCENE_CLASS(ConstantScoreAutoRewrite);
    
    public:
        // Defaults derived from rough tests with a 20.0 million doc Wikipedia index.  With
        // more than 350 terms in the query, the filter method is fastest
        static const int32_t DEFAULT_TERM_COUNT_CUTOFF;
        
        // If the query will hit more than 1 in 1000 of the docs in the index (0.1%), the 
        // filter method is fastest
        static const double DEFAULT_DOC_COUNT_PERCENT;
    
    protected:
        int32_t termCountCutoff;
        double docCountPercent;
    
    public:
        /// If the number of terms in this query is equal to or larger than this setting
        /// then {@link #CONSTANT_SCORE_FILTER_REWRITE} is used.
        virtual void setTermCountCutoff(int32_t count);
        
        /// @see #setTermCountCutoff
        virtual int32_t getTermCountCutoff();
        
        /// If the number of documents to be visited in the postings exceeds this specified 
        /// percentage of the maxDoc() for the index, then {@link #CONSTANT_SCORE_FILTER_REWRITE} 
        /// is used.
        /// @param percent 0.0 to 100.0
        virtual void setDocCountPercent(double percent);
        
        /// @see #setDocCountPercent
        virtual double getDocCountPercent();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader, MultiTermQueryPtr query);
        
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
    
    protected:
        virtual QueryPtr getTopLevelQuery();
        virtual void addClause(QueryPtr topLevel, TermPtr term, double boost);
    };
    
    class ConstantScoreAutoRewriteDefault : public ConstantScoreAutoRewrite
    {
    public:
        virtual ~ConstantScoreAutoRewriteDefault();
        LUCENE_CLASS(ConstantScoreAutoRewriteDefault);
    
    public:
        virtual void setTermCountCutoff(int32_t count);
        virtual void setDocCountPercent(double percent);
    };
}

#endif
