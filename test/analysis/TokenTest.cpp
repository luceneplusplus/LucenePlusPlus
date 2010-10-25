/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Token.h"
#include "Payload.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(TokenTest, LuceneTestFixture)

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

BOOST_AUTO_TEST_CASE(testCtor)
{
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    BOOST_CHECK_EQUAL(L"hello", t->term());
    BOOST_CHECK_EQUAL(L"word", t->type());
    BOOST_CHECK_EQUAL(0, t->getFlags());

    t = newLucene<Token>(6, 22);
    t->setTermBuffer(L"hello");
    BOOST_CHECK_EQUAL(L"hello", t->term());
    BOOST_CHECK_EQUAL(L"(hello,6,22)", t->toString());
    BOOST_CHECK_EQUAL(L"word", t->type());
    BOOST_CHECK_EQUAL(0, t->getFlags());

    t = newLucene<Token>(6, 22, 7);
    t->setTermBuffer(L"hello");
    BOOST_CHECK_EQUAL(L"hello", t->term());
    BOOST_CHECK_EQUAL(L"(hello,6,22)", t->toString());
    BOOST_CHECK_EQUAL(7, t->getFlags());

    t = newLucene<Token>(6, 22, L"junk");
    t->setTermBuffer(L"hello");
    BOOST_CHECK_EQUAL(L"hello", t->term());
    BOOST_CHECK_EQUAL(L"(hello,6,22,type=junk)", t->toString());
    BOOST_CHECK_EQUAL(0, t->getFlags());
}

BOOST_AUTO_TEST_CASE(testResize)
{
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    for (int32_t i = 0; i < 2000; ++i)
    {
        t->resizeTermBuffer(i);
        BOOST_CHECK(i <= t->termBuffer().length());
        BOOST_CHECK_EQUAL(L"hello", t->term());
    }
}

BOOST_AUTO_TEST_CASE(testGrow)
{
    TokenPtr t = newLucene<Token>();
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
    BOOST_CHECK_EQUAL(1179654, t->termBuffer().length());
    
    // Test for slow growth to a long term
    t = newLucene<Token>();
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
    BOOST_CHECK_EQUAL(20167, t->termBuffer().length());
}

BOOST_AUTO_TEST_CASE(testToString)
{
    TokenPtr t = newLucene<Token>(L"", 0, 5);
    t->setTermBuffer(L"aloha");
    BOOST_CHECK_EQUAL(L"(aloha,0,5)", t->toString());

    t->setTermBuffer(L"hi there");
    BOOST_CHECK_EQUAL(L"(hi there,0,5)", t->toString());
}

BOOST_AUTO_TEST_CASE(testTermBufferEquals)
{
    TokenPtr t1a = newLucene<Token>();
    t1a->setTermBuffer(L"hello");
    TokenPtr t1b = newLucene<Token>();
    t1b->setTermBuffer(L"hello");
    TokenPtr t2 = newLucene<Token>();
    t2->setTermBuffer(L"hello2");
    BOOST_CHECK(t1a->equals(t1b));
    BOOST_CHECK(!t1a->equals(t2));
    BOOST_CHECK(!t2->equals(t1b));
}

BOOST_AUTO_TEST_CASE(testMixedStringArray)
{
    TokenPtr t = newLucene<Token>();
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
    
    CharArray buffer = t->termBuffer();
    buffer[1] = L'o';
    BOOST_CHECK_EQUAL(t->term(), L"hollo3");
}

BOOST_AUTO_TEST_CASE(testClone)
{
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    TokenPtr clone = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    BOOST_CHECK_EQUAL(t->term(), clone->term());
    BOOST_CHECK(buf != clone->termBuffer());

    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;

    PayloadPtr pl = newLucene<Payload>(payload);
    t->setPayload(pl);
    clone = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    BOOST_CHECK(pl->equals(clone->getPayload()));
    BOOST_CHECK_NE(pl, clone->getPayload());
}

BOOST_AUTO_TEST_CASE(testCopyTo)
{
    TokenPtr t = newLucene<Token>();
    TokenPtr copy = boost::dynamic_pointer_cast<Token>(checkCopyIsEqual<Token>(t));
    BOOST_CHECK_EQUAL(L"", t->term());
    BOOST_CHECK_EQUAL(L"", copy->term());
    
    t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    copy = boost::dynamic_pointer_cast<Token>(checkCopyIsEqual<Token>(t));
    BOOST_CHECK_EQUAL(t->term(), copy->term());
    BOOST_CHECK(buf != copy->termBuffer());
    
    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;

    PayloadPtr pl = newLucene<Payload>(payload);
    t->setPayload(pl);
    copy = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    BOOST_CHECK(pl->equals(copy->getPayload()));
    BOOST_CHECK_NE(pl, copy->getPayload());
}

BOOST_AUTO_TEST_SUITE_END()
