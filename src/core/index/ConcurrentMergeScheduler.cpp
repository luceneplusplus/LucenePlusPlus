/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ConcurrentMergeScheduler.h"
#include "_ConcurrentMergeScheduler.h"
#include "IndexWriter.h"
#include "LuceneThread.h"
#include "TestPoint.h"
#include "StringUtils.h"
#include "MiscUtils.h"

namespace Lucene
{
    Collection<ConcurrentMergeSchedulerPtr> ConcurrentMergeScheduler::allInstances;
    bool ConcurrentMergeScheduler::anyExceptions = false;
    
    ConcurrentMergeScheduler::ConcurrentMergeScheduler()
    {
        // Max number of merge threads allowed to be running at once.  When there are 
        // more merges then this, we forcefully pause the larger ones, letting the smaller
        // ones run, up until maxMergeCount merges at which point we forcefully pause 
        // incoming threads (that presumably are the ones causing so much merging).  We 
        // dynamically default this from 1 to 3, depending on how many cores you have
        maxThreadCount = std::max(1, std::min(3, LuceneThread::availableProcessors() / 2));
        
        // Max number of merges we accept before forcefully throttling the incoming threads
        maxMergeCount = maxThreadCount + 2;
        
        mergeThreadPriority = -1;
        mergeThreads = Collection<MergeThreadPtr>::newInstance();
        maxThreadCount = 1;
        suppressExceptions = false;
        closed = false;
    }
    
    ConcurrentMergeScheduler::~ConcurrentMergeScheduler()
    {
    }
    
    void ConcurrentMergeScheduler::initialize()
    {
        // Only for testing
        if (allInstances)
            addMyself();
    }
    
    void ConcurrentMergeScheduler::setMaxThreadCount(int32_t count)
    {
        if (count < 1)
            boost::throw_exception(IllegalArgumentException(L"count should be at least 1"));
        if (count > maxMergeCount)
            boost::throw_exception(IllegalArgumentException(L"count should be <= maxMergeCount (= " + StringUtils::toString(maxMergeCount) + L")"));
        maxThreadCount = count;
    }
    
    int32_t ConcurrentMergeScheduler::getMaxThreadCount()
    {
        return maxThreadCount;
    }
    
    void ConcurrentMergeScheduler::setMaxMergeCount(int32_t count)
    {
        if (count < 1)
            boost::throw_exception(IllegalArgumentException(L"count should be at least 1"));
        if (count < maxThreadCount)
            boost::throw_exception(IllegalArgumentException(L"count should be >= maxThreadCount (= " + StringUtils::toString(maxThreadCount) + L")"));
        maxMergeCount = count;
    }
    
    int32_t ConcurrentMergeScheduler::getMaxMergeCount()
    {
        return maxMergeCount;
    }
    
    int32_t ConcurrentMergeScheduler::getMergeThreadPriority()
    {
        SyncLock syncLock(this);
        initMergeThreadPriority();
        return mergeThreadPriority;
    }
    
    void ConcurrentMergeScheduler::setMergeThreadPriority(int32_t pri)
    {
        SyncLock syncLock(this);
        if (pri > LuceneThread::MAX_PRIORITY || pri < LuceneThread::MIN_PRIORITY)
        {
            boost::throw_exception(IllegalArgumentException(L"priority must be in range " + StringUtils::toString(LuceneThread::MIN_PRIORITY) + 
                                                            L" .. " + StringUtils::toString(LuceneThread::MAX_PRIORITY) + L" inclusive"));
        }
        mergeThreadPriority = pri;
        updateMergeThreads();
    }
    
    struct lessMergeDocCount
    {
        inline bool operator()(const MergeThreadPtr& first, const MergeThreadPtr& second) const
        {
            OneMergePtr m1(first->getCurrentMerge());
            OneMergePtr m2(second->getCurrentMerge());

            int32_t c1 = m1 ? m1->segments->totalDocCount() : INT_MAX;
            int32_t c2 = m2 ? m2->segments->totalDocCount() : INT_MAX;
            
            return c1 < c2;
        }
    };
    
    void ConcurrentMergeScheduler::updateMergeThreads()
    {
        // Only look at threads that are alive and not in the process of 
        // stopping (ie. have an active merge)
        Collection<MergeThreadPtr> activeMerges(Collection<MergeThreadPtr>::newInstance());

        int32_t threadIdx = 0;
        while (threadIdx < mergeThreads.size())
        {
            MergeThreadPtr mergeThread(mergeThreads[threadIdx]);
            if (!mergeThread->isAlive())
            {
                // Prune any dead threads
                mergeThreads.remove(mergeThreads.begin() + threadIdx);
                continue;
            }
            if (mergeThread->getCurrentMerge())
                activeMerges.add(mergeThread);
            ++threadIdx;
        }

        // Sort the merge threads in descending order.
        std::sort(activeMerges.begin(), activeMerges.end(), lessMergeDocCount());

        int32_t pri = mergeThreadPriority;
        int32_t activeMergeCount = activeMerges.size();
        for (threadIdx = 0; threadIdx < activeMergeCount; ++threadIdx)
        {
            MergeThreadPtr mergeThread(activeMerges[threadIdx]);
            OneMergePtr merge(mergeThread->getCurrentMerge());
            if (!merge)
                continue;

            // pause the thread if maxThreadCount is smaller than the number of merge threads.
            bool doPause = (threadIdx < activeMergeCount - maxThreadCount);

            if (verbose())
            {
                if (doPause != merge->getPause())
                {
                    if (doPause)
                        message(L"pause thread " + StringUtils::toString(mergeThread->threadId()));
                    else
                        message(L"unpause thread " + StringUtils::toString(mergeThread->threadId()));
                }
            }
            if (doPause != merge->getPause())
                merge->setPause(doPause);

            if (!doPause)
            {
                if (verbose())
                {
                    message(L"set priority of merge thread " + StringUtils::toString(mergeThread->threadId()) + 
                            L" to " + StringUtils::toString(pri));
                }
                mergeThread->setThreadPriority(pri);
                pri = std::min(LuceneThread::MAX_PRIORITY, 1 + pri);
            }
        }
    }
    
    bool ConcurrentMergeScheduler::verbose()
    {
        return (!_writer.expired() && IndexWriterPtr(_writer)->verbose());
    }
    
    void ConcurrentMergeScheduler::message(const String& message)
    {
        IndexWriterPtr(_writer)->message(L"CMS: " + message);
    }
    
    void ConcurrentMergeScheduler::initMergeThreadPriority()
    {
        SyncLock syncLock(this);
        if (mergeThreadPriority == -1)
        {
            // Default to slightly higher priority than our calling thread
            mergeThreadPriority = std::min(LuceneThread::NORM_PRIORITY + 1, LuceneThread::MAX_PRIORITY);
        }
    }
    
    void ConcurrentMergeScheduler::close()
    {
        closed = true;
        sync();
    }
    
    void ConcurrentMergeScheduler::sync()
    {
        while (true)
        {
            MergeThreadPtr toSync;
            {
                SyncLock syncLock(this);
                for (Collection<MergeThreadPtr>::iterator t = mergeThreads.begin(); t != mergeThreads.end(); ++t)
                {
                    if ((*t)->isAlive())
                    {
                        toSync = *t;
                        break;
                    }
                }
            }
            if (toSync)
                toSync->join();
            else
                break;
        }
    }
    
    int32_t ConcurrentMergeScheduler::mergeThreadCount()
    {
        SyncLock syncLock(this);
        int32_t count = 0;
        for (Collection<MergeThreadPtr>::iterator mt = mergeThreads.begin(); mt != mergeThreads.end(); ++mt)
        {
            if ((*mt)->isAlive() && (*mt)->getCurrentMerge())
                ++count;
        }
        return count;
    }
    
    void ConcurrentMergeScheduler::merge(IndexWriterPtr writer)
    {
        BOOST_ASSERT(!writer->holdsLock());
        
        this->_writer = writer;
        
        initMergeThreadPriority();
        
        dir = writer->getDirectory();
        
        // First, quickly run through the newly proposed merges and add any orthogonal merges (ie a merge not
        // involving segments already pending to be merged) to the queue.  If we are way behind on merging, 
        // many of these newly proposed merges will likely already be registered.
        if (verbose())
        {
            message(L"now merge");
            message(L"  index: " + writer->segString());
        }
        
        // Iterate, pulling from the IndexWriter's queue of pending merges, until it's empty
        while (true)
        {
            {
                SyncLock syncLock(this);
                int64_t startStallTime = 0;
                while (mergeThreadCount() >= 1 + maxMergeCount)
                {
                    startStallTime = MiscUtils::currentTimeMillis();
                    if (verbose())
                        message(L"    too many merges; stalling...");
                    wait();
                }

                if (verbose())
                {
                    if (startStallTime != 0)
                        message(L"  stalled for " + StringUtils::toString(MiscUtils::currentTimeMillis() - startStallTime) + L" msec");
                }
            }
            
            OneMergePtr merge(writer->getNextMerge());
            if (!merge)
            {
                if (verbose())
                    message(L"  no more merges pending; now return");
                return;
            }
            
            // We do this with the primary thread to keep deterministic assignment of segment names
            writer->mergeInit(merge);
            
            bool success = false;
            LuceneException finally;
            try
            {
                SyncLock syncLock(this);
                
                if (verbose())
                    message(L"  consider merge " + merge->segString(dir));
                
                // OK to spawn a new merge thread to handle this merge
                MergeThreadPtr merger(getMergeThread(writer, merge));
                mergeThreads.add(merger);
                if (verbose())
                    message(L"    launch new thread [" + StringUtils::toString(merger->threadId()) + L"]");

                merger->start();

                // Must call this after starting the thread else the new thread is removed from 
                // mergeThreads (since it's not alive yet)
                updateMergeThreads();

                success = true;
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            if (!success)
                writer->mergeFinish(merge);
            finally.throwException();
        }
    }
    
    void ConcurrentMergeScheduler::doMerge(OneMergePtr merge)
    {
        TestScope testScope(L"ConcurrentMergeScheduler", L"doMerge");
        IndexWriterPtr(_writer)->merge(merge);
    }
    
    MergeThreadPtr ConcurrentMergeScheduler::getMergeThread(IndexWriterPtr writer, OneMergePtr merge)
    {
        SyncLock syncLock(this);
        MergeThreadPtr thread(newLucene<MergeThread>(shared_from_this(), writer, merge));
        thread->setThreadPriority(mergeThreadPriority);
        return thread;
    }
    
    void ConcurrentMergeScheduler::handleMergeException(const LuceneException& exc)
    {
        // When an exception is hit during merge, IndexWriter removes any partial files and then 
        // allows another merge to run.  If whatever caused the error is not transient then the 
        // exception will keep happening, so, we sleep here to avoid saturating CPU in such cases
        LuceneThread::threadSleep(250); // pause 250 msec
        boost::throw_exception(MergeException());
    }
    
    bool ConcurrentMergeScheduler::anyUnhandledExceptions()
    {
        if (!allInstances)
            boost::throw_exception(RuntimeException(L"setTestMode() was not called"));
        SyncLock instancesLock(&allInstances);
        for (Collection<ConcurrentMergeSchedulerPtr>::iterator instance = allInstances.begin(); instance != allInstances.end(); ++instance)
            (*instance)->sync();
        bool v = anyExceptions;
        anyExceptions = false;
        return v;
    }
    
    void ConcurrentMergeScheduler::clearUnhandledExceptions()
    {
        SyncLock instancesLock(&allInstances);
        anyExceptions = false;
    }
    
    void ConcurrentMergeScheduler::addMyself()
    {
        SyncLock instancesLock(&allInstances);
        int32_t size = allInstances.size();
        int32_t upto = 0;
        for (int32_t i = 0; i < size; ++i)
        {
            ConcurrentMergeSchedulerPtr other(allInstances[i]);
            if (!(other->closed && other->mergeThreadCount() == 0))
            {
                // Keep this one for now: it still has threads or may spawn new threads
                allInstances[upto++] = other;
            }
            
            allInstances.remove(allInstances.begin() + upto, allInstances.end());
            allInstances.add(shared_from_this());
        }
    }
    
    void ConcurrentMergeScheduler::setSuppressExceptions()
    {
        suppressExceptions = true;
    }
    
    void ConcurrentMergeScheduler::clearSuppressExceptions()
    {
        suppressExceptions = false;
    }
    
    void ConcurrentMergeScheduler::setTestMode()
    {
        allInstances = Collection<ConcurrentMergeSchedulerPtr>::newInstance();
    }
    
    MergeThread::MergeThread(ConcurrentMergeSchedulerPtr merger, IndexWriterPtr writer, OneMergePtr startMerge)
    {
        this->_merger = merger;
        this->_tWriter = writer;
        this->startMerge = startMerge;
    }
    
    MergeThread::~MergeThread()
    {
    }
    
    void MergeThread::setRunningMerge(OneMergePtr merge)
    {
        SyncLock syncLock(this);
        runningMerge = merge;
    }
    
    OneMergePtr MergeThread::getRunningMerge()
    {
        SyncLock syncLock(this);
        return runningMerge;
    }
    
    OneMergePtr MergeThread::getCurrentMerge()
    {
        SyncLock syncLock(this);
        if (done)
            return OneMergePtr();
        else if (runningMerge)
            return runningMerge;
        else
            return startMerge;
    }
    
    void MergeThread::setThreadPriority(int32_t pri)
    {
        try
        {
            setPriority(pri);
        }
        catch (...)
        {
        }
    }
    
    void MergeThread::run()
    {
        // First time through the while loop we do the merge that we were started with
        OneMergePtr merge(this->startMerge);
        ConcurrentMergeSchedulerPtr merger(_merger);
        
        LuceneException finally;
        try
        {
            if (merger->verbose())
                merger->message(L"  merge thread: start");
            IndexWriterPtr tWriter(_tWriter);
            
            while (true)
            {
                setRunningMerge(merge);
                merger->doMerge(merge);
                
                // Subsequent times through the loop we do any new merge that writer says is necessary
                merge = tWriter->getNextMerge();
                if (merge)
                {
                    tWriter->mergeInit(merge);
                    merger->updateMergeThreads();
                    if (merger->verbose())
                        merger->message(L"  merge thread: do another merge " + merge->segString(merger->dir));
                }
                else
                    break;
            }
            
            if (merger->verbose())
                merger->message(L"  merge thread: done");
        }
        catch (MergeAbortedException&)
        {
            // Ignore the exception if it was due to abort
        }
        catch (LuceneException& e)
        {
            if (!merger->suppressExceptions)
            {
                // suppressExceptions is normally only set during testing.
                merger->anyExceptions = true;
                merger->handleMergeException(e);
            }
            else
                finally = e;
        }
        
        {
            done = true;
            SyncLock syncLock(merger);
            merger->updateMergeThreads();
            merger->notifyAll();
        }
        finally.throwException();
    }
}
