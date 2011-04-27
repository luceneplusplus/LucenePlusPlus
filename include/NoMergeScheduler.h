/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NOMERGESCHEDULER_H
#define NOMERGESCHEDULER_H

#include "MergeScheduler.h"

namespace Lucene
{
    /// A {@link MergeScheduler} which never executes any merges. It is also a singleton and 
    /// can be accessed through {@link NoMergeScheduler#INSTANCE}. Use it if you want to 
    /// prevent an {@link IndexWriter} from ever executing merges, irregardless of the {@link 
    /// MergePolicy} used. Note that you can achieve the same thing by using {@link 
    /// NoMergePolicy}, however with {@link NoMergeScheduler} you also ensure that no 
    /// unnecessary code of any {@link MergeScheduler} implementation is ever executed. Hence 
    /// it is recommended to use both if you want to disable merges from ever happening.
    class LPPAPI NoMergeScheduler : public MergeScheduler
    {
    public:
        virtual ~NoMergeScheduler();
        
        LUCENE_CLASS(NoMergeScheduler);
    
    public:
        /// The single instance of {@link NoMergeScheduler}
        static MergeSchedulerPtr INSTANCE();
    
        virtual void merge(IndexWriterPtr writer);
        virtual void close();
    };
}

#endif
