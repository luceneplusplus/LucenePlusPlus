/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "SimpleLRUCache.h"
#include "Term.h"

using namespace Lucene;

typedef SimpleLRUCache<int32_t, String> TestLRUSimpleCache;
typedef SimpleLRUCache< TermPtr, int32_t, luceneHash<TermPtr>, luceneEquals<TermPtr> > TestLRUTermCache;

BOOST_FIXTURE_TEST_SUITE(SimpleLRUCacheTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testCachePut)
{
    TestLRUSimpleCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    testCache.put(6, L"test 6"); // this should pop off "1" because size = 5
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    int32_t expectedKey = 6;
    
    // lru = 6, 5, 4, 3, 2
    for (TestLRUSimpleCache::const_iterator cache = testCache.begin(); cache != testCache.end(); ++cache)
        BOOST_CHECK_EQUAL(cache->first, expectedKey--);
}

BOOST_AUTO_TEST_CASE(testCacheGet)
{
    TestLRUSimpleCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    BOOST_CHECK_EQUAL(testCache.get(2), L"test 2");
    BOOST_CHECK_EQUAL(testCache.get(3), L"test 3");
}

BOOST_AUTO_TEST_CASE(testCacheExists)
{
    TestLRUSimpleCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    BOOST_CHECK(testCache.contains(1));
    BOOST_CHECK(!testCache.contains(7));
}

BOOST_AUTO_TEST_CASE(testCachePutGet)
{
    TestLRUSimpleCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    BOOST_CHECK_EQUAL(testCache.get(2), L"test 2");
    BOOST_CHECK_EQUAL(testCache.get(3), L"test 3");
    
    testCache.put(6, L"test 6");
    testCache.put(7, L"test 7");
    testCache.put(8, L"test 8");
    
    Collection<int32_t> expectedLRU = Collection<int32_t>::newInstance();
    for (TestLRUSimpleCache::const_iterator cache = testCache.begin(); cache != testCache.end(); ++cache)
        expectedLRU.add(cache->first);
    
    BOOST_CHECK_EQUAL(expectedLRU.size(), 5);
    
    // lru = 8, 7, 6, 3, 2
    BOOST_CHECK_EQUAL(expectedLRU[0], 8);
    BOOST_CHECK_EQUAL(expectedLRU[1], 7);
    BOOST_CHECK_EQUAL(expectedLRU[2], 6);
    BOOST_CHECK_EQUAL(expectedLRU[3], 3);
    BOOST_CHECK_EQUAL(expectedLRU[4], 2);
}

BOOST_AUTO_TEST_CASE(testRandomAccess)
{
    const int32_t n = 100;
    TestLRUSimpleCache cache(n);
    String value = L"test";
    
    for (int32_t i = 0; i < n; ++i)
        cache.put(i, value);

    // access every 2nd item in cache
    for (int32_t i = 0; i < n; i += 2)
        BOOST_CHECK_NE(cache.get(i), L"");

    // add n/2 elements to cache, the ones that weren't touched in the previous loop should now be thrown away
    for (int32_t i = n; i < n + (n / 2); ++i)
        cache.put(i, value);
    
    // access every 4th item in cache
    for (int32_t i = 0; i < n; i += 4)
        BOOST_CHECK_NE(cache.get(i), L"");
    
    // add 3/4n elements to cache, the ones that weren't touched in the previous loops should now be thrown away
    for (int32_t i = n; i < n + (n * 3 / 4); ++i)
        cache.put(i, value);
    
    // access every 4th item in cache
    for (int32_t i = 0; i < n; i += 4)
        BOOST_CHECK_NE(cache.get(i), L"");
}

BOOST_AUTO_TEST_CASE(testTermCache)
{
    TestLRUTermCache testCache(5);
    
    testCache.put(newLucene<Term>(L"field1", L"text1"), 1);
    testCache.put(newLucene<Term>(L"field2", L"text2"), 2);
    testCache.put(newLucene<Term>(L"field3", L"text3"), 3);
    testCache.put(newLucene<Term>(L"field4", L"text4"), 4);
    testCache.put(newLucene<Term>(L"field5", L"text5"), 5);
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    BOOST_CHECK_EQUAL(testCache.get(newLucene<Term>(L"field2", L"text2")), 2);
    BOOST_CHECK_EQUAL(testCache.get(newLucene<Term>(L"field3", L"text3")), 3);
    
    testCache.put(newLucene<Term>(L"field6", L"text6"), 6);
    testCache.put(newLucene<Term>(L"field7", L"text7"), 7);
    testCache.put(newLucene<Term>(L"field8", L"text8"), 8);
    
    Collection<TermPtr> expectedLRU = Collection<TermPtr>::newInstance();
    for (TestLRUTermCache::const_iterator cache = testCache.begin(); cache != testCache.end(); ++cache)
        expectedLRU.add(cache->first);
    
    BOOST_CHECK_EQUAL(expectedLRU.size(), 5);
    
    // lru = field8, field7, field6, field3, field2
    BOOST_CHECK(expectedLRU[0]->equals(newLucene<Term>(L"field8", L"text8")));
    BOOST_CHECK(expectedLRU[1]->equals(newLucene<Term>(L"field7", L"text7")));
    BOOST_CHECK(expectedLRU[2]->equals(newLucene<Term>(L"field6", L"text6")));
    BOOST_CHECK(expectedLRU[3]->equals(newLucene<Term>(L"field3", L"text3")));
    BOOST_CHECK(expectedLRU[4]->equals(newLucene<Term>(L"field2", L"text2")));
}

BOOST_AUTO_TEST_SUITE_END()
