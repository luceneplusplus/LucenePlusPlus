/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "TermAttribute.h"

using namespace Lucene;

typedef LuceneTestFixture TermAttributeTest;

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

TEST_F(TermAttributeTest, testResize) {
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    for (int32_t i = 0; i < 2000; ++i) {
        t->resizeTermBuffer(i);
        EXPECT_TRUE(i <= t->termBuffer().size());
        EXPECT_EQ(L"hello", t->term());
    }
}

TEST_F(TermAttributeTest, testGrow) {
    TermAttributePtr t = newLucene<TermAttribute>();
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
    t = newLucene<TermAttribute>();
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

TEST_F(TermAttributeTest, testToString) {
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"aloha");
    EXPECT_EQ(L"term=aloha", t->toString());

    t->setTermBuffer(L"hi there");
    EXPECT_EQ(L"term=hi there", t->toString());
}

TEST_F(TermAttributeTest, testMixedStringArray) {
    TermAttributePtr t = newLucene<TermAttribute>();
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

    // Make sure if we get the buffer and change a character that term() reflects the change
    CharArray buffer = t->termBuffer();
    buffer[1] = L'o';
    EXPECT_EQ(t->term(), L"hollo3");
}

TEST_F(TermAttributeTest, testClone) {
    TermAttributePtr t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    TermAttributePtr clone = boost::dynamic_pointer_cast<TermAttribute>(checkCloneIsEqual(t));
    EXPECT_EQ(t->term(), clone->term());
    EXPECT_TRUE(buf != clone->termBuffer());
}

TEST_F(TermAttributeTest, testEquals) {
    TermAttributePtr t1a = newLucene<TermAttribute>();
    t1a->setTermBuffer(L"hello");
    TermAttributePtr t1b = newLucene<TermAttribute>();
    t1b->setTermBuffer(L"hello");
    TermAttributePtr t2 = newLucene<TermAttribute>();
    t2->setTermBuffer(L"hello2");
    EXPECT_TRUE(t1a->equals(t1b));
    EXPECT_TRUE(!t1a->equals(t2));
    EXPECT_TRUE(!t2->equals(t1b));
}

TEST_F(TermAttributeTest, testCopyTo) {
    TermAttributePtr t = newLucene<TermAttribute>();
    TermAttributePtr copy = boost::dynamic_pointer_cast<TermAttribute>(checkCopyIsEqual<TermAttribute>(t));
    EXPECT_EQ(L"", t->term());
    EXPECT_EQ(L"", copy->term());

    t = newLucene<TermAttribute>();
    t->setTermBuffer(L"hello");
    CharArray buf = t->termBuffer();
    copy = boost::dynamic_pointer_cast<TermAttribute>(checkCopyIsEqual<TermAttribute>(t));
    EXPECT_EQ(t->term(), copy->term());
    EXPECT_TRUE(buf != copy->termBuffer());
}
