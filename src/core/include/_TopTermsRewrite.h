/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TOPTERMSREWRITE_H
#define _TOPTERMSREWRITE_H

#include "PriorityQueue.h"

namespace Lucene
{
    class ScoreTerm : public LuceneObject
    {
    public:
        ScoreTerm();
        virtual ~ScoreTerm();
        
        LUCENE_CLASS(ScoreTerm);
    
    public:
        TermPtr term;
        double boost;
    
    public:
        int32_t compareTo(ScoreTermPtr other);
    };
    
    class ScoreTermQueue : public PriorityQueue<ScoreTermPtr>
    {
    public:
        ScoreTermQueue(int32_t size);
        virtual ~ScoreTermQueue();
        
        LUCENE_CLASS(ScoreTermQueue);
    
    protected:
        virtual bool lessThan(const ScoreTermPtr& first, const ScoreTermPtr& second);
    };
    
    class TopTermsRewriteTermCollector : public TermCollector
    {
    public:
        TopTermsRewriteTermCollector(ScoreTermQueuePtr stQueue, int32_t maxSize);
        virtual ~TopTermsRewriteTermCollector();
        
        LUCENE_CLASS(TopTermsRewriteTermCollector);
    
    protected:
        ScoreTermQueuePtr stQueue;
        int32_t maxSize;
        
        // reusable instance
        ScoreTermPtr st;
    
    protected:
        virtual bool collect(TermPtr t, double boost);
    };
}

#endif
