/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ATOMICLONG_H
#define ATOMICLONG_H

#include "LuceneObject.h"

namespace Lucene
{
    class AtomicLong : public LuceneObject
    {
    public:
        AtomicLong(int64_t initialValue = 0);
        virtual ~AtomicLong();
        
        LUCENE_CLASS(AtomicLong);
    
    private:
        int64_t value;
    
    public:
        int64_t addAndGet(int64_t delta);
        bool compareAndSet(int64_t expect, int64_t update);
        int64_t decrementAndGet();
        double doubleValue();
        int64_t get();
        int64_t getAndAdd(int64_t delta);
        int64_t getAndDecrement();
        int64_t getAndIncrement();
        int64_t getAndSet(int64_t newValue);
        int64_t incrementAndGet();
        int32_t intValue();
        void set(int64_t newValue);
        
        virtual String toString();
    };
}

#endif
