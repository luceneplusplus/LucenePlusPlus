/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
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

BOOST_FIXTURE_TEST_SUITE(AttributeSourceTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testCaptureState)
{
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
    BOOST_CHECK_NE(hashCode, src->hashCode());

    src->restoreState(state);
    BOOST_CHECK_EQUAL(L"TestTerm", termAtt->term());
    BOOST_CHECK_EQUAL(L"TestType", typeAtt->type());
    BOOST_CHECK_EQUAL(hashCode, src->hashCode());

    // restore into an exact configured copy
    AttributeSourcePtr copy = newLucene<AttributeSource>();
    copy->addAttribute<TermAttribute>();
    copy->addAttribute<TypeAttribute>();
    copy->restoreState(state);
    BOOST_CHECK_EQUAL(src->hashCode(), copy->hashCode());
    BOOST_CHECK(src->equals(copy));

    // init a second instance (with attributes in different order and one additional attribute)
    AttributeSourcePtr src2 = newLucene<AttributeSource>();
    typeAtt = src2->addAttribute<TypeAttribute>();
    FlagsAttributePtr flagsAtt = src2->addAttribute<FlagsAttribute>();
    termAtt = src2->addAttribute<TermAttribute>();
    flagsAtt->setFlags(12345);

    src2->restoreState(state);
    BOOST_CHECK_EQUAL(L"TestTerm", termAtt->term());
    BOOST_CHECK_EQUAL(L"TestType", typeAtt->type());
    BOOST_CHECK_EQUAL(12345, flagsAtt->getFlags());

    // init a third instance missing one Attribute
    AttributeSourcePtr src3 = newLucene<AttributeSource>();
    termAtt = src3->addAttribute<TermAttribute>();
    BOOST_CHECK_EXCEPTION(src3->restoreState(state), IllegalArgumentException, check_exception(LuceneException::IllegalArgument));
}

BOOST_AUTO_TEST_CASE(testCloneAttributes)
{
    AttributeSourcePtr src = newLucene<AttributeSource>();
    TermAttributePtr termAtt = src->addAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = src->addAttribute<TypeAttribute>();
    termAtt->setTermBuffer(L"TestTerm");
    typeAtt->setType(L"TestType");

    AttributeSourcePtr clone = src->cloneAttributes();
    Collection<AttributePtr> attributes = clone->getAttributes();
    BOOST_CHECK_EQUAL(2, attributes.size());
    BOOST_CHECK(MiscUtils::typeOf<TermAttribute>(attributes[0]));
    BOOST_CHECK(MiscUtils::typeOf<TypeAttribute>(attributes[1]));
    
    TermAttributePtr termAtt2 = clone->getAttribute<TermAttribute>();
    TypeAttributePtr typeAtt2 = clone->getAttribute<TypeAttribute>();
    BOOST_CHECK(termAtt2 != termAtt);
    BOOST_CHECK(typeAtt2 != typeAtt);
    BOOST_CHECK(termAtt2->equals(termAtt));
    BOOST_CHECK(typeAtt2->equals(typeAtt));
}

BOOST_AUTO_TEST_CASE(testToStringAndMultiAttributeImplementations)
{
    AttributeSourcePtr src = newLucene<AttributeSource>();
    TermAttributePtr termAtt = src->addAttribute<TermAttribute>();
    TypeAttributePtr typeAtt = src->addAttribute<TypeAttribute>();
    termAtt->setTermBuffer(L"TestTerm");
    typeAtt->setType(L"TestType");
    
    BOOST_CHECK_EQUAL(L"(" + termAtt->toString() + L"," + typeAtt->toString() + L")", src->toString());
    Collection<AttributePtr> attributes = src->getAttributes();
    BOOST_CHECK_EQUAL(2, attributes.size());
    BOOST_CHECK(attributes[0]->equals(termAtt));
    BOOST_CHECK(attributes[1]->equals(typeAtt));
}

BOOST_AUTO_TEST_CASE(testDefaultAttributeFactory)
{
    AttributeSourcePtr src = newLucene<AttributeSource>();
    BOOST_CHECK(MiscUtils::typeOf<TermAttribute>(src->addAttribute<TermAttribute>()));
    BOOST_CHECK(MiscUtils::typeOf<OffsetAttribute>(src->addAttribute<OffsetAttribute>()));
    BOOST_CHECK(MiscUtils::typeOf<FlagsAttribute>(src->addAttribute<FlagsAttribute>()));
    BOOST_CHECK(MiscUtils::typeOf<PayloadAttribute>(src->addAttribute<PayloadAttribute>()));
    BOOST_CHECK(MiscUtils::typeOf<PositionIncrementAttribute>(src->addAttribute<PositionIncrementAttribute>()));
    BOOST_CHECK(MiscUtils::typeOf<TypeAttribute>(src->addAttribute<TypeAttribute>()));
}

BOOST_AUTO_TEST_SUITE_END()
