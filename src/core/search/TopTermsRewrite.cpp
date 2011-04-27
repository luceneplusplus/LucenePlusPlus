/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TopTermsRewrite.h"
#include "_TopTermsRewrite.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "ConstantScoreQuery.h"
#include "Term.h"

namespace Lucene
{
    TopTermsRewrite::TopTermsRewrite(int32_t size)
    {
        this->size = size;
    }
    
    TopTermsRewrite::~TopTermsRewrite()
    {
    }
    
    int32_t TopTermsRewrite::getSize()
    {
        return size;
    }
    
    QueryPtr TopTermsRewrite::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        int32_t maxSize = std::min(size, getMaxSize());
        ScoreTermQueuePtr stQueue(newLucene<ScoreTermQueue>(maxSize + 1));
        collectTerms(reader, query, newLucene<TopTermsRewriteTermCollector>(stQueue, maxSize));
        QueryPtr q(getTopLevelQuery());
        int32_t size = stQueue->size();
        ScoreTermPtr st(stQueue->pop());
        while (st)
        {
            addClause(q, st->term, query->getBoost() * st->boost); // add to query
            st = stQueue->pop();
        }
        query->incTotalNumberOfTerms(size);
        return q;
    }
    
    int32_t TopTermsRewrite::hashCode()
    {
        return 31 * size;
    }
    
    bool TopTermsRewrite::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        if (!other)
            return false;
        if (!MiscUtils::equalTypes(shared_from_this(), other))
            return false;
        
        TopTermsRewritePtr otherTopTermsRewrite(boost::dynamic_pointer_cast<TopTermsRewrite>(other));
        if (!otherTopTermsRewrite)
            return false;
        
        if (size != otherTopTermsRewrite->size)
            return false;
        
        return true;
    }
    
    TopTermsScoringBooleanQueryRewrite::TopTermsScoringBooleanQueryRewrite(int32_t size) : TopTermsRewrite(size)
    {
    }
    
    TopTermsScoringBooleanQueryRewrite::~TopTermsScoringBooleanQueryRewrite()
    {
    }
    
    int32_t TopTermsScoringBooleanQueryRewrite::getMaxSize()
    {
        return BooleanQuery::getMaxClauseCount();
    }
    
    QueryPtr TopTermsScoringBooleanQueryRewrite::getTopLevelQuery()
    {
        return newLucene<BooleanQuery>(true);
    }
    
    void TopTermsScoringBooleanQueryRewrite::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        TermQueryPtr tq(newLucene<TermQuery>(term));
        tq->setBoost(boost);
        boost::static_pointer_cast<BooleanQuery>(topLevel)->add(tq, BooleanClause::SHOULD);
    }
    
    TopTermsBoostOnlyBooleanQueryRewrite::TopTermsBoostOnlyBooleanQueryRewrite(int32_t size) : TopTermsRewrite(size)
    {
    }
    
    TopTermsBoostOnlyBooleanQueryRewrite::~TopTermsBoostOnlyBooleanQueryRewrite()
    {
    }
    
    int32_t TopTermsBoostOnlyBooleanQueryRewrite::getMaxSize()
    {
        return BooleanQuery::getMaxClauseCount();
    }
    
    QueryPtr TopTermsBoostOnlyBooleanQueryRewrite::getTopLevelQuery()
    {
        return newLucene<BooleanQuery>(true);
    }
    
    void TopTermsBoostOnlyBooleanQueryRewrite::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        QueryPtr tq(newLucene<ConstantScoreQuery>(newLucene<TermQuery>(term)));
        tq->setBoost(boost);
        boost::static_pointer_cast<BooleanQuery>(topLevel)->add(tq, BooleanClause::SHOULD);
    }
    
    ScoreTerm::ScoreTerm()
    {
        boost = 0;
    }
    
    ScoreTerm::~ScoreTerm()
    {
    }
    
    int32_t ScoreTerm::compareTo(ScoreTermPtr other)
    {
        if (this->boost == other->boost)
            return other->term->compareTo(this->term);
        else
            return this->boost < other->boost ? -1 : (this->boost > other->boost ? 1 : 0);
    }
    
    ScoreTermQueue::ScoreTermQueue(int32_t size) : PriorityQueue<ScoreTermPtr>(size)
    {
    }
    
    ScoreTermQueue::~ScoreTermQueue()
    {
    }
    
    bool ScoreTermQueue::lessThan(const ScoreTermPtr& first, const ScoreTermPtr& second)
    {
        return (first->compareTo(second) < 0);
    }
    
    TopTermsRewriteTermCollector::TopTermsRewriteTermCollector(ScoreTermQueuePtr stQueue, int32_t maxSize)
    {
        this->stQueue = stQueue;
        this->maxSize = maxSize;
        this->st = newLucene<ScoreTerm>();
    }
    
    TopTermsRewriteTermCollector::~TopTermsRewriteTermCollector()
    {
    }
    
    bool TopTermsRewriteTermCollector::collect(TermPtr t, double boost)
    {
        // ignore uncompetitive hits
        if (stQueue->size() >= maxSize && boost <= stQueue->top()->boost)
            return true;
        // add new entry in PQ
        st->term = t;
        st->boost = boost;
        stQueue->add(st);
        // possibly drop entries from queue
        st = (stQueue->size() > maxSize) ? stQueue->pop() : newLucene<ScoreTerm>();
        return true;
    }
}
