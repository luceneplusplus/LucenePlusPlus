/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneThread.h"

namespace Lucene
{
    #ifdef _WIN32
	const int32_t LuceneThread::MAX_PRIORITY = THREAD_PRIORITY_HIGHEST;
	const int32_t LuceneThread::NORM_PRIORITY = THREAD_PRIORITY_NORMAL;
	const int32_t LuceneThread::MIN_PRIORITY = THREAD_PRIORITY_LOWEST;
	#else
	const int32_t LuceneThread::MAX_PRIORITY = 2;
	const int32_t LuceneThread::NORM_PRIORITY = 0;
	const int32_t LuceneThread::MIN_PRIORITY = -2;
	#endif
	
    LuceneThread::LuceneThread()
    {
        running = false;
    }
    
    LuceneThread::~LuceneThread()
    {
    }
    
    void LuceneThread::start()
    {
        setRunning(false);
        thread = newInstance<boost::thread>(LuceneThread::runThread, this);
        setRunning(true);
    }
    
    void LuceneThread::runThread(LuceneThread* thread)
    {
        LuceneThreadPtr threadObject(thread->shared_from_this());
        try
        {
            threadObject->run();
        }
        catch (...)
        {
        }
        threadObject->setRunning(false);
        threadObject.reset();
        ReleaseThreadCache();
    }

    void LuceneThread::setRunning(bool running)
    {
        SyncLock syncLock(this);
        this->running = running;
    }
    
    bool LuceneThread::isRunning()
    {
        SyncLock syncLock(this);
        return running;
    }
    
    bool LuceneThread::isAlive()
    {
        return (thread && isRunning());
    }
    
    void LuceneThread::setPriority(int32_t priority)
    {
        #ifdef _WIN32
        if (thread)
            SetThreadPriority(thread->native_handle(), priority);
        #endif
    }
    
    int32_t LuceneThread::getPriority()
    {
        #ifdef _WIN32
        return thread ? GetThreadPriority(thread->native_handle()) : NORM_PRIORITY;
        #else
        return NORM_PRIORITY;
        #endif
    }
    
    void LuceneThread::yield()
    {
        if (thread)
            thread->yield();
    }
    
    bool LuceneThread::join(int32_t timeout)
    {
        while (isAlive() && !thread->timed_join(boost::posix_time::milliseconds(timeout)))
        {
            if (timeout != 0)
                return false;
            if (thread->timed_join(boost::posix_time::milliseconds(10)))
                return true;
        }
        return true;
    }
    
    ThreadId LuceneThread::nullId()
    {
        #ifdef _WIN32
        return 0;
        #else
        return ThreadId();
        #endif
    }
    
    ThreadId LuceneThread::currentId()
    {
        #ifdef _WIN32
        return ::GetCurrentThreadId();
        #else
        return boost::this_thread::get_id();
        #endif
    }
}
