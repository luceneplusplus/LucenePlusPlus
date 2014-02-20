/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <float.h>
#include "LuceneTestFixture.h"
#include "DocValues.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture DocValuesTest;

DECLARE_SHARED_PTR(TestableDocValues)

class TestableDocValues : public DocValues {
public:
    TestableDocValues(Collection<double> innerArray) {
        this->innerArray = innerArray;
    }

    virtual ~TestableDocValues() {
    }

public:
    Collection<double> innerArray;

public:
    virtual double doubleVal(int32_t doc) {
        if (doc < 0 || doc >= innerArray.size()) {
            boost::throw_exception(IndexOutOfBoundsException());
        }
        return innerArray[doc];
    }

    virtual String toString(int32_t doc) {
        return StringUtils::toString(doc);
    }
};

TEST_F(DocValuesTest, testGetMinValue) {
    Collection<double> innerArray = newCollection<double>(1.0, 2.0, -1.0, 100.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(-1.0, docValues->getMinValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_TRUE(MiscUtils::isNaN(docValues->getMinValue()));
}

TEST_F(DocValuesTest, testGetMaxValue) {
    Collection<double> innerArray = newCollection<double>(1.0, 2.0, -1.0, 10.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(10.0, docValues->getMaxValue());

    innerArray = newCollection<double>(-3.0, -1.0, -100.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(-1.0, docValues->getMaxValue());

    innerArray = newCollection<double>(-3.0, -1.0, -100.0, DBL_MAX, DBL_MAX - 1);
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(DBL_MAX, docValues->getMaxValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_TRUE(MiscUtils::isNaN(docValues->getMaxValue()));
}

TEST_F(DocValuesTest, testGetAverageValue) {
    Collection<double> innerArray = newCollection<double>(1.0, 1.0, 1.0, 1.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(1.0, docValues->getAverageValue());

    innerArray = newCollection<double>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(3.5, docValues->getAverageValue());

    // test with negative values
    innerArray = newCollection<double>(-1.0, 2.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_EQ(0.5, docValues->getAverageValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    EXPECT_TRUE(MiscUtils::isNaN(docValues->getAverageValue()));
}
