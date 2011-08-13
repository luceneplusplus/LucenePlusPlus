/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "HitQueueBase.h"
#include "ScoreDoc.h"

namespace Lucene
{
    HitQueueBase::HitQueueBase(int32_t size)
    {
        queueSize = size;
    }
    
    HitQueueBase::~HitQueueBase()
    {
    }
    
    void HitQueueBase::initialize()
    {
        queue = newLucene<PriorityQueueScoreDocs>(shared_from_this(), queueSize);
    }
    
    const ScoreDocPtr& HitQueueBase::add(const ScoreDocPtr& scoreDoc)
    {
        return queue->add(scoreDoc);
    }
    
    bool HitQueueBase::addOverflow(const ScoreDocPtr& scoreDoc)
    {
        return queue->addOverflow(scoreDoc);
    }
    
    const ScoreDocPtr& HitQueueBase::top()
    {
        return queue->top();
    }
    
    ScoreDocPtr HitQueueBase::pop()
    {
        return queue->pop();
    }
    
    const ScoreDocPtr& HitQueueBase::updateTop()
    {
        return queue->updateTop();
    }
    
    int32_t HitQueueBase::size()
    {
        return queue->size();
    }
    
    bool HitQueueBase::empty()
    {
        return queue->empty();
    }
    
    void HitQueueBase::clear()
    {
        queue->clear();
    }
    
    ScoreDocPtr HitQueueBase::getSentinelObject()
    {
        return ScoreDocPtr();
    }
    
    PriorityQueueScoreDocs::PriorityQueueScoreDocs(HitQueueBasePtr hitQueue, int32_t size) : PriorityQueue<ScoreDocPtr>(size)
    {
        _hitQueue = hitQueue.get();
    }
    
    PriorityQueueScoreDocs::~PriorityQueueScoreDocs()
    {
    }
    
    bool PriorityQueueScoreDocs::lessThan(const ScoreDocPtr& first, const ScoreDocPtr& second)
    {
        return _hitQueue->lessThan(first, second);
    }
    
    ScoreDocPtr PriorityQueueScoreDocs::getSentinelObject()
    {
        return _hitQueue->getSentinelObject();
    }
}
