/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SINGLEINSTANCELOCKFACTORY_H
#define SINGLEINSTANCELOCKFACTORY_H

#include "LockFactory.h"
#include "Lock.h"

namespace Lucene
{
    /// Implements {@link LockFactory} for a single in-process instance, meaning all 
    /// locking will take place through this one instance.  Only use this {@link LockFactory} 
    /// when you are certain all IndexReaders and IndexWriters for a given index are running
    /// against a single shared in-process Directory instance.  This is currently the 
    /// default locking for RAMDirectory.
    /// @see LockFactory
    class LPPAPI SingleInstanceLockFactory : public LockFactory
    {
    public:
        SingleInstanceLockFactory();
        virtual ~SingleInstanceLockFactory();
        
        LUCENE_CLASS(SingleInstanceLockFactory);

    protected:
        HashSet<String> locks;
        
    public:
        /// Return a new Lock instance identified by lockName.
        /// @param lockName name of the lock to be created.
        virtual LockPtr makeLock(const String& lockName);
        
        /// Attempt to clear (forcefully unlock and remove) the
        /// specified lock.  Only call this at a time when you are
        /// certain this lock is no longer in use.
        /// @param lockName name of the lock to be cleared.
        virtual void clearLock(const String& lockName);
    };
    
    class LPPAPI SingleInstanceLock : public Lock
    {
    public:
        SingleInstanceLock(HashSet<String> locks, const String& lockName);
        virtual ~SingleInstanceLock();
        
        LUCENE_CLASS(SingleInstanceLock);
                
    protected:
        HashSet<String> locks;
        String lockName;
        
    public:
        /// Attempts to obtain exclusive access and immediately return
        /// upon success or failure.
        /// @return true if exclusive access is obtained.
        virtual bool obtain();
        
        /// Releases exclusive access.
        virtual void release();
        
        /// Returns true if the resource is currently locked. Note that
        /// one must still call {@link #obtain()} before using the resource.
        virtual bool isLocked();
        
        /// Returns derived object name.
        virtual String toString();
    };
}

#endif
