/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "SimpleLRUCache.h"

using namespace Lucene;

typedef SimpleLRUCache<int32_t, String> TestLRUCache;

BOOST_FIXTURE_TEST_SUITE(SimpleLRUCacheTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testCachePut)
{
    TestLRUCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    testCache.put(6, L"test 6"); // should pop off "1"
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    int32_t expectedKey = 2;
    
    // lru = 2, 3, 4, 5, 6
    for (TestLRUCache::iterator cache = testCache.begin(); cache != testCache.end(); ++cache)
        BOOST_CHECK_EQUAL(cache->first, expectedKey++);
}

BOOST_AUTO_TEST_CASE(testCacheGet)
{
    TestLRUCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    String val;
    BOOST_CHECK(testCache.get(2, val));
    BOOST_CHECK_EQUAL(val, L"test 2");
    
    BOOST_CHECK(testCache.get(3, val));
    BOOST_CHECK_EQUAL(val, L"test 3");
}

BOOST_AUTO_TEST_CASE(testCacheGetNotExists)
{
    TestLRUCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    String val;
    BOOST_CHECK(!testCache.get(7, val)); // doesn't exist
}

BOOST_AUTO_TEST_CASE(testCachePutGet)
{
    TestLRUCache testCache(5);
    
    testCache.put(1, L"test 1");
    testCache.put(2, L"test 2");
    testCache.put(3, L"test 3");
    testCache.put(4, L"test 4");
    testCache.put(5, L"test 5");
    
    BOOST_CHECK_EQUAL(testCache.size(), 5);
    
    String val;
    BOOST_CHECK(testCache.get(2, val));
    BOOST_CHECK(testCache.get(3, val));
    
    testCache.put(6, L"test 6");
    testCache.put(7, L"test 7");
    testCache.put(8, L"test 8");
    
    std::vector<int32_t> expectedLRU;
    for (TestLRUCache::iterator cache = testCache.begin(); cache != testCache.end(); ++cache)
        expectedLRU.push_back(cache->first);
    
    BOOST_CHECK_EQUAL(expectedLRU.size(), 5);
    
    // lru = 5, 2, 3, 6, 7
    BOOST_CHECK_EQUAL(expectedLRU[0], 2);
    BOOST_CHECK_EQUAL(expectedLRU[1], 3);
    BOOST_CHECK_EQUAL(expectedLRU[2], 6);
    BOOST_CHECK_EQUAL(expectedLRU[3], 7);
    BOOST_CHECK_EQUAL(expectedLRU[4], 8);
}

BOOST_AUTO_TEST_CASE(testRandomAccess)
{
    const int32_t n = 100;
    TestLRUCache cache(n);
    String value = L"test";
    
    for (int32_t i = 0; i < n; ++i)
        cache.put(i, value);

    // access every 2nd item in cache
    for (int32_t i = 0; i < n; i += 2)
        BOOST_CHECK(cache.get(i, value));

    // add n/2 elements to cache, the ones that weren't touched in the previous loop should now be thrown away
    for (int32_t i = n; i < n + (n / 2); ++i)
        cache.put(i, value);
    
    // access every 4th item in cache
    for (int32_t i = 0; i < n; i += 4)
        BOOST_CHECK(cache.get(i, value));
    
    // add 3/4n elements to cache, the ones that weren't touched in the previous loops should now be thrown away
    for (int32_t i = n; i < n + (n * 3 / 4); ++i)
        cache.put(i, value);
    
    // access every 4th item in cache
    for (int32_t i = 0; i < n; i += 4)
        BOOST_CHECK(cache.get(i, value));
}

BOOST_AUTO_TEST_SUITE_END()
