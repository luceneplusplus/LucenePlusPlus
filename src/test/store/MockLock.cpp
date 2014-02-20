/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockLock.h"

namespace Lucene {

MockLock::MockLock() {
    lockAttempts = 0;
}

MockLock::~MockLock() {
}

bool MockLock::obtain() {
    ++lockAttempts;
    return true;
}

void MockLock::release() {
    // do nothing
}

bool MockLock::isLocked() {
    return false;
}

String MockLock::toString() {
    return L"MockLock";
}

}
