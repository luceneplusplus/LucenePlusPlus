/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <float.h>
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "FieldCache.h"

using namespace Lucene;

class FieldCacheTest : public LuceneTestFixture {
public:
    FieldCacheTest() {
        RAMDirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer= newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        int64_t theLong = LLONG_MAX;
        double theDouble = DBL_MAX;
        uint8_t theByte = UCHAR_MAX;
        int32_t theInt = INT_MAX;
        for (int32_t i = 0; i < NUM_DOCS; ++i) {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(L"theLong", StringUtils::toString(theLong--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theDouble", StringUtils::toString(theDouble--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theByte", StringUtils::toString(theByte--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"theInt", StringUtils::toString(theInt--), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        writer->close();
        reader = IndexReader::open(directory, true);
    }

    virtual ~FieldCacheTest() {
    }

protected:
    IndexReaderPtr reader;

public:
    static const int32_t NUM_DOCS;
};

const int32_t FieldCacheTest::NUM_DOCS = 1000;

TEST_F(FieldCacheTest, testFieldCache) {
    FieldCachePtr cache = FieldCache::DEFAULT();
    Collection<double> doubles = cache->getDoubles(reader, L"theDouble");
    EXPECT_EQ(doubles.hashCode(), cache->getDoubles(reader, L"theDouble").hashCode());
    EXPECT_EQ(doubles.hashCode(), cache->getDoubles(reader, L"theDouble", FieldCache::DEFAULT_DOUBLE_PARSER()).hashCode());
    EXPECT_EQ(doubles.size(), NUM_DOCS);
    for (int32_t i = 0; i < doubles.size(); ++i) {
        StringStream first;
        first.precision(5);
        first << doubles[i];
        StringStream second;
        second.precision(5);
        second << (DBL_MAX - i);
        EXPECT_EQ(first.str(), second.str());
    }

    Collection<int64_t> longs = cache->getLongs(reader, L"theLong");
    EXPECT_EQ(longs.hashCode(), cache->getLongs(reader, L"theLong").hashCode());
    EXPECT_EQ(longs.hashCode(), cache->getLongs(reader, L"theLong", FieldCache::DEFAULT_LONG_PARSER()).hashCode());
    EXPECT_EQ(longs.size(), NUM_DOCS);
    for (int32_t i = 0; i < longs.size(); ++i) {
        EXPECT_EQ(longs[i], (LLONG_MAX - i));
    }

    Collection<uint8_t> bytes = cache->getBytes(reader, L"theByte");
    EXPECT_EQ(bytes.hashCode(), cache->getBytes(reader, L"theByte").hashCode());
    EXPECT_EQ(bytes.hashCode(), cache->getBytes(reader, L"theByte", FieldCache::DEFAULT_BYTE_PARSER()).hashCode());
    EXPECT_EQ(bytes.size(), NUM_DOCS);
    for (int32_t i = 0; i < bytes.size(); ++i) {
        EXPECT_EQ(bytes[i], (uint8_t)(UCHAR_MAX - i));
    }

    Collection<int32_t> ints = cache->getInts(reader, L"theInt");
    EXPECT_EQ(ints.hashCode(), cache->getInts(reader, L"theInt").hashCode());
    EXPECT_EQ(ints.hashCode(), cache->getInts(reader, L"theInt", FieldCache::DEFAULT_INT_PARSER()).hashCode());
    EXPECT_EQ(ints.size(), NUM_DOCS);
    for (int32_t i = 0; i < ints.size(); ++i) {
        EXPECT_EQ(ints[i], (INT_MAX - i));
    }
}
