/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockFilter.h"
#include "DocIdBitSet.h"
#include "BitSet.h"

namespace Lucene {

MockFilter::MockFilter() {
    _wasCalled = false;
}

MockFilter::~MockFilter() {
}

DocIdSetPtr MockFilter::getDocIdSet(const IndexReaderPtr& reader) {
    _wasCalled = true;
    return newLucene<DocIdBitSet>(newLucene<BitSet>());
}

void MockFilter::clear() {
    _wasCalled = false;
}

bool MockFilter::wasCalled() {
    return _wasCalled;
}

}
