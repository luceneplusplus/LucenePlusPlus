/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NoMergeScheduler.h"

namespace Lucene
{
    NoMergeScheduler::~NoMergeScheduler()
    {
    }
    
    MergeSchedulerPtr NoMergeScheduler::INSTANCE()
    {
        static MergeSchedulerPtr _INSTANCE;
        if (!_INSTANCE)
        {
            _INSTANCE = newLucene<NoMergeScheduler>();
            CycleCheck::addStatic(_INSTANCE);
        }
        return _INSTANCE;
    }
    
    void NoMergeScheduler::merge(IndexWriterPtr writer)
    {
    }
    
    void NoMergeScheduler::close()
    {
    }
}
