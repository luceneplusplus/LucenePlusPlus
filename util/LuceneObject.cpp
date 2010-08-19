/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneObject.h"
#include "Synchronize.h"
#include "LuceneSignal.h"

namespace Lucene
{
    boost::mutex LuceneObject::lockMutex;
    
    LuceneObject::~LuceneObject()
    {
    }
    
    void LuceneObject::initialize()
    {
        // override
    }
    
    LuceneObjectPtr LuceneObject::clone(LuceneObjectPtr other)
    {
        if (!other)
            boost::throw_exception(UnsupportedOperationException(L"clone must not be null"));
        return other;
    }
    
    int32_t LuceneObject::hashCode()
    {
        return (int32_t)this;
    }
    
    bool LuceneObject::equals(LuceneObjectPtr other)
    {
        return (other && this == other.get());
    }
    
    int32_t LuceneObject::compareTo(LuceneObjectPtr other)
    {
        return (int32_t)(this - other.get());
    }
    
    String LuceneObject::toString()
    {
        return StringUtils::toString(hashCode());
    }
    
    SynchronizePtr LuceneObject::getSync()
    {
        boost::mutex::scoped_lock syncLock(lockMutex);
        if (!objectLock)
            objectLock = newInstance<Synchronize>();
        return objectLock;
    }
    
    LuceneSignalPtr LuceneObject::getSignal()
    {
        if (!objectSignal)
            objectSignal = newInstance<LuceneSignal>(getSync());
        return objectSignal;
    }
    
    void LuceneObject::lock(int32_t timeout)
    {
        getSync()->lock();
    }
    
    void LuceneObject::unlock()
    {
        getSync()->unlock();
    }
    
    bool LuceneObject::holdsLock()
    {
        return getSync()->holdsLock();
    }
    
    void LuceneObject::wait(int32_t timeout)
    {
        getSignal()->wait(timeout);
    }
    
    void LuceneObject::notifyAll()
    {
        getSignal()->notifyAll();
    }
}
