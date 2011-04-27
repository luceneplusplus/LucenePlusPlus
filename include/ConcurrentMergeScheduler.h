/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CONCURRENTMERGESCHEDULER_H
#define CONCURRENTMERGESCHEDULER_H

#include "MergeScheduler.h"

namespace Lucene
{
    /// A {@link MergeScheduler} that runs each merge using a separate thread.
    ///
    /// Specify the max number of threads that may run at once with {@link #setMaxThreadCount}.
    ///
    /// Separately specify the maximum number of simultaneous merges with {@link #setMaxMergeCount}.
    /// If the number of merges exceeds the max number of threads then the largest merges are paused 
    /// until one of the smaller merges completes.
    ///
    /// If more than {@link #getMaxMergeCount} merges are requested then this class will forcefully 
    /// throttle the incoming threads by pausing until one more more merges complete.
    class LPPAPI ConcurrentMergeScheduler : public MergeScheduler
    {
    public:
        ConcurrentMergeScheduler();
        virtual ~ConcurrentMergeScheduler();
        
        LUCENE_CLASS(ConcurrentMergeScheduler);
        
    protected:
        int32_t mergeThreadPriority;
        
        Collection<MergeThreadPtr> mergeThreads;
        
        /// Max number of threads allowed to be merging at once
        int32_t maxThreadCount;
        
        // Max number of merges we accept before forcefully throttling the incoming threads
        int32_t maxMergeCount;
        
        DirectoryPtr dir;
        
        bool closed;
        IndexWriterWeakPtr _writer;
                
        static Collection<ConcurrentMergeSchedulerPtr> allInstances;
        
        bool suppressExceptions;
        static bool anyExceptions;
    
    public:
        virtual void initialize();
        
        /// Sets the max # simultaneous merge threads that should be running at once.  This must 
        /// be <= {@link #setMaxMergeCount}.
        virtual void setMaxThreadCount(int32_t count);
        
        /// @see #setMaxThreadCount(int32_t)
        virtual int32_t getMaxThreadCount();
        
        /// Sets the max # simultaneous merges that are allowed. If a merge is necessary yet we 
        /// already have this many threads running, the incoming thread (that is calling 
        /// add/updateDocument) will block until a merge thread has completed.  Note that we will 
        /// only run the smallest {@link #setMaxThreadCount} merges at a time.
        virtual void setMaxMergeCount(int32_t count);
        
        /// See {@link #setMaxMergeCount}.
        virtual int32_t getMaxMergeCount();
        
        /// Return the priority that merge threads run at.  By default the priority is 1 plus the 
        /// priority of (ie, slightly higher priority than) the first thread that calls merge.
        virtual int32_t getMergeThreadPriority();
        
        /// Set the base priority that merge threads run at. Note that CMS may increase priority 
        /// of some merge threads beyond this base priority.  It's best not to set this any higher 
        /// than Thread.MAX_PRIORITY - maxThreadCount, so that CMS has room to set relative 
        /// priority among threads.
        virtual void setMergeThreadPriority(int32_t pri);
        
        virtual void close();
        
        /// Wait for any running merge threads to finish.
        virtual void sync();
        
        virtual void merge(IndexWriterPtr writer);
        
        /// Used for testing
        static bool anyUnhandledExceptions();
        static void clearUnhandledExceptions();
        
        /// Used for testing
        void setSuppressExceptions();
        void clearSuppressExceptions();
        
        /// Used for testing
        /// @Deprecated
        static void setTestMode();
    
    protected:
        /// Called whenever the running merges have changed, to pause and unpause threads. This 
        /// method sorts the merge threads by their merge size in descending order and then 
        /// pauses/unpauses threads from first to last - that way, smaller merges are guaranteed 
        /// to run before larger ones.
        virtual void updateMergeThreads();
        
        /// Returns true if verbosing is enabled. This method is usually used in conjunction with 
        /// {@link #message(String)}, like that
        ///
        /// <pre>
        /// if (verbose())
        ///     message(L"your message");
        /// </pre>
        virtual bool verbose();
        
        /// Outputs the given message - this method assumes {@link #verbose()} was called and 
        /// returned true.
        virtual void message(const String& message);
        
        virtual void initMergeThreadPriority();
        
        /// Returns the number of merge threads that are alive. Note that this number is <=
        /// {@link #mergeThreads} size.
        virtual int32_t mergeThreadCount();
        
        /// Does the actual merge, by calling {@link IndexWriter#merge}
        virtual void doMerge(OneMergePtr merge);
        
        virtual MergeThreadPtr getMergeThread(IndexWriterPtr writer, OneMergePtr merge);
        
        /// Called when an exception is hit in a background merge thread
        virtual void handleMergeException(const LuceneException& exc);
        
        virtual void addMyself();
        
        friend class MergeThread;
    };
}

#endif
