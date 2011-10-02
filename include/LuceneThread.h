/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENETHREAD_H
#define LUCENETHREAD_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Lucene thread container.
    class LPPAPI LuceneThread : public LuceneObject
    {
    public:
        LuceneThread();
        virtual ~LuceneThread();
        
        LUCENE_CLASS(LuceneThread);
    
    public:        
        static const int32_t MAX_PRIORITY;
        static const int32_t NORM_PRIORITY;
        static const int32_t MIN_PRIORITY;
    
    protected:
        #ifdef _WIN32
        HANDLE thread;
        #else
        pthread_t thread;
        #endif
        
        /// Flag to indicate running thread.
        /// @see #isAlive
        bool running;
        
        int32_t threadPriority;
                
    public:
        /// start thread see {@link #run}.
        virtual void start();
        
        /// return whether thread is current running.
        virtual bool isAlive();
        
        /// set running thread priority.
        virtual void setPriority(int32_t priority);
        
        /// return running thread priority.
        virtual int32_t getPriority();
        
        /// wait for thread to finish using an optional timeout.
        virtual bool join(int32_t timeout = 0);
        
        /// override to provide the body of the thread.
        virtual void run() = 0;
        
        /// Return representation of current execution thread.
        static int64_t currentId();
        
        /// Suspends current execution thread for a given time.
        static void threadSleep(int32_t time);
        
        /// Yield current execution thread.
        static void threadYield();
    
    protected:
        /// function that controls the lifetime of the running thread.
        #ifdef _WIN32
        static DWORD __stdcall runThread(void* thread);
        #else
        static void* runThread(void* thread);
        #endif
    };
}

#endif
