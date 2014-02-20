/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BASETESTRANGEFILTERFIXTURE_H
#define BASETESTRANGEFILTERFIXTURE_H

#include "LuceneTestFixture.h"
#include "LuceneObject.h"

namespace Lucene {

DECLARE_SHARED_PTR(TestIndex)

/// Collation interacts badly with hyphens -- collation produces different ordering than Unicode code-point ordering,
/// so two indexes are created:
/// one which can't have negative random integers, for testing collated ranges, and the other which can have negative
/// random integers, for all other tests.
class TestIndex : public LuceneObject {
public:
    TestIndex(int32_t minR, int32_t maxR, bool allowNegativeRandomInts);
    virtual ~TestIndex();

    LUCENE_CLASS(TestIndex);

public:
    int32_t maxR;
    int32_t minR;
    bool allowNegativeRandomInts;
    RAMDirectoryPtr index;
};

class BaseTestRangeFilterFixture : public LuceneTestFixture {
public:
    BaseTestRangeFilterFixture();
    virtual ~BaseTestRangeFilterFixture();

public:
    TestIndexPtr signedIndex;
    TestIndexPtr unsignedIndex;

    int32_t minId;
    int32_t maxId;
    int32_t intLength;
    RandomPtr random;

protected:
    void build(const TestIndexPtr& index);
    String pad(int32_t n);
};

}

#endif
