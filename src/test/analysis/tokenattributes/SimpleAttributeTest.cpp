/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "FlagsAttribute.h"
#include "PositionIncrementAttribute.h"
#include "TypeAttribute.h"
#include "PayloadAttribute.h"
#include "Payload.h"
#include "OffsetAttribute.h"

using namespace Lucene;

typedef LuceneTestFixture SimpleAttributeTest;

static AttributePtr checkCloneIsEqual(const AttributePtr& att) {
    AttributePtr clone = boost::dynamic_pointer_cast<Attribute>(att->clone());
    EXPECT_TRUE(att->equals(clone));
    EXPECT_EQ(att->hashCode(), clone->hashCode());
    return clone;
}

template <class ATTR>
static AttributePtr checkCopyIsEqual(const AttributePtr& att) {
    AttributePtr copy = newLucene<ATTR>();
    att->copyTo(copy);
    EXPECT_TRUE(att->equals(copy));
    EXPECT_EQ(att->hashCode(), copy->hashCode());
    return copy;
}

TEST_F(SimpleAttributeTest, testFlagsAttribute) {
    FlagsAttributePtr att = newLucene<FlagsAttribute>();
    EXPECT_EQ(0, att->getFlags());

    att->setFlags(1234);
    EXPECT_EQ(L"flags=1234", att->toString());

    FlagsAttributePtr att2 = boost::dynamic_pointer_cast<FlagsAttribute>(checkCloneIsEqual(att));
    EXPECT_EQ(1234, att2->getFlags());

    att2 = boost::dynamic_pointer_cast<FlagsAttribute>(checkCopyIsEqual<FlagsAttribute>(att));
    EXPECT_EQ(1234, att2->getFlags());

    att->clear();
    EXPECT_EQ(0, att->getFlags());
}

TEST_F(SimpleAttributeTest, testPositionIncrementAttribute) {
    PositionIncrementAttributePtr att = newLucene<PositionIncrementAttribute>();
    EXPECT_EQ(1, att->getPositionIncrement());

    att->setPositionIncrement(1234);
    EXPECT_EQ(L"positionIncrement=1234", att->toString());

    PositionIncrementAttributePtr att2 = boost::dynamic_pointer_cast<PositionIncrementAttribute>(checkCloneIsEqual(att));
    EXPECT_EQ(1234, att2->getPositionIncrement());

    att2 = boost::dynamic_pointer_cast<PositionIncrementAttribute>(checkCopyIsEqual<PositionIncrementAttribute>(att));
    EXPECT_EQ(1234, att2->getPositionIncrement());

    att->clear();
    EXPECT_EQ(1, att->getPositionIncrement());
}

namespace TestTypeAttribute {

class TestableTypeAttribute : public TypeAttribute {
public:
    virtual ~TestableTypeAttribute() {
    }

    LUCENE_CLASS(TestableTypeAttribute);

public:
    using TypeAttribute::DEFAULT_TYPE;
};

}

TEST_F(SimpleAttributeTest, testTypeAttribute) {
    TypeAttributePtr att = newLucene<TestTypeAttribute::TestableTypeAttribute>();
    EXPECT_EQ(TestTypeAttribute::TestableTypeAttribute::DEFAULT_TYPE(), att->type());

    att->setType(L"hello");
    EXPECT_EQ(L"type=hello", att->toString());

    TypeAttributePtr att2 = boost::dynamic_pointer_cast<TypeAttribute>(checkCloneIsEqual(att));
    EXPECT_EQ(L"hello", att2->type());

    att2 = boost::dynamic_pointer_cast<TypeAttribute>(checkCopyIsEqual<TypeAttribute>(att));
    EXPECT_EQ(L"hello", att2->type());

    att->clear();
    EXPECT_EQ(TestTypeAttribute::TestableTypeAttribute::DEFAULT_TYPE(), att->type());
}

TEST_F(SimpleAttributeTest, testPayloadAttribute) {
    PayloadAttributePtr att = newLucene<PayloadAttribute>();
    EXPECT_TRUE(!att->getPayload());

    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;

    PayloadPtr pl = newLucene<Payload>(payload);
    att->setPayload(pl);

    PayloadAttributePtr att2 = boost::dynamic_pointer_cast<PayloadAttribute>(checkCloneIsEqual(att));
    EXPECT_TRUE(pl->equals(att2->getPayload()));
    EXPECT_NE(pl, att2->getPayload());

    att2 = boost::dynamic_pointer_cast<PayloadAttribute>(checkCopyIsEqual<PayloadAttribute>(att));
    EXPECT_TRUE(pl->equals(att2->getPayload()));
    EXPECT_NE(pl, att2->getPayload());

    att->clear();
    EXPECT_TRUE(!att->getPayload());
}

TEST_F(SimpleAttributeTest, testOffsetAttribute) {
    OffsetAttributePtr att = newLucene<OffsetAttribute>();
    EXPECT_EQ(0, att->startOffset());
    EXPECT_EQ(0, att->endOffset());

    att->setOffset(12, 34);
    // no string test here, because order unknown

    OffsetAttributePtr att2 = boost::dynamic_pointer_cast<OffsetAttribute>(checkCloneIsEqual(att));
    EXPECT_EQ(12, att2->startOffset());
    EXPECT_EQ(34, att2->endOffset());

    att2 = boost::dynamic_pointer_cast<OffsetAttribute>(checkCopyIsEqual<OffsetAttribute>(att));
    EXPECT_EQ(12, att2->startOffset());
    EXPECT_EQ(34, att2->endOffset());

    att->clear();
    EXPECT_EQ(0, att->startOffset());
    EXPECT_EQ(0, att->endOffset());
}
