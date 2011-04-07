/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <float.h>
#include "LuceneTestFixture.h"
#include "IndexReader.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "MultiReader.h"
#include "FieldCache.h"
#include "FieldCacheSanityChecker.h"
#include "Document.h"
#include "Field.h"

using namespace Lucene;

class FieldCacheSanityCheckerTestFixture : public LuceneTestFixture
{
public:
    FieldCacheSanityCheckerTestFixture()
    {
        RAMDirectoryPtr dirA = newLucene<RAMDirectory>();
        RAMDirectoryPtr dirB = newLucene<RAMDirectory>();

        IndexWriterPtr wA = newLucene<IndexWriter>(dirA, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        IndexWriterPtr wB = newLucene<IndexWriter>(dirB, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        int64_t theLong = LLONG_MAX;
        double theDouble = DBL_MAX;
        uint8_t theByte = UCHAR_MAX;
        int32_t theInt = INT_MAX;
        for (int32_t i = 0; i < NUM_DOCS; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"theLong", StringUtils::toString(theLong--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theDouble", StringUtils::toString(theDouble--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theByte", StringUtils::toString(theByte--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theInt", StringUtils::toString(theInt--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            if (i % 3 == 0)
                wA->addDocument(doc);
            else
                wB->addDocument(doc);
        }
        wA->close();
        wB->close();
        readerA = IndexReader::open(dirA, true);
        readerB = IndexReader::open(dirB, true);
        readerX = newLucene<MultiReader>(newCollection<IndexReaderPtr>(readerA, readerB));
    }
    
    virtual ~FieldCacheSanityCheckerTestFixture()
    {
        readerA->close();
        readerB->close();
        readerX->close();
    }

protected:
    IndexReaderPtr readerA;
    IndexReaderPtr readerB;
    IndexReaderPtr readerX;

    static const int32_t NUM_DOCS;
};

const int32_t FieldCacheSanityCheckerTestFixture::NUM_DOCS = 1000;

BOOST_FIXTURE_TEST_SUITE(FieldCacheSanityCheckerTest, FieldCacheSanityCheckerTestFixture)

BOOST_AUTO_TEST_CASE(testSanity)
{
    FieldCachePtr cache = FieldCache::DEFAULT();
    cache->purgeAllCaches();

    Collection<double> doubles = cache->getDoubles(readerA, L"theDouble");
    doubles = cache->getDoubles(readerA, L"theDouble", FieldCache::DEFAULT_DOUBLE_PARSER());
    doubles = cache->getDoubles(readerB, L"theDouble", FieldCache::DEFAULT_DOUBLE_PARSER());

    Collection<int32_t> ints = cache->getInts(readerX, L"theInt");
    ints = cache->getInts(readerX, L"theInt", FieldCache::DEFAULT_INT_PARSER());

    Collection<InsanityPtr> insanity = FieldCacheSanityChecker::checkSanity(cache->getCacheEntries());
    
    BOOST_CHECK_EQUAL(0, insanity.size());
    cache->purgeAllCaches();
}

BOOST_AUTO_TEST_CASE(testInsanity1)
{
    FieldCachePtr cache = FieldCache::DEFAULT();
    cache->purgeAllCaches();

    Collection<int32_t> ints = cache->getInts(readerX, L"theInt", FieldCache::DEFAULT_INT_PARSER());
    Collection<String> strings = cache->getStrings(readerX, L"theInt");

    // this one is ok
    Collection<uint8_t> bytes = cache->getBytes(readerX, L"theByte");

    Collection<InsanityPtr> insanity = FieldCacheSanityChecker::checkSanity(cache->getCacheEntries());

    BOOST_CHECK_EQUAL(1, insanity.size());
    BOOST_CHECK_EQUAL(FieldCacheSanityChecker::VALUEMISMATCH, insanity[0]->getType());
    BOOST_CHECK_EQUAL(2, insanity[0]->getCacheEntries().size());

    // we expect bad things, don't let tearDown complain about them
    cache->purgeAllCaches();
}

BOOST_AUTO_TEST_CASE(testInsanity2)
{
    FieldCachePtr cache = FieldCache::DEFAULT();
    cache->purgeAllCaches();

    Collection<String> strings = cache->getStrings(readerA, L"theString");
    strings = cache->getStrings(readerB, L"theString");
    strings = cache->getStrings(readerX, L"theString");
    
    // this one is ok
    Collection<uint8_t> bytes = cache->getBytes(readerX, L"theByte");

    Collection<InsanityPtr> insanity = FieldCacheSanityChecker::checkSanity(cache->getCacheEntries());

    BOOST_CHECK_EQUAL(1, insanity.size());
    BOOST_CHECK_EQUAL(FieldCacheSanityChecker::SUBREADER, insanity[0]->getType());
    BOOST_CHECK_EQUAL(3, insanity[0]->getCacheEntries().size());

    // we expect bad things, don't let tearDown complain about them
    cache->purgeAllCaches();
}

BOOST_AUTO_TEST_SUITE_END()
