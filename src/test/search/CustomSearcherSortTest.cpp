/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TermQuery.h"
#include "Term.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "Sort.h"
#include "SortField.h"
#include "IndexSearcher.h"
#include "IndexReader.h"
#include "BooleanQuery.h"
#include "DateTools.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TopFieldDocs.h"
#include "MultiSearcher.h"
#include "Random.h"
#include "MiscUtils.h"

using namespace Lucene;

class CustomSearcher : public IndexSearcher {
public:
    CustomSearcher(const DirectoryPtr& directory, int32_t switcher) : IndexSearcher(directory, true) {
        this->switcher = switcher;
    }

    CustomSearcher(const IndexReaderPtr& r, int32_t switcher) : IndexSearcher(r) {
        this->switcher = switcher;
    }

    virtual ~CustomSearcher() {
    }

protected:
    int32_t switcher;

public:
    virtual TopFieldDocsPtr search(const QueryPtr& query, const FilterPtr& filter, int32_t n, const SortPtr& sort) {
        BooleanQueryPtr bq = newLucene<BooleanQuery>();
        bq->add(query, BooleanClause::MUST);
        bq->add(newLucene<TermQuery>(newLucene<Term>(L"mandant", StringUtils::toString(switcher))), BooleanClause::MUST);
        return IndexSearcher::search(bq, filter, n, sort);
    }

    virtual TopDocsPtr search(const QueryPtr& query, const FilterPtr& filter, int32_t n) {
        BooleanQueryPtr bq = newLucene<BooleanQuery>();
        bq->add(query, BooleanClause::MUST);
        bq->add(newLucene<TermQuery>(newLucene<Term>(L"mandant", StringUtils::toString(switcher))), BooleanClause::MUST);
        return IndexSearcher::search(bq, filter, n);
    }
};

class CustomSearcherSortTest : public LuceneTestFixture {
public:
    CustomSearcherSortTest() {
        random = newLucene<Random>();
        index = getIndex();
        query = newLucene<TermQuery>(newLucene<Term>(L"content", L"test"));
    }

    virtual ~CustomSearcherSortTest() {
    }

protected:
    DirectoryPtr index;
    QueryPtr query;
    RandomPtr random;

    static const int32_t INDEX_SIZE;

public:
    DirectoryPtr getIndex() {
        RAMDirectoryPtr indexStore = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(indexStore, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < INDEX_SIZE; ++i) {
            DocumentPtr doc = newLucene<Document>();
            if ((i % 5) != 0) {
                doc->add(newLucene<Field>(L"publicationDate_", getLuceneDate(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            }
            if ((i % 7) == 0) {
                doc->add(newLucene<Field>(L"content", L"test", Field::STORE_YES, Field::INDEX_ANALYZED));
            }
            // every document has a defined 'mandant' field
            doc->add(newLucene<Field>(L"mandant", StringUtils::toString(i % 3), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        writer->optimize();
        writer->close();
        return indexStore;
    }

    String getLuceneDate() {
        DateTools::setDateOrder(DateTools::DATEORDER_DMY);
        boost::posix_time::ptime base = DateTools::parseDate(L"01/01/1980");
        return DateTools::timeToString(MiscUtils::getTimeMillis(base) + random->nextInt() - INT_MIN, DateTools::RESOLUTION_DAY);
    }

    /// make sure the documents returned by the search match the expected list
    void matchHits(const SearcherPtr& searcher, const SortPtr& sort) {
        // make a query without sorting first
        Collection<ScoreDocPtr> hitsByRank = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
        checkHits(hitsByRank); // check for duplicates
        Map<int32_t, int32_t> resultMap = Map<int32_t, int32_t>::newInstance();
        // store hits in Map - Map does not allow duplicates; existing entries are silently overwritten
        for (int32_t hitid = 0; hitid < hitsByRank.size(); ++hitid) {
            resultMap.put(hitsByRank[hitid]->doc, hitid);
        }

        // now make a query using the sort criteria
        Collection<ScoreDocPtr> resultSort = searcher->search (query, FilterPtr(), 1000, sort)->scoreDocs;
        checkHits(resultSort); // check for duplicates

        // besides the sorting both sets of hits must be identical
        for (int32_t hitid = 0; hitid < resultSort.size(); ++hitid) {
            int32_t idHitDate = resultSort[hitid]->doc; // document ID from sorted search
            EXPECT_TRUE(resultMap.contains(idHitDate)); // same ID must be in the Map from the rank-sorted search
            // every hit must appear once in both result sets --> remove it from the Map.
            // At the end the Map must be empty!
            resultMap.remove(idHitDate);
        }
        EXPECT_TRUE(resultMap.empty());
    }

    void checkHits(Collection<ScoreDocPtr> hits) {
        if (hits) {
            Map<int32_t, int32_t> idMap = Map<int32_t, int32_t>::newInstance();
            for (int32_t docnum = 0; docnum < hits.size(); ++docnum) {
                int32_t luceneId = hits[docnum]->doc;
                EXPECT_TRUE(!idMap.contains(luceneId));
                idMap.put(luceneId, docnum);
            }
        }
    }
};

const int32_t CustomSearcherSortTest::INDEX_SIZE = 2000;

/// Run the test using two CustomSearcher instances.
TEST_F(CustomSearcherSortTest, testFieldSortCustomSearcher) {
    SortPtr custSort = newLucene<Sort>(newCollection<SortFieldPtr>(newLucene<SortField>(L"publicationDate_", SortField::STRING), SortField::FIELD_SCORE()));
    SearcherPtr searcher = newLucene<CustomSearcher>(index, 2);
    // search and check hits
    matchHits(searcher, custSort);
}

/// Run the test using one CustomSearcher wrapped by a MultiSearcher.
TEST_F(CustomSearcherSortTest, testFieldSortSingleSearcher) {
    SortPtr custSort = newLucene<Sort>(newCollection<SortFieldPtr>(newLucene<SortField>(L"publicationDate_", SortField::STRING), SortField::FIELD_SCORE()));
    SearcherPtr searcher = newLucene<MultiSearcher>(newCollection<SearchablePtr>(newLucene<CustomSearcher>(index, 2)));
    // search and check hits
    matchHits(searcher, custSort);
}

/// Run the test using two CustomSearcher instances.
TEST_F(CustomSearcherSortTest, testFieldSortMultiCustomSearcher) {
    SortPtr custSort = newLucene<Sort>(newCollection<SortFieldPtr>(newLucene<SortField>(L"publicationDate_", SortField::STRING), SortField::FIELD_SCORE()));
    SearcherPtr searcher = newLucene<MultiSearcher>(newCollection<SearchablePtr>(newLucene<CustomSearcher>(index, 0), newLucene<CustomSearcher>(index, 2)));
    // search and check hits
    matchHits(searcher, custSort);
}
