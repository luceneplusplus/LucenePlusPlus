/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONCURRENTMERGESCHEDULER_H
#define _CONCURRENTMERGESCHEDULER_H

#include "LuceneThread.h"

namespace Lucene
{
    class MergeThread : public LuceneThread
    {
    public:
        MergeThread(ConcurrentMergeSchedulerPtr merger, IndexWriterPtr writer, OneMergePtr startMerge);
        virtual ~MergeThread();

        LUCENE_CLASS(MergeThread);

    protected:
        ConcurrentMergeSchedulerPtr merger;
        IndexWriterPtr writer;
        OneMergePtr startMerge;
        OneMergePtr runningMerge;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(merger);
            gc->mark(writer);
            gc->mark(startMerge);
            gc->mark(runningMerge);
            LuceneThread::mark_members(gc);
        }

    public:
        void setRunningMerge(OneMergePtr merge);
        OneMergePtr getRunningMerge();
        void setThreadPriority(int32_t pri);
        virtual void run();
    };
}

#endif
