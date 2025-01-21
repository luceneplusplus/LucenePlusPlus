/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ThreadPool.h"

namespace Lucene {

Future::~Future() {
}

const int32_t ThreadPool::THREADPOOL_SIZE = 5;

ThreadPool::ThreadPool()
    :
        work(boost::asio::make_work_guard(io_context))
{
    for (int32_t i = 0; i < THREADPOOL_SIZE; ++i) {
        threadGroup.create_thread(boost::bind(&boost::asio::io_context::run, &io_context));
    }
}

ThreadPool::~ThreadPool() {
    threadGroup.join_all(); // wait for all competition
}

ThreadPoolPtr ThreadPool::getInstance() {
    static ThreadPoolPtr threadPool;
    LUCENE_RUN_ONCE(
        threadPool = newLucene<ThreadPool>();
        CycleCheck::addStatic(threadPool);
    );
    return threadPool;
}

}
