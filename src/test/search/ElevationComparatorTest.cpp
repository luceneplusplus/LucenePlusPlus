/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "Sort.h"
#include "SortField.h"
#include "TopFieldCollector.h"
#include "TopDocsCollector.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "FieldComparatorSource.h"
#include "FieldComparator.h"
#include "FieldCache.h"

using namespace Lucene;

class ElevationFieldComparator : public FieldComparator {
public:
    ElevationFieldComparator(MapStringInt priority, const String& fieldname, int32_t numHits) {
        this->priority = priority;
        this->fieldname = fieldname;
        this->values = Collection<int32_t>::newInstance(numHits);
        this->bottomVal = 0;
    }

    virtual ~ElevationFieldComparator() {
    }

public:
    MapStringInt priority;
    String fieldname;
    StringIndexPtr idIndex;
    Collection<int32_t> values;
    int32_t bottomVal;

public:
    virtual int32_t compare(int32_t slot1, int32_t slot2) {
        return values[slot2] - values[slot1]; // values will be small enough that there is no overflow concern
    }

    virtual void setBottom(int32_t slot) {
        bottomVal = values[slot];
    }

    virtual int32_t compareBottom(int32_t doc) {
        return docVal(doc) - bottomVal;
    }

    virtual void copy(int32_t slot, int32_t doc) {
        values[slot] = docVal(doc);
    }

    virtual void setNextReader(const IndexReaderPtr& reader, int32_t docBase) {
        idIndex = FieldCache::DEFAULT()->getStringIndex(reader, fieldname);
    }

    virtual ComparableValue value(int32_t slot) {
        return values[slot];
    }

protected:
    int32_t docVal(int32_t doc) {
        String id = idIndex->lookup[idIndex->order[doc]];
        return priority.contains(id) ? priority.get(id) : 0;
    }
};

class ElevationComparatorSource : public FieldComparatorSource {
public:
    ElevationComparatorSource(MapStringInt priority) {
        this->priority = priority;
    }

    virtual ~ElevationComparatorSource() {
    }

protected:
    MapStringInt priority;

public:
    virtual FieldComparatorPtr newComparator(const String& fieldname, int32_t numHits, int32_t sortPos, bool reversed) {
        return newLucene<ElevationFieldComparator>(priority, fieldname, numHits);
    }
};

class ElevationComparatorTest : public LuceneTestFixture {
public:
    ElevationComparatorTest() {
        priority = MapStringInt::newInstance();
    }

    virtual ~ElevationComparatorTest() {
    }

public:
    MapStringInt priority;

public:
    DocumentPtr adoc(Collection<String> vals) {
        DocumentPtr doc = newLucene<Document>();
        for (int32_t i = 0; i < vals.size() - 2; i += 2) {
            doc->add(newLucene<Field>(vals[i], vals[i + 1], Field::STORE_YES, Field::INDEX_ANALYZED));
        }
        return doc;
    }

    void runTest(const IndexSearcherPtr& searcher, bool reversed) {
        BooleanQueryPtr newq = newLucene<BooleanQuery>(false);
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"title", L"ipod"));

        newq->add(query, BooleanClause::SHOULD);
        newq->add(getElevatedQuery(newCollection<String>(L"id", L"a", L"id", L"x")), BooleanClause::SHOULD);

        SortPtr sort = newLucene<Sort>(newCollection<SortFieldPtr>(
                                           newLucene<SortField>(L"id", newLucene<ElevationComparatorSource>(priority), false),
                                           newLucene<SortField>(L"", SortField::SCORE, reversed)
                                       ));

        TopDocsCollectorPtr topCollector = TopFieldCollector::create(sort, 50, false, true, true, true);
        searcher->search(newq, FilterPtr(), topCollector);

        TopDocsPtr topDocs = topCollector->topDocs(0, 10);
        int32_t numDocsReturned = topDocs->scoreDocs.size();

        EXPECT_EQ(4, numDocsReturned);

        // 0 and 3 were elevated
        EXPECT_EQ(0, topDocs->scoreDocs[0]->doc);
        EXPECT_EQ(3, topDocs->scoreDocs[1]->doc);

        if (reversed) {
            EXPECT_EQ(2, topDocs->scoreDocs[2]->doc);
            EXPECT_EQ(1, topDocs->scoreDocs[3]->doc);
        } else {
            EXPECT_EQ(1, topDocs->scoreDocs[2]->doc);
            EXPECT_EQ(2, topDocs->scoreDocs[3]->doc);
        }
    }

    QueryPtr getElevatedQuery(Collection<String> vals) {
        BooleanQueryPtr q = newLucene<BooleanQuery>(false);
        q->setBoost(0);
        int32_t max = (vals.size() / 2) + 5;
        for (int32_t i = 0; i < vals.size() - 1; i += 2) {
            q->add(newLucene<TermQuery>(newLucene<Term>(vals[i], vals[i + 1])), BooleanClause::SHOULD);
            priority.put(vals[i + 1], max--);
        }
        return q;
    }
};

TEST_F(ElevationComparatorTest, testSorting) {
    DirectoryPtr directory = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->setMaxBufferedDocs(2);
    writer->setMergeFactor(1000);
    writer->addDocument(adoc(newCollection<String>(L"id", L"a", L"title", L"ipod", L"str_s", L"a")));
    writer->addDocument(adoc(newCollection<String>(L"id", L"b", L"title", L"ipod ipod", L"str_s", L"b")));
    writer->addDocument(adoc(newCollection<String>(L"id", L"c", L"title", L"ipod ipod ipod", L"str_s", L"c")));
    writer->addDocument(adoc(newCollection<String>(L"id", L"x", L"title", L"boosted", L"str_s", L"x")));
    writer->addDocument(adoc(newCollection<String>(L"id", L"y", L"title", L"boosted boosted", L"str_s", L"y")));
    writer->addDocument(adoc(newCollection<String>(L"id", L"z", L"title", L"boosted boosted boosted", L"str_s", L"z")));

    IndexReaderPtr r = writer->getReader();
    writer->close();

    IndexSearcherPtr searcher = newLucene<IndexSearcher>(r);

    runTest(searcher, true);
    runTest(searcher, false);

    searcher->close();
    r->close();
    directory->close();
}
