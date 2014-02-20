/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

typedef BaseTokenStreamFixture NumericTokenStreamTest;

static int64_t lvalue = 4573245871874382LL;
static int32_t ivalue = 123456;

TEST_F(NumericTokenStreamTest, testLongStream) {
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>()->setLongValue(lvalue);
    // use getAttribute to test if attributes really exist, if not an IAE will be thrown
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = stream->getAttribute<TypeAttribute>();
    for (int32_t shift = 0; shift < 64; shift += NumericUtils::PRECISION_STEP_DEFAULT) {
        EXPECT_TRUE(stream->incrementToken());
        EXPECT_EQ(NumericUtils::longToPrefixCoded(lvalue, shift), termAtt->term());
        EXPECT_EQ(shift == 0 ? NumericTokenStream::TOKEN_TYPE_FULL_PREC() : NumericTokenStream::TOKEN_TYPE_LOWER_PREC(), typeAtt->type());
    }
    EXPECT_TRUE(!stream->incrementToken());
}

TEST_F(NumericTokenStreamTest, testIntStream) {
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>()->setIntValue(ivalue);
    // use getAttribute to test if attributes really exist, if not an IAE will be thrown
    TermAttributePtr termAtt = stream->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = stream->getAttribute<TypeAttribute>();
    for (int32_t shift = 0; shift < 32; shift += NumericUtils::PRECISION_STEP_DEFAULT) {
        EXPECT_TRUE(stream->incrementToken());
        EXPECT_EQ(NumericUtils::intToPrefixCoded(ivalue, shift), termAtt->term());
        EXPECT_EQ(shift == 0 ? NumericTokenStream::TOKEN_TYPE_FULL_PREC() : NumericTokenStream::TOKEN_TYPE_LOWER_PREC(), typeAtt->type());
    }
    EXPECT_TRUE(!stream->incrementToken());
}

TEST_F(NumericTokenStreamTest, testNotInitialized) {
    NumericTokenStreamPtr stream = newLucene<NumericTokenStream>();
    try {
        stream->reset();
    } catch (IllegalStateException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalState)(e));
    }
    try {
        stream->incrementToken();
    } catch (IllegalStateException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalState)(e));
    }
}
