/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TermQuery.h"
#include "Term.h"
#include "Sort.h"
#include "SortField.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TopFieldDocs.h"
#include "Random.h"
#include "MatchAllDocsQuery.h"
#include "FieldCache.h"
#include "FieldCacheSanityChecker.h"
#include "FieldComparatorSource.h"
#include "FieldComparator.h"
#include "MultiSearcher.h"
#include "ParallelMultiSearcher.h"
#include "Filter.h"
#include "BitSet.h"
#include "DocIdBitSet.h"
#include "IndexReader.h"
#include "TopFieldCollector.h"
#include "BooleanQuery.h"
#include "MiscUtils.h"

using namespace Lucene;

class SortTest : public LuceneTestFixture {
public:
    SortTest() {
        r = newLucene<Random>();

        // document data:
        // the tracer field is used to determine which document was hit the contents field is used to search and sort by relevance
        // the int field to sort by int the double field to sort by double the string field to sort by string the i18n field
        // includes accented characters for testing locale-specific sorting
        data = Collection< Collection<String> >::newInstance(14);
        // tracer contents int double string custom i18n long byte encoding

        data[0] = newCollection<String>(L"A", L"x a", L"5", L"4.0", L"c", L"A-3", L"p\u00EAche", L"10", L"177", L"J");
        data[1] = newCollection<String>(L"B", L"y a", L"5", L"3.4028235E38", L"i", L"B-10", L"HAT", L"1000000000", L"52", L"I");
        data[2] = newCollection<String>(L"C", L"x a b c", L"2147483647", L"1.0", L"j", L"A-2", L"p\u00E9ch\u00E9", L"99999999", L"66", L"H");
        data[3] = newCollection<String>(L"D", L"y a b c", L"-1", L"0.0", L"a", L"C-0", L"HUT", StringUtils::toString(LLONG_MAX), L"0", L"G");
        data[4] = newCollection<String>(L"E", L"x a b c d", L"5", L"2.0", L"h", L"B-8", L"peach", StringUtils::toString(LLONG_MIN), StringUtils::toString(UCHAR_MAX), L"F");
        data[5] = newCollection<String>(L"F", L"y a b c d", L"2", L"3.14159", L"g", L"B-1", L"H\u00C5T", L"-44", L"51", L"E");
        data[6] = newCollection<String>(L"G", L"x a b c d", L"3", L"-1.0", L"f", L"C-100", L"sin", L"323254543543", L"151", L"D");
        data[7] = newCollection<String>(L"H", L"y a b c d", L"0", L"1.4E-45", L"e", L"C-88", L"H\u00D8T", L"1023423423005", L"1", L"C");
        data[8] = newCollection<String>(L"I", L"x a b c d e f", L"-2147483648", L"1.0e+0", L"d", L"A-10", L"s\u00EDn", L"332422459999", L"102", L"B");
        data[9] = newCollection<String>(L"J", L"y a b c d e f", L"4", L".5", L"b", L"C-7", L"HOT", L"34334543543", L"53", L"A");
        data[10] = newCollection<String>(L"W", L"g", L"1", L"", L"", L"", L"", L"", L"", L"");
        data[11] = newCollection<String>(L"X", L"g", L"1", L"0.1", L"", L"", L"", L"", L"", L"");
        data[12] = newCollection<String>(L"Y", L"g", L"1", L"0.2", L"", L"", L"", L"", L"", L"");
        data[13] = newCollection<String>(L"Z", L"f g", L"", L"", L"", L"", L"", L"", L"", L"");

        full = getFullIndex();
        searchX = getXIndex();
        searchY = getYIndex();
        queryX = newLucene<TermQuery>(newLucene<Term>(L"contents", L"x"));
        queryY = newLucene<TermQuery>(newLucene<Term>(L"contents", L"y"));
        queryA = newLucene<TermQuery>(newLucene<Term>(L"contents", L"a"));
        queryE = newLucene<TermQuery>(newLucene<Term>(L"contents", L"e"));
        queryF = newLucene<TermQuery>(newLucene<Term>(L"contents", L"f"));
        queryG = newLucene<TermQuery>(newLucene<Term>(L"contents", L"g"));
        sort = newLucene<Sort>();
    }

    virtual ~SortTest() {
    }

protected:
    static const int32_t NUM_STRINGS;

    SearcherPtr full;
    SearcherPtr searchX;
    SearcherPtr searchY;
    QueryPtr queryX;
    QueryPtr queryY;
    QueryPtr queryA;
    QueryPtr queryE;
    QueryPtr queryF;
    QueryPtr queryG;
    SortPtr sort;
    RandomPtr r;

    Collection< Collection<String> > data;

protected:
    SearcherPtr getFullIndex() {
        return getIndex(true, true);
    }

    SearcherPtr getXIndex() {
        return getIndex(true, false);
    }

    SearcherPtr getYIndex() {
        return getIndex(false, true);
    }

    SearcherPtr getEmptyIndex() {
        return getIndex(false, false);
    }

    SearcherPtr getIndex(bool even, bool odd) {
        RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(2);
        writer->setMergeFactor(1000);
        for (int32_t i = 0; i < data.size(); ++i) {
            if (((i % 2) == 0 && even) || ((i % 2) == 1 && odd)) {
                DocumentPtr doc = newLucene<Document>();
                doc->add(newLucene<Field>(L"tracer", data[i][0], Field::STORE_YES, Field::INDEX_NO));
                doc->add(newLucene<Field>(L"contents", data[i][1], Field::STORE_NO, Field::INDEX_ANALYZED));
                if (!data[i][2].empty()) {
                    doc->add(newLucene<Field>(L"int", data[i][2], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][3].empty()) {
                    doc->add(newLucene<Field>(L"double", data[i][3], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][4].empty()) {
                    doc->add(newLucene<Field>(L"string", data[i][4], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][5].empty()) {
                    doc->add(newLucene<Field>(L"custom", data[i][5], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][6].empty()) {
                    doc->add(newLucene<Field>(L"i18n", data[i][6], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][7].empty()) {
                    doc->add(newLucene<Field>(L"long", data[i][7], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][8].empty()) {
                    doc->add(newLucene<Field>(L"byte", data[i][8], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                if (!data[i][9].empty()) {
                    doc->add(newLucene<Field>(L"parser", data[i][9], Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
                }
                doc->setBoost(2); // produce some scores above 1.0
                writer->addDocument(doc);
            }
        }
        writer->close();
        IndexSearcherPtr s = newLucene<IndexSearcher>(indexStore, true);
        s->setDefaultFieldSortScoring(true, true);
        return s;
    }

    MapStringDouble getScores(Collection<ScoreDocPtr> hits, const SearcherPtr& searcher) {
        MapStringDouble scoreMap = MapStringDouble::newInstance();
        int32_t n = hits.size();
        for (int32_t i = 0; i < n; ++i) {
            DocumentPtr doc = searcher->doc(hits[i]->doc);
            Collection<String> v = doc->getValues(L"tracer");
            EXPECT_EQ(v.size(), 1);
            scoreMap.put(v[0], hits[i]->score);
        }
        return scoreMap;
    }

    void checkSameValues(MapStringDouble m1, MapStringDouble m2) {
        int32_t n = m1.size();
        int32_t m = m2.size();
        EXPECT_EQ(n, m);
        for (MapStringDouble::iterator key = m1.begin(); key != m1.end(); ++key) {
            double o1 = m1.get(key->first);
            double o2 = m2.get(key->first);
            EXPECT_NEAR(o1, o2, 1e-6);
        }
    }

    /// make sure the documents returned by the search match the expected list
    void checkMatches(const SearcherPtr& searcher, const QueryPtr& query, const SortPtr& sort, const String& expectedResult) {
        TopDocsPtr hits = searcher->search(query, FilterPtr(), expectedResult.length(), sort);
        Collection<ScoreDocPtr> result = hits->scoreDocs;
        EXPECT_EQ(hits->totalHits, expectedResult.length());
        StringStream buff;
        int32_t n = result.size();
        for (int32_t i = 0; i < n; ++i) {
            DocumentPtr doc = searcher->doc(result[i]->doc);
            Collection<String> v = doc->getValues(L"tracer");
            for (int32_t j = 0; j < v.size(); ++j) {
                buff << v[j];
            }
        }
        EXPECT_EQ(expectedResult, buff.str());
    }

    IndexSearcherPtr getFullStrings() {
        RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        writer->setMaxBufferedDocs(4);
        writer->setMergeFactor(97);
        for (int32_t i = 0; i < NUM_STRINGS; ++i) {
            DocumentPtr doc = newLucene<Document>();
            String num = getRandomCharString(getRandomNumber(2, 8), 48, 52);
            doc->add(newLucene<Field>(L"tracer", num, Field::STORE_YES, Field::INDEX_NO));
            doc->add(newLucene<Field>(L"string", num, Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            String num2 = getRandomCharString(getRandomNumber(1, 4), 48, 50);
            doc->add(newLucene<Field>(L"string2", num2, Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
            doc->add(newLucene<Field>(L"tracer2", num2, Field::STORE_YES, Field::INDEX_NO));
            doc->setBoost(2.0); // produce some scores above 1.0
            writer->setMaxBufferedDocs(getRandomNumber(2, 12));
            writer->addDocument(doc);
        }
        writer->close();
        return newLucene<IndexSearcher>(indexStore, true);
    }

    String getRandomNumberString(int32_t num, int32_t low, int32_t high) {
        StringStream buff;
        for (int32_t i = 0; i < num; ++i) {
            buff << getRandomNumber(low, high);
        }
        return buff.str();
    }

    String getRandomCharString(int32_t num) {
        return getRandomCharString(num, 48, 122);
    }

    String getRandomCharString(int32_t num, int32_t start, int32_t end) {
        static const wchar_t* alphanum = L"0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz";
        StringStream buff;
        for (int32_t i = 0; i < num; ++i) {
            buff << alphanum[getRandomNumber(start, end)];
        }
        return buff.str();
    }

    int32_t getRandomNumber(int32_t low, int32_t high) {
        return (std::abs(r->nextInt()) % (high - low)) + low;
    }

    void checkSaneFieldCaches() {
        Collection<FieldCacheEntryPtr> entries = FieldCache::DEFAULT()->getCacheEntries();
        Collection<InsanityPtr> insanity = FieldCacheSanityChecker::checkSanity(entries);
        EXPECT_EQ(0, insanity.size());
    }

    /// runs a variety of sorts useful for multisearchers
    void runMultiSorts(const SearcherPtr& multi, bool isFull) {
        sort->setSort(SortField::FIELD_DOC());
        String expected = isFull ? L"ABCDEFGHIJ" : L"ACEGIBDFHJ";
        checkMatches(multi, queryA, sort, expected);

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT)));
        expected = isFull ? L"IDHFGJABEC" : L"IDHFGJAEBC";
        checkMatches(multi, queryA, sort, expected);

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), SortField::FIELD_DOC()));
        expected = isFull ? L"IDHFGJABEC" : L"IDHFGJAEBC";
        checkMatches(multi, queryA, sort, expected);

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT)));
        expected = isFull ? L"IDHFGJABEC" : L"IDHFGJAEBC";
        checkMatches(multi, queryA, sort, expected);

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE), SortField::FIELD_DOC()));
        checkMatches(multi, queryA, sort, L"GDHJCIEFAB");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE)));
        checkMatches(multi, queryA, sort, L"GDHJCIEFAB");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING)));
        checkMatches(multi, queryA, sort, L"DJAIHGFEBC");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true)));
        expected = isFull ? L"CABEJGFHDI" : L"CAEBJGFHDI";
        checkMatches(multi, queryA, sort, expected);

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE, true)));
        checkMatches(multi, queryA, sort, L"BAFECIJHDG");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING, true)));
        checkMatches(multi, queryA, sort, L"CBEFGHIAJD");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"double", SortField::DOUBLE)));
        checkMatches(multi, queryA, sort, L"IDHFGJEABC");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE), newLucene<SortField>(L"string", SortField::STRING)));
        checkMatches(multi, queryA, sort, L"GDHJICEFAB");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT)));
        checkMatches(multi, queryF, sort, L"IZJ");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true)));
        checkMatches(multi, queryF, sort, L"JZI");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE)));
        checkMatches(multi, queryF, sort, L"ZJI");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING)));
        checkMatches(multi, queryF, sort, L"ZJI");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING, true)));
        checkMatches(multi, queryF, sort, L"IJZ");

        // up to this point, all of the searches should have "sane" FieldCache behavior, and should have reused
        // the cache in several cases
        checkSaneFieldCaches();

        // next we'll check Locale based (Collection<String>) for 'string', so purge first
        FieldCache::DEFAULT()->purgeAllCaches();

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", std::locale())));
        checkMatches(multi, queryA, sort, L"DJAIHGFEBC");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", std::locale(), true)));
        checkMatches(multi, queryA, sort, L"CBEFGHIAJD");

        sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", std::locale())));
        checkMatches(multi, queryA, sort, L"DJAIHGFEBC");

        checkSaneFieldCaches();
        FieldCache::DEFAULT()->purgeAllCaches();
    }
};

const int32_t SortTest::NUM_STRINGS = 6000;

/// test the sorts by score and document number
TEST_F(SortTest, testBuiltInSorts) {
    sort = newLucene<Sort>();
    checkMatches(full, queryX, sort, L"ACEGI");
    checkMatches(full, queryY, sort, L"BDFHJ");

    sort->setSort(SortField::FIELD_DOC());
    checkMatches(full, queryX, sort, L"ACEGI");
    checkMatches(full, queryY, sort, L"BDFHJ");
}

/// test sorts where the type of field is specified
TEST_F(SortTest, testTypedSort) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"IGAEC");
    checkMatches(full, queryY, sort, L"DHFJB");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"GCIEA");
    checkMatches(full, queryY, sort, L"DHJFB");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"long", SortField::LONG), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"EACGI");
    checkMatches(full, queryY, sort, L"FBJHD");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"byte", SortField::BYTE), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"CIGAE");
    checkMatches(full, queryY, sort, L"DHFBJ");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"AIGEC");
    checkMatches(full, queryY, sort, L"DJHFB");
}

/// Test String sorting: small queue to many matches, multi field sort, reverse sort
TEST_F(SortTest, testStringSort) {
    IndexSearcherPtr searcher = getFullStrings();
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"string2", SortField::STRING, true), SortField::FIELD_DOC()));
    Collection<ScoreDocPtr> result = searcher->search(newLucene<MatchAllDocsQuery>(), FilterPtr(), 500, sort)->scoreDocs;

    StringStream buff;
    int32_t n = result.size();
    String last;
    String lastSub;
    int32_t lastDocId = 0;
    for (int32_t x = 0; x < n; ++x) {
        DocumentPtr doc2 = searcher->doc(result[x]->doc);
        Collection<String> v = doc2->getValues(L"tracer");
        Collection<String> v2 = doc2->getValues(L"tracer2");
        for (int32_t j = 0; j < v.size(); ++j) {
            if (!last.empty()) {
                int32_t cmp = v[j].compare(last);
                if (cmp < 0) {
                    FAIL() << "first field out of order";
                }
                if (cmp == 0) { // ensure second field is in reverse order
                    cmp = v2[j].compare(lastSub);
                    if (cmp > 0) {
                        FAIL() << "second field out of order";
                    } else if (cmp == 0) { // ensure docid is in order
                        if (result[x]->doc < lastDocId) {
                            FAIL() << "docid out of order";
                        }
                    }

                }
            }
            last = v[j];
            lastSub = v2[j];
            lastDocId = result[x]->doc;
            buff << v[j] << L"(" << v2[j] << L")(" << result[x]->doc << L") ";
        }
    }
}

namespace TestCustomFieldParserSort {

class CustomIntParser : public IntParser {
public:
    virtual ~CustomIntParser() {
    }

public:
    virtual int32_t parseInt(const String& string) {
        return (string[0] - L'A') * 123456;
    }
};

class CustomDoubleParser : public DoubleParser {
public:
    virtual ~CustomDoubleParser() {
    }

public:
    virtual double parseDouble(const String& string) {
        return std::sqrt((double)string[0]);
    }
};

class CustomLongParser : public LongParser {
public:
    virtual ~CustomLongParser() {
    }

public:
    virtual int64_t parseLong(const String& string) {
        return (string[0] - L'A') * (int64_t)1234567890;
    }
};

class CustomByteParser : public ByteParser {
public:
    virtual ~CustomByteParser() {
    }

public:
    virtual uint8_t parseByte(const String& string) {
        return (uint8_t)(string[0] - L'A');
    }
};

}

/// Test sorts where the type of field is specified and a custom field parser  is used, that uses a simple char encoding.
/// The sorted string contains a character beginning from 'A' that is mapped to a numeric value using some "funny"
/// algorithm to be different for each data type.
TEST_F(SortTest, testCustomFieldParserSort) {
    // since tests explicitly uses different parsers on the same field name we explicitly check/purge the FieldCache between each assertMatch
    FieldCachePtr fc = FieldCache::DEFAULT();

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"parser", newLucene<TestCustomFieldParserSort::CustomIntParser>()), SortField::FIELD_DOC()));
    checkMatches(full, queryA, sort, L"JIHGFEDCBA");
    checkSaneFieldCaches();
    fc->purgeAllCaches();

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"parser", newLucene<TestCustomFieldParserSort::CustomDoubleParser>()), SortField::FIELD_DOC()));
    checkMatches(full, queryA, sort, L"JIHGFEDCBA");
    checkSaneFieldCaches();
    fc->purgeAllCaches();

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"parser", newLucene<TestCustomFieldParserSort::CustomLongParser>()), SortField::FIELD_DOC()));
    checkMatches(full, queryA, sort, L"JIHGFEDCBA");
    checkSaneFieldCaches();
    fc->purgeAllCaches();

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"parser", newLucene<TestCustomFieldParserSort::CustomByteParser>()), SortField::FIELD_DOC()));
    checkMatches(full, queryA, sort, L"JIHGFEDCBA");
    checkSaneFieldCaches();
    fc->purgeAllCaches();
}

/// test sorts when there's nothing in the index
TEST_F(SortTest, testEmptyIndex) {
    SearcherPtr empty = getEmptyIndex();

    sort = newLucene<Sort>();
    checkMatches(empty, queryX, sort, L"");

    sort->setSort(SortField::FIELD_DOC());
    checkMatches(empty, queryX, sort, L"");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), SortField::FIELD_DOC()));
    checkMatches(empty, queryX, sort, L"");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING, true), SortField::FIELD_DOC()));
    checkMatches(empty, queryX, sort, L"");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE), newLucene<SortField>(L"string", SortField::STRING)));
    checkMatches(empty, queryX, sort, L"");
}

namespace TestNewCustomFieldParserSort {

class MyIntParser : public IntParser {
public:
    virtual ~MyIntParser() {
    }

public:
    virtual int32_t parseInt(const String& string) {
        return (string[0] - L'A') * 123456;
    }
};

class MyFieldComparator : public FieldComparator {
public:
    MyFieldComparator(int32_t numHits) {
        slotValues = Collection<int32_t>::newInstance(numHits);
        bottomValue = 0;
    }

    virtual ~MyFieldComparator() {
    }

public:
    Collection<int32_t> docValues;
    Collection<int32_t> slotValues;
    int32_t bottomValue;

public:
    virtual void copy(int32_t slot, int32_t doc) {
        slotValues[slot] = docValues[doc];
    }

    virtual int32_t compare(int32_t slot1, int32_t slot2) {
        return slotValues[slot1] - slotValues[slot2];
    }

    virtual int32_t compareBottom(int32_t doc) {
        return bottomValue - docValues[doc];
    }

    virtual void setBottom(int32_t slot) {
        bottomValue = slotValues[slot];
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        docValues = FieldCache::DEFAULT()->getInts(reader, L"parser", newLucene<MyIntParser>());
    }

    virtual ComparableValue value(int32_t slot) {
        return slotValues[slot];
    }
};

class MyFieldComparatorSource : public FieldComparatorSource {
public:
    virtual ~MyFieldComparatorSource() {
    }

public:
    virtual FieldComparatorPtr newComparator(const String& fieldname, int32_t numHits, int32_t sortPos, bool reversed) {
        return newLucene<MyFieldComparator>(numHits);
    }
};

}

// Test sorting with custom FieldComparator
TEST_F(SortTest, testNewCustomFieldParserSort) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"parser", newLucene<TestNewCustomFieldParserSort::MyFieldComparatorSource>())));
    checkMatches(full, queryA, sort, L"JIHGFEDCBA");
}

/// test sorts in reverse
TEST_F(SortTest, testReverseSort) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"", SortField::SCORE, true), SortField::FIELD_DOC()));
    checkMatches(full, queryX, sort, L"IEGCA");
    checkMatches(full, queryY, sort, L"JFHDB");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"", SortField::DOC, true)));
    checkMatches(full, queryX, sort, L"IGECA");
    checkMatches(full, queryY, sort, L"JHFDB");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true)));
    checkMatches(full, queryX, sort, L"CAEGI");
    checkMatches(full, queryY, sort, L"BJFHD");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE, true)));
    checkMatches(full, queryX, sort, L"AECIG");
    checkMatches(full, queryY, sort, L"BFJHD");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING, true)));
    checkMatches(full, queryX, sort, L"CEGIA");
    checkMatches(full, queryY, sort, L"BFHJD");
}

/// test sorting when the sort field is empty (undefined) for some of the documents
TEST_F(SortTest, testEmptyFieldSort) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING)));
    checkMatches(full, queryF, sort, L"ZJI");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING, true)));
    checkMatches(full, queryF, sort, L"IJZ");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"i18n", std::locale())));
    checkMatches(full, queryF, sort, L"ZJI");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"i18n", std::locale(), true)));
    checkMatches(full, queryF, sort, L"IJZ");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT)));
    checkMatches(full, queryF, sort, L"IZJ");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true)));
    checkMatches(full, queryF, sort, L"JZI");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(full, queryF, sort, L"ZJI");

    // using a non-existing field as first sort key shouldn't make a difference
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"nosuchfield", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(full, queryF, sort, L"ZJI");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE, true)));
    checkMatches(full, queryF, sort, L"IJZ");

    // When a field is null for both documents, the next SortField should be used.
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(full, queryG, sort, L"ZWXY");

    // Reverse the last criterion to make sure the test didn't pass by chance
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE, true)));
    checkMatches(full, queryG, sort, L"ZYXW");

    // Do the same for a MultiSearcher
    SearcherPtr multiSearcher = newLucene<MultiSearcher>(newCollection<SearchablePtr>(full));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(multiSearcher, queryG, sort, L"ZWXY");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE, true)));
    checkMatches(multiSearcher, queryG, sort, L"ZYXW");

    // Don't close the multiSearcher. it would close the full searcher too!

    // Do the same for a ParallelMultiSearcher
    SearcherPtr parallelSearcher = newLucene<ParallelMultiSearcher>(newCollection<SearchablePtr>(full));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(parallelSearcher, queryG, sort, L"ZWXY");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING), newLucene<SortField>(L"double", SortField::DOUBLE, true)));
    checkMatches(parallelSearcher, queryG, sort, L"ZYXW");
}

/// test sorts using a series of fields
TEST_F(SortTest, testSortCombos) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkMatches(full, queryX, sort, L"IGEAC");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true), newLucene<SortField>(L"", SortField::DOC, true)));
    checkMatches(full, queryX, sort, L"CEAGI");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE), newLucene<SortField>(L"string", SortField::STRING)));
    checkMatches(full, queryX, sort, L"GICEA");
}

/// test using a Locale for sorting strings
TEST_F(SortTest, testLocaleSort) {
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", std::locale())));
    checkMatches(full, queryX, sort, L"AIGEC");
    checkMatches(full, queryY, sort, L"DJHFB");

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", std::locale(), true)));
    checkMatches(full, queryX, sort, L"CEGIA");
    checkMatches(full, queryY, sort, L"BFHJD");
}

/// test a variety of sorts using more than one searcher
TEST_F(SortTest, testMultiSort) {
    MultiSearcherPtr searcher = newLucene<MultiSearcher>(newCollection<SearchablePtr>(searchX, searchY));
    runMultiSorts(searcher, false);
}

/// test a variety of sorts using a parallel multisearcher
TEST_F(SortTest, testParallelMultiSort) {
    MultiSearcherPtr searcher = newLucene<ParallelMultiSearcher>(newCollection<SearchablePtr>(searchX, searchY));
    runMultiSorts(searcher, false);
}

// test that the relevancy scores are the same even if hits are sorted
TEST_F(SortTest, testNormalizedScores) {
    // capture relevancy scores
    MapStringDouble scoresX = getScores(full->search(queryX, FilterPtr(), 1000)->scoreDocs, full);
    MapStringDouble scoresY = getScores(full->search(queryY, FilterPtr(), 1000)->scoreDocs, full);
    MapStringDouble scoresA = getScores(full->search(queryA, FilterPtr(), 1000)->scoreDocs, full);

    // we'll test searching locally, remote and multi

    MultiSearcherPtr multi  = newLucene<MultiSearcher>(newCollection<SearchablePtr>(searchX, searchY));

    // change sorting and make sure relevancy stays the same

    sort = newLucene<Sort>();
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(SortField::FIELD_DOC());
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"double", SortField::DOUBLE)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT, true), newLucene<SortField>(L"", SortField::DOC, true)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));

    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"int", SortField::INT), newLucene<SortField>(L"string", SortField::STRING)));
    checkSameValues(scoresX, getScores(full->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresX, getScores(multi->search(queryX, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresY, getScores(full->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresY, getScores(multi->search(queryY, FilterPtr(), 1000, sort)->scoreDocs, multi));
    checkSameValues(scoresA, getScores(full->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, full));
    checkSameValues(scoresA, getScores(multi->search(queryA, FilterPtr(), 1000, sort)->scoreDocs, multi));
}

namespace TestTopDocsScores {

class TopDocsFilter : public Filter {
public:
    TopDocsFilter(const TopDocsPtr& docs) {
        this->docs = docs;
    }

    virtual ~TopDocsFilter() {
    }

protected:
    TopDocsPtr docs;

public:
    virtual DocIdSetPtr getDocIdSet(const IndexReaderPtr& reader) {
        BitSetPtr bs = newLucene<BitSet>(reader->maxDoc());
        bs->set((uint32_t)0, (uint32_t)reader->maxDoc());
        bs->set(docs->scoreDocs[0]->doc);
        return newLucene<DocIdBitSet>(bs);
    }
};

}

TEST_F(SortTest, testTopDocsScores) {
    SortPtr sort = newLucene<Sort>();
    int32_t numDocs = 10;

    // try to pick a query that will result in an unnormalized score greater than 1 to test for correct normalization
    TopDocsPtr docs1 = full->search(queryE, FilterPtr(), numDocs, sort);

    // a filter that only allows through the first hit
    FilterPtr filter = newLucene<TestTopDocsScores::TopDocsFilter>(docs1);

    TopDocsPtr docs2 = full->search(queryE, filter, numDocs, sort);
    EXPECT_NEAR(docs1->scoreDocs[0]->score, docs2->scoreDocs[0]->score, 1e-6);
}

TEST_F(SortTest, testSortWithoutFillFields) {
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    for (int32_t i = 0; i < sort.size(); ++i) {
        QueryPtr q = newLucene<MatchAllDocsQuery>();
        TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, false, false, false, true);

        full->search(q, tdc);

        Collection<ScoreDocPtr> sd = tdc->topDocs()->scoreDocs;
        for (int32_t j = 1; j < sd.size(); ++j) {
            EXPECT_NE(sd[j]->doc, sd[j - 1]->doc);
        }
    }
}

TEST_F(SortTest, testSortWithoutScoreTracking) {
    // Two Sort criteria to instantiate the multi/single comparators.
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    for (int32_t i = 0; i < sort.size(); ++i) {
        QueryPtr q = newLucene<MatchAllDocsQuery>();
        TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, true, false, false, true);

        full->search(q, tdc);

        TopDocsPtr td = tdc->topDocs();
        Collection<ScoreDocPtr> sd = td->scoreDocs;
        for (int32_t j = 1; j < sd.size(); ++j) {
            EXPECT_TRUE(MiscUtils::isNaN(sd[j]->score));
        }
        EXPECT_TRUE(MiscUtils::isNaN(td->maxScore));
    }
}

TEST_F(SortTest, testSortWithScoreNoMaxScoreTracking) {
    // Two Sort criteria to instantiate the multi/single comparators.
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    for (int32_t i = 0; i < sort.size(); ++i) {
        QueryPtr q = newLucene<MatchAllDocsQuery>();
        TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, true, true, false, true);

        full->search(q, tdc);

        TopDocsPtr td = tdc->topDocs();
        Collection<ScoreDocPtr> sd = td->scoreDocs;
        for (int32_t j = 1; j < sd.size(); ++j) {
            EXPECT_TRUE(!MiscUtils::isNaN(sd[j]->score));
        }
        EXPECT_TRUE(MiscUtils::isNaN(td->maxScore));
    }
}

TEST_F(SortTest, testSortWithScoreAndMaxScoreTracking) {
    // Two Sort criteria to instantiate the multi/single comparators.
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    for (int32_t i = 0; i < sort.size(); ++i) {
        QueryPtr q = newLucene<MatchAllDocsQuery>();
        TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, true, true, true, true);

        full->search(q, tdc);

        TopDocsPtr td = tdc->topDocs();
        Collection<ScoreDocPtr> sd = td->scoreDocs;
        for (int32_t j = 1; j < sd.size(); ++j) {
            EXPECT_TRUE(!MiscUtils::isNaN(sd[j]->score));
        }
        EXPECT_TRUE(!MiscUtils::isNaN(td->maxScore));
    }
}

TEST_F(SortTest, testOutOfOrderDocsScoringSort) {
    // Two Sort criteria to instantiate the multi/single comparators.
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    Collection< Collection<uint8_t> > tfcOptions = newCollection< Collection<uint8_t> >(
                newCollection<uint8_t>(false, false, false),
                newCollection<uint8_t>(false, false, true),
                newCollection<uint8_t>(false, true, false),
                newCollection<uint8_t>(false, true, true),
                newCollection<uint8_t>(true, false, false),
                newCollection<uint8_t>(true, false, true),
                newCollection<uint8_t>(true, true, false),
                newCollection<uint8_t>(true, true, true)
            );
    Collection<String> actualTFCClasses = newCollection<String>(
            L"OutOfOrderOneComparatorNonScoringCollector",
            L"OutOfOrderOneComparatorScoringMaxScoreCollector",
            L"OutOfOrderOneComparatorScoringNoMaxScoreCollector",
            L"OutOfOrderOneComparatorScoringMaxScoreCollector",
            L"OutOfOrderOneComparatorNonScoringCollector",
            L"OutOfOrderOneComparatorScoringMaxScoreCollector",
            L"OutOfOrderOneComparatorScoringNoMaxScoreCollector",
            L"OutOfOrderOneComparatorScoringMaxScoreCollector"
                                          );

    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    // Add a Query with SHOULD, since bw.scorer() returns BooleanScorer2 which delegates to
    // BS if there are no mandatory clauses.
    bq->add(newLucene<MatchAllDocsQuery>(), BooleanClause::SHOULD);
    // Set minNrShouldMatch to 1 so that BQ will not optimize rewrite to return the clause
    // instead of BQ.
    bq->setMinimumNumberShouldMatch(1);
    for (int32_t i = 0; i < sort.size(); ++i) {
        for (int32_t j = 0; j < tfcOptions.size(); ++j) {
            TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, tfcOptions[j][0] == 1, tfcOptions[j][1] == 1, tfcOptions[j][2] == 1, false);

            EXPECT_EQ(tdc->getClassName(), actualTFCClasses[j]);

            full->search(bq, tdc);

            TopDocsPtr td = tdc->topDocs();
            Collection<ScoreDocPtr> sd = td->scoreDocs;
            EXPECT_EQ(10, sd.size());
        }
    }
}

TEST_F(SortTest, testSortWithScoreAndMaxScoreTrackingNoResults) {
    // Two Sort criteria to instantiate the multi/single comparators.
    Collection<SortPtr> sort = newCollection<SortPtr>(newLucene<Sort>(SortField::FIELD_DOC()), newLucene<Sort>());
    for (int32_t i = 0; i < sort.size(); ++i) {
        TopDocsCollectorPtr tdc = TopFieldCollector::create(sort[i], 10, true, true, true, true);
        TopDocsPtr td = tdc->topDocs();
        EXPECT_EQ(0, td->totalHits);
        EXPECT_TRUE(MiscUtils::isNaN(td->maxScore));
    }
}

TEST_F(SortTest, testSortWithStringNoException) {
    RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 5; ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"string", L"a" + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"string", L"b" + StringUtils::toString(i), Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        writer->addDocument (doc);
    }
    writer->optimize(); // enforce one segment to have a higher unique term count in all cases
    writer->close();
    sort->setSort(newCollection<SortFieldPtr>(newLucene<SortField>(L"string", SortField::STRING), SortField::FIELD_DOC()));
    // this should not throw
    IndexSearcherPtr is = newLucene<IndexSearcher>(indexStore, true);
    is->search(newLucene<MatchAllDocsQuery>(), FilterPtr(), 500, sort);
}
