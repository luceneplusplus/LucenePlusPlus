/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <boost/thread/thread.hpp>
#include "LuceneThread.h"

namespace Lucene
{
    #if defined(_WIN32) || defined(_WIN64)
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
        thread = 0;
        threadPriority = NORM_PRIORITY;
    }

    LuceneThread::~LuceneThread()
    {
    }

    void LuceneThread::start()
    {
        #if defined(_WIN32) || defined(_WIN64)
        DWORD threadId;
        #if defined(LPP_USE_GC)
        thread = GC_CreateThread(NULL, 0, LuceneThread::runThread, (void*)this, 0, &threadId);
        #else
        thread = CreateThread(NULL, 0, LuceneThread::runThread, (void*)this, 0, &threadId);
        #endif
        running = true;
        #else
        // todo
        // todo: what to call if GC or not GC?
        // GC_pthread_create
        // pthread_create
        #endif
        setPriority(threadPriority);
    }

    #if defined(_WIN32) || defined(_WIN64)
    DWORD __stdcall LuceneThread::runThread(void* threadPtr)
    #else
    void* LuceneThread::runThread(void* threadPtr)
    #endif
    {
        LuceneThread* thread = reinterpret_cast<LuceneThread*>(threadPtr);
        try
        {
            thread->run();
        }
        catch (...)
        {
        }
        thread->running = false;
        ReleaseThreadCache();
        return 0;
    }

    bool LuceneThread::isAlive()
    {
        return (thread && running);
    }

    void LuceneThread::setPriority(int32_t priority)
    {
        threadPriority = priority;
        #if defined(_WIN32) || defined(_WIN64)
        if (thread)
            SetThreadPriority(thread, threadPriority);
        #endif
    }

    int32_t LuceneThread::getPriority()
    {
        return threadPriority;
    }

    bool LuceneThread::join(int32_t timeout)
    {
        #if defined(_WIN32) || defined(_WIN64)
        if (!thread)
            return true;
        if (WaitForSingleObject(thread, timeout == 0 ? INFINITE : timeout) != WAIT_OBJECT_0)
            return false;
        CloseHandle(thread);
        thread = 0;
        return true;
        #else
        // todo
        // todo: see http://www.linuxquestions.org/questions/programming-9/pthread_join-with-timeout-365304/
        // todo: see http://pubs.opengroup.org/onlinepubs/000095399/xrat/xsh_chap02.html#tag_03_02_08_21
        #endif
    }

    int64_t LuceneThread::currentId()
    {
        #if defined(_WIN32) || defined(_WIN64)
        return (int64_t)GetCurrentThreadId();
        #else
        return (int64_t)pthread_self();
        #endif
    }

    void LuceneThread::threadSleep(int32_t time)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(time));
    }

    void LuceneThread::threadYield()
    {
        boost::this_thread::yield();
    }
}
