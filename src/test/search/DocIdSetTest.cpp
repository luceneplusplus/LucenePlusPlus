/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "DocIdSet.h"
#include "DocIdSetIterator.h"
#include "FilteredDocIdSet.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "MatchAllDocsQuery.h"
#include "Filter.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture DocIdSetTest;

static const int32_t maxdoc = 10;

namespace TestFilteredDocIdSet {

class TestDocIdSetIterator : public DocIdSetIterator {
public:
    TestDocIdSetIterator() {
        docid = -1;
    }

    virtual ~TestDocIdSetIterator() {
    }

public:
    int32_t docid;

public:
    virtual int32_t docID() {
        return docid;
    }

    virtual int32_t nextDoc() {
        return ++docid < maxdoc ? docid : (docid = NO_MORE_DOCS);
    }

    virtual int32_t advance(int32_t target) {
        while (nextDoc() < target) {
        }
        return docid;
    }
};

class TestDocIdSet : public DocIdSet {
public:
    virtual ~TestDocIdSet() {
    }

public:
    virtual DocIdSetIteratorPtr iterator() {
        return newLucene<TestDocIdSetIterator>();
    }
};

class TestFilteredDocIdSet : public FilteredDocIdSet {
public:
    TestFilteredDocIdSet(const DocIdSetPtr& innerSet) : FilteredDocIdSet(innerSet) {
    }

    virtual ~TestFilteredDocIdSet() {
    }

protected:
    virtual bool match(int32_t docid) {
        return (docid % 2 == 0); // validate only even docids
    }
};

}

TEST_F(DocIdSetTest, testFilteredDocIdSet) {
    DocIdSetPtr innerSet = newLucene<TestFilteredDocIdSet::TestDocIdSet>();
    DocIdSetPtr filteredSet = newLucene<TestFilteredDocIdSet::TestFilteredDocIdSet>(innerSet);

    DocIdSetIteratorPtr iter = filteredSet->iterator();
    Collection<int32_t> docs = Collection<int32_t>::newInstance();
    int32_t doc = iter->advance(3);
    if (doc != DocIdSetIterator::NO_MORE_DOCS) {
        docs.add(doc);
        while ((doc = iter->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
            docs.add(doc);
        }
    }

    Collection<int32_t> answer = newCollection<int32_t>(4, 6, 8);
    EXPECT_TRUE(docs.equals(answer));
}

namespace TestNullDocIdSet {

class TestFilter : public Filter {
public:
    virtual ~TestFilter() {
    }

public:
    virtual DocIdSetPtr getDocIdSet(const IndexReaderPtr& reader) {
        return DocIdSetPtr();
    }
};

}

/// Tests that if a Filter produces a null DocIdSet, which is given to IndexSearcher, everything works fine
TEST_F(DocIdSetTest, testNullDocIdSet) {
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"c", L"val", Field::STORE_NO, Field::INDEX_ANALYZED_NO_NORMS));
    writer->addDocument(doc);
    writer->close();

    // First verify the document is searchable.
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    EXPECT_EQ(1, searcher->search(newLucene<MatchAllDocsQuery>(), 10)->totalHits);

    // Now search with a Filter which returns a null DocIdSet
    FilterPtr f = newLucene<TestNullDocIdSet::TestFilter>();
    EXPECT_EQ(0, searcher->search(newLucene<MatchAllDocsQuery>(), f, 10)->totalHits);
    searcher->close();
}
