/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ThreadPool.h"

namespace Lucene
{
    Future::~Future()
    {
    }

    ThreadFunction::ThreadFunction(const boost::asio::io_service& io_service) : service(io_service)
    {
    }

    ThreadFunction::~ThreadFunction()
    {
    }

    void ThreadFunction::run()
    {
        const_cast<boost::asio::io_service&>(service).run();
    }

    const int32_t ThreadPool::THREADPOOL_SIZE = 5;

    ThreadPool::ThreadPool()
    {
        work.reset(new boost::asio::io_service::work(io_service));
        threads = Collection<LuceneThreadPtr>::newInstance(THREADPOOL_SIZE);
        for (int32_t i = 0; i < THREADPOOL_SIZE; ++i)
        {
            threads[i] = newLucene<ThreadFunction>(io_service);
            threads[i]->start();
        }
    }

    ThreadPool::~ThreadPool()
    {
        work.reset(); // stop all threads
        for (int32_t i = 0; i < THREADPOOL_SIZE; ++i)
            threads[i]->join();
    }

    ThreadPoolPtr ThreadPool::getInstance()
    {
        static ThreadPoolPtr threadPool;
        if (!threadPool)
            threadPool = newStaticLucene<ThreadPool>();
        return threadPool;
    }
}
