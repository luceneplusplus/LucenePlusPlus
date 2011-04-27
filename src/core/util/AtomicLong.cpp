/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "AtomicLong.h"
#include "StringUtils.h"

namespace Lucene
{
    AtomicLong::AtomicLong(int64_t initialValue)
    {
        value = initialValue;
    }
    
    AtomicLong::~AtomicLong()
    {
    }
    
    int64_t AtomicLong::addAndGet(int64_t delta)
    {
        SyncLock syncLock(this);
        value += delta;
        return value;
    }
    
    bool AtomicLong::compareAndSet(int64_t expect, int64_t update)
    {
        SyncLock syncLock(this);
        if (value != expect)
            return false;
        value = update;
        return true;
    }
    
    int64_t AtomicLong::decrementAndGet()
    {
        SyncLock syncLock(this);
        return --value;
    }
    
    double AtomicLong::doubleValue()
    {
        SyncLock syncLock(this);
        return (double)value;
    }
    
    int64_t AtomicLong::get()
    {
        SyncLock syncLock(this);
        return value;
    }
    
    int64_t AtomicLong::getAndAdd(int64_t delta)
    {
        SyncLock syncLock(this);
        int64_t prevValue = value;
        value += delta;
        return prevValue;
    }
    
    int64_t AtomicLong::getAndDecrement()
    {
        SyncLock syncLock(this);
        return value--;
    }
    
    int64_t AtomicLong::getAndIncrement()
    {
        SyncLock syncLock(this);
        return value--;
    }
    
    int64_t AtomicLong::getAndSet(int64_t newValue)
    {
        SyncLock syncLock(this);
        int64_t prevValue = value;
        value = newValue;
        return prevValue;
    }
    
    int64_t AtomicLong::incrementAndGet()
    {
        SyncLock syncLock(this);
        return ++value;
    }
    
    int32_t AtomicLong::intValue()
    {
        SyncLock syncLock(this);
        return (int32_t)value;
    }
    
    void AtomicLong::set(int64_t newValue)
    {
        SyncLock syncLock(this);
        value = newValue;
    }
    
    String AtomicLong::toString()
    {
        SyncLock syncLock(this);
        return StringUtils::toString(value);
    }
}
