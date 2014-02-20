/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Token.h"
#include "Payload.h"

using namespace Lucene;

typedef LuceneTestFixture TokenTest;

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

TEST_F(TokenTest, testCtor) {
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    EXPECT_EQ(L"hello", t->term());
    EXPECT_EQ(L"word", t->type());
    EXPECT_EQ(0, t->getFlags());

    t = newLucene<Token>(6, 22);
    t->setTermBuffer(L"hello");
    EXPECT_EQ(L"hello", t->term());
    EXPECT_EQ(L"(hello,6,22)", t->toString());
    EXPECT_EQ(L"word", t->type());
    EXPECT_EQ(0, t->getFlags());

    t = newLucene<Token>(6, 22, 7);
    t->setTermBuffer(L"hello");
    EXPECT_EQ(L"hello", t->term());
    EXPECT_EQ(L"(hello,6,22)", t->toString());
    EXPECT_EQ(7, t->getFlags());

    t = newLucene<Token>(6, 22, L"junk");
    t->setTermBuffer(L"hello");
    EXPECT_EQ(L"hello", t->term());
    EXPECT_EQ(L"(hello,6,22,type=junk)", t->toString());
    EXPECT_EQ(0, t->getFlags());
}

TEST_F(TokenTest, testResize) {
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    for (int32_t i = 0; i < 2000; ++i) {
        t->resizeTermBuffer(i);
        EXPECT_TRUE(i <= t->termBuffer().size());
        EXPECT_EQ(L"hello", t->term());
    }
}

TEST_F(TokenTest, testGrow) {
    TokenPtr t = newLucene<Token>();
    StringStream buf;
    buf << L"ab";
    for (int32_t i = 0; i < 20; ++i) {
        String content = buf.str();
        t->setTermBuffer(content);
        EXPECT_EQ(content.length(), t->termLength());
        EXPECT_EQ(content, t->term());
        buf << content;
    }
    EXPECT_EQ(1048576, t->termLength());
    EXPECT_EQ(1179654, t->termBuffer().size());

    // Test for slow growth to a long term
    t = newLucene<Token>();
    buf.str(L"");
    buf << L"a";
    for (int32_t i = 0; i < 20000; ++i) {
        String content = buf.str();
        t->setTermBuffer(content);
        EXPECT_EQ(content.length(), t->termLength());
        EXPECT_EQ(content, t->term());
        buf << L"a";
    }
    EXPECT_EQ(20000, t->termLength());
    EXPECT_EQ(20167, t->termBuffer().size());
}

TEST_F(TokenTest, testToString) {
    TokenPtr t = newLucene<Token>(L"", 0, 5);
    t->setTermBuffer(L"aloha");
    EXPECT_EQ(L"(aloha,0,5)", t->toString());

    t->setTermBuffer(L"hi there");
    EXPECT_EQ(L"(hi there,0,5)", t->toString());
}

TEST_F(TokenTest, testTermBufferEquals) {
    TokenPtr t1a = newLucene<Token>();
    t1a->setTermBuffer(L"hello");
    TokenPtr t1b = newLucene<Token>();
    t1b->setTermBuffer(L"hello");
    TokenPtr t2 = newLucene<Token>();
    t2->setTermBuffer(L"hello2");
    EXPECT_TRUE(t1a->equals(t1b));
    EXPECT_TRUE(!t1a->equals(t2));
    EXPECT_TRUE(!t2->equals(t1b));
}

TEST_F(TokenTest, testMixedStringArray) {
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    EXPECT_EQ(t->termLength(), 5);
    EXPECT_EQ(t->term(), L"hello");
    t->setTermBuffer(L"hello2");
    EXPECT_EQ(t->termLength(), 6);
    EXPECT_EQ(t->term(), L"hello2");

    CharArray test = CharArray::newInstance(6);
    test[0] = L'h';
    test[1] = L'e';
    test[2] = L'l';
    test[3] = L'l';
    test[4] = L'o';
    test[5] = L'3';

    t->setTermBuffer(test.get(), 0, 6);
    EXPECT_EQ(t->term(), L"hello3");

    CharArray buffer = t->termBuffer();
    buffer[1] = L'o';
    EXPECT_EQ(t->term(), L"hollo3");
}

TEST_F(TokenTest, testClone) {
    TokenPtr t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    TokenPtr clone = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    EXPECT_EQ(t->term(), clone->term());
    EXPECT_TRUE(buf != clone->termBuffer());

    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;

    PayloadPtr pl = newLucene<Payload>(payload);
    t->setPayload(pl);
    clone = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    EXPECT_TRUE(pl->equals(clone->getPayload()));
    EXPECT_NE(pl, clone->getPayload());
}

TEST_F(TokenTest, testCopyTo) {
    TokenPtr t = newLucene<Token>();
    TokenPtr copy = boost::dynamic_pointer_cast<Token>(checkCopyIsEqual<Token>(t));
    EXPECT_EQ(L"", t->term());
    EXPECT_EQ(L"", copy->term());

    t = newLucene<Token>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    copy = boost::dynamic_pointer_cast<Token>(checkCopyIsEqual<Token>(t));
    EXPECT_EQ(t->term(), copy->term());
    EXPECT_TRUE(buf != copy->termBuffer());

    ByteArray payload = ByteArray::newInstance(4);
    payload[0] = 1;
    payload[1] = 2;
    payload[2] = 3;
    payload[3] = 4;

    PayloadPtr pl = newLucene<Payload>(payload);
    t->setPayload(pl);
    copy = boost::dynamic_pointer_cast<Token>(checkCloneIsEqual(t));
    EXPECT_TRUE(pl->equals(copy->getPayload()));
    EXPECT_NE(pl, copy->getPayload());
}
