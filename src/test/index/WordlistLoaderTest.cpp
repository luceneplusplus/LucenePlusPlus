/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "WordlistLoader.h"
#include "StringReader.h"
#include "BufferedReader.h"

using namespace Lucene;

typedef LuceneTestFixture WordlistLoaderTest;

static void checkSet(HashSet<String> wordset) {
    EXPECT_EQ(3, wordset.size());
    EXPECT_TRUE(wordset.contains(L"ONE")); // case is not modified
    EXPECT_TRUE(!wordset.contains(L"one")); // case is not modified
    EXPECT_TRUE(wordset.contains(L"two")); // surrounding whitespace is removed
    EXPECT_TRUE(wordset.contains(L"three"));
    EXPECT_TRUE(!wordset.contains(L"four"));
}

TEST_F(WordlistLoaderTest, testWordlistLoading) {
    String s = L"ONE\n  two \nthree";
    HashSet<String> wordSet1 = WordlistLoader::getWordSet(newLucene<StringReader>(s));
    checkSet(wordSet1);
    HashSet<String> wordSet2 = WordlistLoader::getWordSet(newLucene<BufferedReader>(newLucene<StringReader>(s)));
    checkSet(wordSet2);
}

TEST_F(WordlistLoaderTest, testComments) {
    String s = L"ONE\n  two \nthree\n#comment";
    HashSet<String> wordSet1 = WordlistLoader::getWordSet(newLucene<StringReader>(s), L"#");
    checkSet(wordSet1);
    EXPECT_TRUE(!wordSet1.contains(L"#comment"));
    EXPECT_TRUE(!wordSet1.contains(L"comment"));
}
