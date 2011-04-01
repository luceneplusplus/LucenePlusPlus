/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "TermAttribute.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(TermAttributeTest, LuceneTestFixture)

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

BOOST_AUTO_TEST_CASE(testResize)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    for (int32_t i = 0; i < 2000; ++i)
    {
        t->resizeTermBuffer(i);
        BOOST_CHECK(i <= t->termBuffer().size());
        BOOST_CHECK_EQUAL(L"hello", t->term());
    }
}

BOOST_AUTO_TEST_CASE(testGrow)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    StringStream buf;
    buf << L"ab";
    for (int32_t i = 0; i < 20; ++i)
    {
        String content = buf.str();
        t->setTermBuffer(content);
        BOOST_CHECK_EQUAL(content.length(), t->termLength());
        BOOST_CHECK_EQUAL(content, t->term());
        buf << content;
    }
    BOOST_CHECK_EQUAL(1048576, t->termLength());
    BOOST_CHECK_EQUAL(1179654, t->termBuffer().size());
    
    // Test for slow growth to a long term
    t = newLucene<TermAttribute>();
    buf.str(L"");
    buf << L"a";
    for (int32_t i = 0; i < 20000; ++i)
    {
        String content = buf.str();
        t->setTermBuffer(content);
        BOOST_CHECK_EQUAL(content.length(), t->termLength());
        BOOST_CHECK_EQUAL(content, t->term());
        buf << L"a";
    }
    BOOST_CHECK_EQUAL(20000, t->termLength());
    BOOST_CHECK_EQUAL(20167, t->termBuffer().size());
}

BOOST_AUTO_TEST_CASE(testToString)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"aloha");
    BOOST_CHECK_EQUAL(L"term=aloha", t->toString());

    t->setTermBuffer(L"hi there");
    BOOST_CHECK_EQUAL(L"term=hi there", t->toString());
}

BOOST_AUTO_TEST_CASE(testMixedStringArray)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    BOOST_CHECK_EQUAL(t->termLength(), 5);
    BOOST_CHECK_EQUAL(t->term(), L"hello");
    t->setTermBuffer(L"hello2");
    BOOST_CHECK_EQUAL(t->termLength(), 6);
    BOOST_CHECK_EQUAL(t->term(), L"hello2");
    
    CharArray test = CharArray::newInstance(6);
    test[0] = L'h';
    test[1] = L'e';
    test[2] = L'l';
    test[3] = L'l';
    test[4] = L'o';
    test[5] = L'3';
    
    t->setTermBuffer(test.get(), 0, 6);
    BOOST_CHECK_EQUAL(t->term(), L"hello3");

    // Make sure if we get the buffer and change a character that term() reflects the change
    CharArray buffer = t->termBuffer();
    buffer[1] = L'o';
    BOOST_CHECK_EQUAL(t->term(), L"hollo3");
}

BOOST_AUTO_TEST_CASE(testClone)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    TermAttributePtr clone = boost::dynamic_pointer_cast<TermAttribute>(checkCloneIsEqual(t));
    BOOST_CHECK_EQUAL(t->term(), clone->term());
    BOOST_CHECK(buf != clone->termBuffer());
}

BOOST_AUTO_TEST_CASE(testEquals)
{
    TermAttributePtr t1a = newLucene<TermAttribute>();
    t1a->setTermBuffer(L"hello");
    TermAttributePtr t1b = newLucene<TermAttribute>();
    t1b->setTermBuffer(L"hello");
    TermAttributePtr t2 = newLucene<TermAttribute>();
    t2->setTermBuffer(L"hello2");
    BOOST_CHECK(t1a->equals(t1b));
    BOOST_CHECK(!t1a->equals(t2));
    BOOST_CHECK(!t2->equals(t1b));
}

BOOST_AUTO_TEST_CASE(testCopyTo)
{
    TermAttributePtr t = newLucene<TermAttribute>();
    TermAttributePtr copy = boost::dynamic_pointer_cast<TermAttribute>(checkCopyIsEqual<TermAttribute>(t));
    BOOST_CHECK_EQUAL(L"", t->term());
    BOOST_CHECK_EQUAL(L"", copy->term());
    
    t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    copy = boost::dynamic_pointer_cast<TermAttribute>(checkCopyIsEqual<TermAttribute>(t));
    BOOST_CHECK_EQUAL(t->term(), copy->term());
    BOOST_CHECK(buf != copy->termBuffer());
}

BOOST_AUTO_TEST_SUITE_END()
