/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockLockFactory.h"
#include "MockLock.h"

namespace Lucene {

MockLockFactory::MockLockFactory() {
    locksCreated = MapStringLock::newInstance();
    lockPrefixSet = false;
    makeLockCount = 0;
}

MockLockFactory::~MockLockFactory() {
}

void MockLockFactory::setLockPrefix(const String& lockPrefix) {
    LockFactory::setLockPrefix(lockPrefix);
    lockPrefixSet = true;
}

LockPtr MockLockFactory::makeLock(const String& lockName) {
    LockPtr lock(newLucene<MockLock>());
    SyncLock createdLock(&locksCreated);
    locksCreated.put(lockName, lock);
    ++makeLockCount;
    return lock;
}

void MockLockFactory::clearLock(const String& lockName) {
}

}
