/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "AttributeSource.h"
#include "TermAttribute.h"
#include "TypeAttribute.h"
#include "FlagsAttribute.h"
#include "OffsetAttribute.h"
#include "PayloadAttribute.h"
#include "PositionIncrementAttribute.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture AttributeSourceTest;

TEST_F(AttributeSourceTest, testCaptureState) {
    // init a first instance
    AttributeSourcePtr src = newLucene<AttributeSource>();
    TermAttributePtr termAtt = src->addAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = src->addAttribute<TypeAttribute>();
    termAtt->setTermBuffer(L"TestTerm");
    typeAtt->setType(L"TestType");
    int32_t hashCode = src->hashCode();

    AttributeSourceStatePtr state = src->captureState();

    // modify the attributes
    termAtt->setTermBuffer(L"AnotherTestTerm");
    typeAtt->setType(L"AnotherTestType");
    EXPECT_NE(hashCode, src->hashCode());

    src->restoreState(state);
    EXPECT_EQ(L"TestTerm", termAtt->term());
    EXPECT_EQ(L"TestType", typeAtt->type());
    EXPECT_EQ(hashCode, src->hashCode());

    // restore into an exact configured copy
    AttributeSourcePtr copy = newLucene<AttributeSource>();
    copy->addAttribute<TermAttribute>();
    copy->addAttribute<TypeAttribute>();
    copy->restoreState(state);
    EXPECT_EQ(src->hashCode(), copy->hashCode());
    EXPECT_TRUE(src->equals(copy));

    // init a second instance (with attributes in different order and one additional attribute)
    AttributeSourcePtr src2 = newLucene<AttributeSource>();
    typeAtt = src2->addAttribute<TypeAttribute>();
    FlagsAttributePtr flagsAtt = src2->addAttribute<FlagsAttribute>();
    termAtt = src2->addAttribute<TermAttribute>();
    flagsAtt->setFlags(12345);

    src2->restoreState(state);
    EXPECT_EQ(L"TestTerm", termAtt->term());
    EXPECT_EQ(L"TestType", typeAtt->type());
    EXPECT_EQ(12345, flagsAtt->getFlags());

    // init a third instance missing one Attribute
    AttributeSourcePtr src3 = newLucene<AttributeSource>();
    termAtt = src3->addAttribute<TermAttribute>();
    try {
        src3->restoreState(state);
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
}

TEST_F(AttributeSourceTest, testCloneAttributes) {
    AttributeSourcePtr src = newLucene<AttributeSource>();
    TermAttributePtr termAtt = src->addAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = src->addAttribute<TypeAttribute>();
    termAtt->setTermBuffer(L"TestTerm");
    typeAtt->setType(L"TestType");

    AttributeSourcePtr clone = src->cloneAttributes();
    Collection<AttributePtr> attributes = clone->getAttributes();
    EXPECT_EQ(2, attributes.size());
    EXPECT_TRUE(MiscUtils::typeOf<TermAttribute>(attributes[0]));
    EXPECT_TRUE(MiscUtils::typeOf<TypeAttribute>(attributes[1]));

    TermAttributePtr termAtt2 = clone->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt2 = clone->getAttribute<TypeAttribute>();
    EXPECT_TRUE(termAtt2 != termAtt);
    EXPECT_TRUE(typeAtt2 != typeAtt);
    EXPECT_TRUE(termAtt2->equals(termAtt));
    EXPECT_TRUE(typeAtt2->equals(typeAtt));
}

TEST_F(AttributeSourceTest, testToStringAndMultiAttributeImplementations) {
    AttributeSourcePtr src = newLucene<AttributeSource>();
    TermAttributePtr termAtt = src->addAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = src->addAttribute<TypeAttribute>();
    termAtt->setTermBuffer(L"TestTerm");
    typeAtt->setType(L"TestType");

    EXPECT_EQ(L"(" + termAtt->toString() + L"," + typeAtt->toString() + L")", src->toString());
    Collection<AttributePtr> attributes = src->getAttributes();
    EXPECT_EQ(2, attributes.size());
    EXPECT_TRUE(attributes[0]->equals(termAtt));
    EXPECT_TRUE(attributes[1]->equals(typeAtt));
}

TEST_F(AttributeSourceTest, testDefaultAttributeFactory) {
    AttributeSourcePtr src = newLucene<AttributeSource>();
    EXPECT_TRUE(MiscUtils::typeOf<TermAttribute>(src->addAttribute<TermAttribute>()));
    EXPECT_TRUE(MiscUtils::typeOf<OffsetAttribute>(src->addAttribute<OffsetAttribute>()));
    EXPECT_TRUE(MiscUtils::typeOf<FlagsAttribute>(src->addAttribute<FlagsAttribute>()));
    EXPECT_TRUE(MiscUtils::typeOf<PayloadAttribute>(src->addAttribute<PayloadAttribute>()));
    EXPECT_TRUE(MiscUtils::typeOf<PositionIncrementAttribute>(src->addAttribute<PositionIncrementAttribute>()));
    EXPECT_TRUE(MiscUtils::typeOf<TypeAttribute>(src->addAttribute<TypeAttribute>()));
}
