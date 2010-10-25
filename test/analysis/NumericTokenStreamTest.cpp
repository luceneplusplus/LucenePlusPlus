/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "NumericTokenStream.h"
#include "TermAttribute.h"
#include "TypeAttribute.h"
#include "NumericUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(NumericTokenStreamTest, BaseTokenStreamFixture)

static int64_t lvalue = 4573245871874382LL;
static int32_t ivalue = 123456;

BOOST_AUTO_TEST_CASE(testLongStream)
{
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>()->setLongValue(lvalue);
    // use getAttribute to test if attributes really exist, if not an IAE will be thrown
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = stream->getAttribute<TypeAttribute>();
    for (int32_t shift = 0; shift < 64; shift += NumericUtils::PRECISION_STEP_DEFAULT)
    {
        BOOST_CHECK(stream->incrementToken());
        BOOST_CHECK_EQUAL(NumericUtils::longToPrefixCoded(lvalue, shift), termAtt->term());
        BOOST_CHECK_EQUAL(shift == 0 ? NumericTokenStream::TOKEN_TYPE_FULL_PREC() : NumericTokenStream::TOKEN_TYPE_LOWER_PREC(), typeAtt->type());
    }
    BOOST_CHECK(!stream->incrementToken());
}

BOOST_AUTO_TEST_CASE(testIntStream)
{
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>()->setIntValue(ivalue);
    // use getAttribute to test if attributes really exist, if not an IAE will be thrown
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = stream->getAttribute<TypeAttribute>();
    for (int32_t shift = 0; shift < 32; shift += NumericUtils::PRECISION_STEP_DEFAULT)
    {
        BOOST_CHECK(stream->incrementToken());
        BOOST_CHECK_EQUAL(NumericUtils::intToPrefixCoded(ivalue, shift), termAtt->term());
        BOOST_CHECK_EQUAL(shift == 0 ? NumericTokenStream::TOKEN_TYPE_FULL_PREC() : NumericTokenStream::TOKEN_TYPE_LOWER_PREC(), typeAtt->type());
    }
    BOOST_CHECK(!stream->incrementToken());
}

BOOST_AUTO_TEST_CASE(testNotInitialized)
{
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>();
    BOOST_CHECK_EXCEPTION(stream->reset(), IllegalStateException, check_exception(LuceneException::IllegalState));
    BOOST_CHECK_EXCEPTION(stream->incrementToken(), IllegalStateException, check_exception(LuceneException::IllegalState));
}

BOOST_AUTO_TEST_SUITE_END()
