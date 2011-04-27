/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "MultiCollector.h"

namespace Lucene
{
    MultiCollector::MultiCollector(Collection<CollectorPtr> collectors)
    {
        this->collectors = collectors;
    }
    
    MultiCollector::~MultiCollector()
    {
    }
    
    CollectorPtr MultiCollector::wrap(Collection<CollectorPtr> collectors)
    {
        // For the user's convenience, we allow null collectors to be passed. However, to improve performance, 
        // these null collectors are found and dropped from the array we save for actual collection time.
        int32_t n = 0;
        for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
        {
            if (*c)
                ++n;
        }
        if (n == 0)
            boost::throw_exception(IllegalArgumentException(L"At least 1 collector must not be null"));
        else if (n == 1)
        {
            // only 1 Collector - return it.
            CollectorPtr col;
            for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
            {
                if (*c)
                {
                    col = *c;
                    break;
                }
            }
            return col;
        }
        else if (n == collectors.size())
            return newLucene<MultiCollector>(collectors);
        else
        {
            Collection<CollectorPtr> colls(Collection<CollectorPtr>::newInstance(n));
            n = 0;
            for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
            {
                if (*c)
                    colls[n++] = *c;
            }
            return newLucene<MultiCollector>(colls);
        }
    }
    
    bool MultiCollector::acceptsDocsOutOfOrder()
    {
        for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
        {
            if (!(*c)->acceptsDocsOutOfOrder())
                return false;
        }
        return true;
    }
    
    void MultiCollector::collect(int32_t doc)
    {
        for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
            (*c)->collect(doc);
    }
    
    void MultiCollector::setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
        for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
            (*c)->setNextReader(reader, docBase);
    }
    
    void MultiCollector::setScorer(ScorerPtr scorer)
    {
        for (Collection<CollectorPtr>::iterator c = collectors.begin(); c != collectors.end(); ++c)
            (*c)->setScorer(scorer);
    }
}
