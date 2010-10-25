/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NOLOCKFACTORY_H
#define NOLOCKFACTORY_H

#include "LockFactory.h"
#include "Lock.h"

namespace Lucene
{
    /// Use this {@link LockFactory} to disable locking entirely.  Only one instance of this lock is created.  
    /// You should call {@link #getNoLockFactory()} to get the instance.
    ///
    /// @see LockFactory
    class LPPAPI NoLockFactory : public LockFactory
    {
    public:
        virtual ~NoLockFactory();
        
        LUCENE_CLASS(NoLockFactory);
            
    public:
        static NoLockFactoryPtr getNoLockFactory();
        static NoLockPtr getSingletonLock();
        
        /// Return a new Lock instance identified by lockName.
        virtual LockPtr makeLock(const String& lockName);
        
        /// Attempt to clear (forcefully unlock and remove) the specified lock.  Only call this at a time when you 
        /// are certain this lock is no longer in use.
        virtual void clearLock(const String& lockName);
    };
    
    class LPPAPI NoLock : public Lock
    {
    public:
        virtual ~NoLock();
        
        LUCENE_CLASS(NoLock);
            
    public:
        virtual bool obtain();
        virtual void release();
        virtual bool isLocked();
        virtual String toString();
    };
}

#endif
