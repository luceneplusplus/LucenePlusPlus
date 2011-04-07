/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <float.h>
#include "LuceneTestFixture.h"
#include "DocValues.h"
#include "MiscUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(DocValuesTest, LuceneTestFixture)

DECLARE_SHARED_PTR(TestableDocValues)

class TestableDocValues : public DocValues
{
public:
    TestableDocValues(Collection<double> innerArray)
    {
        this->innerArray = innerArray;
    }
    
    virtual ~TestableDocValues()
    {
    }

public:
    Collection<double> innerArray;

public:
    virtual double doubleVal(int32_t doc)
    {
        if (doc < 0 || doc >= innerArray.size())
            boost::throw_exception(IndexOutOfBoundsException());
        return innerArray[doc];
    }
    
    virtual String toString(int32_t doc)
    {
        return StringUtils::toString(doc);
    }
};

BOOST_AUTO_TEST_CASE(testGetMinValue)
{
    Collection<double> innerArray = newCollection<double>(1.0, 2.0, -1.0, 100.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(-1.0, docValues->getMinValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK(MiscUtils::isNaN(docValues->getMinValue()));
}

BOOST_AUTO_TEST_CASE(testGetMaxValue)
{
    Collection<double> innerArray = newCollection<double>(1.0, 2.0, -1.0, 10.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(10.0, docValues->getMaxValue());

    innerArray = newCollection<double>(-3.0, -1.0, -100.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(-1.0, docValues->getMaxValue());

    innerArray = newCollection<double>(-3.0, -1.0, -100.0, DBL_MAX, DBL_MAX - 1);
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(DBL_MAX, docValues->getMaxValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK(MiscUtils::isNaN(docValues->getMaxValue()));
}

BOOST_AUTO_TEST_CASE(testGetAverageValue)
{
    Collection<double> innerArray = newCollection<double>(1.0, 1.0, 1.0, 1.0);
    TestableDocValuesPtr docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(1.0, docValues->getAverageValue());

    innerArray = newCollection<double>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(3.5, docValues->getAverageValue());

    // test with negative values
    innerArray = newCollection<double>(-1.0, 2.0);
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK_EQUAL(0.5, docValues->getAverageValue());

    // test with without values - NaN
    innerArray = Collection<double>::newInstance();
    docValues = newLucene<TestableDocValues>(innerArray);
    BOOST_CHECK(MiscUtils::isNaN(docValues->getAverageValue()));
}

BOOST_AUTO_TEST_SUITE_END()
