/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "Filter.h"
#include "OpenBitSet.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Term.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

DECLARE_SHARED_PTR(SimpleDocIdSetFilter)

class SimpleDocIdSetFilter : public Filter {
public:
    SimpleDocIdSetFilter(Collection<int32_t> docs) {
        this->docs = docs;
        this->docBase = 0;
        this->index = 0;
    }

    virtual ~SimpleDocIdSetFilter() {
    }

protected:
    int32_t docBase;
    Collection<int32_t> docs;
    int32_t index;

public:
    virtual DocIdSetPtr getDocIdSet(const IndexReaderPtr& reader) {
        OpenBitSetPtr set = newLucene<OpenBitSet>();
        int32_t limit = docBase + reader->maxDoc();
        for (; index < docs.size(); ++index) {
            int32_t docId = docs[index];
            if (docId > limit) {
                break;
            }
            set->set(docId - docBase);
        }
        docBase = limit;
        return set->isEmpty() ? DocIdSetPtr() : set;
    }

    void reset() {
        index = 0;
        docBase = 0;
    }
};

static const String FIELD = L"category";

static void searchFiltered(const IndexWriterPtr& writer, const DirectoryPtr& directory, const FilterPtr& filter, bool optimize) {
    for (int32_t i = 0; i < 60; ++i) {
        // Simple docs
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD, StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }
    if (optimize) {
        writer->optimize();
    }
    writer->close();

    BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"36")), BooleanClause::SHOULD);

    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    Collection<ScoreDocPtr> hits = indexSearcher->search(booleanQuery, filter, 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
}

typedef LuceneTestFixture FilteredSearchTest;

TEST_F(FilteredSearchTest, testFilteredSearch) {
    bool enforceSingleSegment = true;
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    Collection<int32_t> filterBits = newCollection<int32_t>(1, 36);
    SimpleDocIdSetFilterPtr filter = newLucene<SimpleDocIdSetFilter>(filterBits);
    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    searchFiltered(writer, directory, filter, enforceSingleSegment);
    // run the test on more than one segment
    enforceSingleSegment = false;
    // reset - it is stateful
    filter->reset();
    writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    // we index 60 docs - this will create 6 segments
    writer->setMaxBufferedDocs(10);
    searchFiltered(writer, directory, filter, enforceSingleSegment);
}
