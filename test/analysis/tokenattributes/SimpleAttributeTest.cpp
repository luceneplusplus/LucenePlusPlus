/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(SimpleAttributeTest, LuceneTestFixture)

static AttributePtr checkCloneIsEqual(AttributePtr att)
{
    AttributePtr clone = boost::dynamic_pointer_cast<Attribute>(att->clone());
    BOOST_CHECK(att->equals(clone));
    BOOST_CHECK_EQUAL(att->hashCode(), clone->hashCode());
    return clone;
}

template <class ATTR>
static AttributePtr checkCopyIsEqual(AttributePtr att)
{
    AttributePtr copy = newLucene<ATTR>();
    att->copyTo(copy);
    BOOST_CHECK(att->equals(copy));
    BOOST_CHECK_EQUAL(att->hashCode(), copy->hashCode());
    return copy;
}

BOOST_AUTO_TEST_CASE(testFlagsAttribute)
{
    FlagsAttributePtr att = newLucene<FlagsAttribute>();
    BOOST_CHECK_EQUAL(0, att->getFlags());

    att->setFlags(1234);
    BOOST_CHECK_EQUAL(L"flags=1234", att->toString());

    FlagsAttributePtr att2 = boost::dynamic_pointer_cast<FlagsAttribute>(checkCloneIsEqual(att));
    BOOST_CHECK_EQUAL(1234, att2->getFlags());

    att2 = boost::dynamic_pointer_cast<FlagsAttribute>(checkCopyIsEqual<FlagsAttribute>(att));
    BOOST_CHECK_EQUAL(1234, att2->getFlags());

    att->clear();
    BOOST_CHECK_EQUAL(0, att->getFlags());
}

BOOST_AUTO_TEST_CASE(testPositionIncrementAttribute)
{
    PositionIncrementAttributePtr att = newLucene<PositionIncrementAttribute>();
    BOOST_CHECK_EQUAL(1, att->getPositionIncrement());

    att->setPositionIncrement(1234);
    BOOST_CHECK_EQUAL(L"positionIncrement=1234", att->toString());

    PositionIncrementAttributePtr att2 = boost::dynamic_pointer_cast<PositionIncrementAttribute>(checkCloneIsEqual(att));
    BOOST_CHECK_EQUAL(1234, att2->getPositionIncrement());

    att2 = boost::dynamic_pointer_cast<PositionIncrementAttribute>(checkCopyIsEqual<PositionIncrementAttribute>(att));
    BOOST_CHECK_EQUAL(1234, att2->getPositionIncrement());

    att->clear();
    BOOST_CHECK_EQUAL(1, att->getPositionIncrement());
}

namespace TestTypeAttribute
{
    class TestableTypeAttribute : public TypeAttribute
    {
    public:
        virtual ~TestableTypeAttribute()
        {
        }
        
        LUCENE_CLASS(TestableTypeAttribute);
        
    public:
        using TypeAttribute::DEFAULT_TYPE;
    };
}

BOOST_AUTO_TEST_CASE(testTypeAttribute)
{
    TypeAttributePtr att = newLucene<TestTypeAttribute::TestableTypeAttribute>();
    BOOST_CHECK_EQUAL(TestTypeAttribute::TestableTypeAttribute::DEFAULT_TYPE(), att->type());

    att->setType(L"hello");
    BOOST_CHECK_EQUAL(L"type=hello", att->toString());

    TypeAttributePtr att2 = boost::dynamic_pointer_cast<TypeAttribute>(checkCloneIsEqual(att));
    BOOST_CHECK_EQUAL(L"hello", att2->type());

    att2 = boost::dynamic_pointer_cast<TypeAttribute>(checkCopyIsEqual<TypeAttribute>(att));
    BOOST_CHECK_EQUAL(L"hello", att2->type());

    att->clear();
    BOOST_CHECK_EQUAL(TestTypeAttribute::TestableTypeAttribute::DEFAULT_TYPE(), att->type());
}

BOOST_AUTO_TEST_CASE(testPayloadAttribute)
{
    PayloadAttributePtr att = newLucene<PayloadAttribute>();
    BOOST_CHECK(!att->getPayload());

    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;
    
    PayloadPtr pl = newLucene<Payload>(payload);
    att->setPayload(pl);

    PayloadAttributePtr att2 = boost::dynamic_pointer_cast<PayloadAttribute>(checkCloneIsEqual(att));
    BOOST_CHECK(pl->equals(att2->getPayload()));
    BOOST_CHECK_NE(pl, att2->getPayload());

    att2 = boost::dynamic_pointer_cast<PayloadAttribute>(checkCopyIsEqual<PayloadAttribute>(att));
    BOOST_CHECK(pl->equals(att2->getPayload()));
    BOOST_CHECK_NE(pl, att2->getPayload());

    att->clear();
    BOOST_CHECK(!att->getPayload());
}

BOOST_AUTO_TEST_CASE(testOffsetAttribute)
{
    OffsetAttributePtr att = newLucene<OffsetAttribute>();
    BOOST_CHECK_EQUAL(0, att->startOffset());
    BOOST_CHECK_EQUAL(0, att->endOffset());

    att->setOffset(12, 34);
    // no string test here, because order unknown
    
    OffsetAttributePtr att2 = boost::dynamic_pointer_cast<OffsetAttribute>(checkCloneIsEqual(att));
    BOOST_CHECK_EQUAL(12, att2->startOffset());
    BOOST_CHECK_EQUAL(34, att2->endOffset());

    att2 = boost::dynamic_pointer_cast<OffsetAttribute>(checkCopyIsEqual<OffsetAttribute>(att));
    BOOST_CHECK_EQUAL(12, att2->startOffset());
    BOOST_CHECK_EQUAL(34, att2->endOffset());

    att->clear();
    BOOST_CHECK_EQUAL(0, att->startOffset());
    BOOST_CHECK_EQUAL(0, att->endOffset());
}

BOOST_AUTO_TEST_SUITE_END()
