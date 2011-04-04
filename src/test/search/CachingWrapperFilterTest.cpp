/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "KeywordAnalyzer.h"
#include "WhitespaceAnalyzer.h"
#include "IndexReader.h"
#include "MockFilter.h"
#include "CachingWrapperFilter.h"
#include "QueryWrapperFilter.h"
#include "TermQuery.h"
#include "Term.h"
#include "NumericRangeFilter.h"
#include "FieldCacheRangeFilter.h"
#include "OpenBitSet.h"
#include "DocIdSet.h"
#include "OpenBitSetDISI.h"
#include "IndexSearcher.h"
#include "Field.h"
#include "Document.h"
#include "TopDocs.h"
#include "ScoreDoc.h"
#include "MatchAllDocsQuery.h"
#include "ConstantScoreQuery.h"
#include "MiscUtils.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(CachingWrapperFilterTest, LuceneTestFixture)

static void checkDocIdSetCacheable(IndexReaderPtr reader, FilterPtr filter, bool shouldCacheable)
{
    CachingWrapperFilterPtr cacher = newLucene<CachingWrapperFilter>(filter);
    DocIdSetPtr originalSet = filter->getDocIdSet(reader);
    DocIdSetPtr cachedSet = cacher->getDocIdSet(reader);
    BOOST_CHECK(cachedSet->isCacheable());
    BOOST_CHECK_EQUAL(shouldCacheable, originalSet->isCacheable());
    if (originalSet->isCacheable())
        BOOST_CHECK(MiscUtils::equalTypes(originalSet, cachedSet));
    else
        BOOST_CHECK(MiscUtils::typeOf<OpenBitSetDISI>(cachedSet));
}

static IndexReaderPtr refreshReader(IndexReaderPtr reader)
{
    IndexReaderPtr oldReader = reader;
    reader = reader->reopen();
    if (reader != oldReader)
        oldReader->close();
    return reader;
}

BOOST_AUTO_TEST_CASE(testCachingWorks)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);

    MockFilterPtr filter = newLucene<MockFilter>();
    CachingWrapperFilterPtr cacher = newLucene<CachingWrapperFilter>(filter);

    // first time, nested filter is called
    cacher->getDocIdSet(reader);
    BOOST_CHECK(filter->wasCalled());

    // make sure no exception if cache is holding the wrong docIdSet
    cacher->getDocIdSet(reader);

    // second time, nested filter should not be called
    filter->clear();
    cacher->getDocIdSet(reader);
    BOOST_CHECK(!filter->wasCalled());

    reader->close();
}

namespace TestNullDocIdSet
{
    class NullDocIdSetFilter : public Filter
    {
    public:
        virtual ~NullDocIdSetFilter()
        {
        }
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
        {
            return DocIdSetPtr();
        }
    };
}

BOOST_AUTO_TEST_CASE(testNullDocIdSet)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    FilterPtr filter = newLucene<TestNullDocIdSet::NullDocIdSetFilter>();
    CachingWrapperFilterPtr cacher = newLucene<CachingWrapperFilter>(filter);

    // the caching filter should return the empty set constant
    BOOST_CHECK_EQUAL(DocIdSet::EMPTY_DOCIDSET(), cacher->getDocIdSet(reader));
    
    reader->close();
}

namespace TestNullDocIdSetIterator
{
    class NullDocIdSetIterator : public DocIdSet
    {
    public:
        virtual ~NullDocIdSetIterator()
        {
        }
    
    public:
        virtual DocIdSetIteratorPtr iterator()
        {
            return DocIdSetIteratorPtr();
        }
    };
    
    class NullDocIdSetIteratorFilter : public Filter
    {
    public:
        virtual ~NullDocIdSetIteratorFilter()
        {
        }
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
        {
            return newLucene<NullDocIdSetIterator>();
        }
    };
}

BOOST_AUTO_TEST_CASE(testNullDocIdSetIterator)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    FilterPtr filter = newLucene<TestNullDocIdSetIterator::NullDocIdSetIteratorFilter>();
    CachingWrapperFilterPtr cacher = newLucene<CachingWrapperFilter>(filter);

    // the caching filter should return the empty set constant
    BOOST_CHECK_EQUAL(DocIdSet::EMPTY_DOCIDSET(), cacher->getDocIdSet(reader));
    
    reader->close();
}

namespace TestIsCacheable
{
    class OpenBitSetFilter : public Filter
    {
    public:
        virtual ~OpenBitSetFilter()
        {
        }
    
    public:
        virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
        {
            return newLucene<OpenBitSet>();
        }
    };
}

BOOST_AUTO_TEST_CASE(testIsCacheable)
{
    DirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    
    // not cacheable
    checkDocIdSetCacheable(reader, newLucene<QueryWrapperFilter>(newLucene<TermQuery>(newLucene<Term>(L"test", L"value"))), false);
    
    // returns default empty docidset, always cacheable
    checkDocIdSetCacheable(reader, NumericRangeFilter::newIntRange(L"test", 10000, -10000, true, true), true);
    
    // is cacheable
    checkDocIdSetCacheable(reader, FieldCacheRangeFilter::newIntRange(L"test", 10, 20, true, true), true);
    
    // a openbitset filter is always cacheable
    checkDocIdSetCacheable(reader, newLucene<TestIsCacheable::OpenBitSetFilter>(), true);
}

BOOST_AUTO_TEST_CASE(testEnforceDeletions)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), IndexWriter::MaxFieldLengthUNLIMITED);
    IndexReaderPtr reader = writer->getReader();
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(reader);

    // add a doc, refresh the reader, and check that its there
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"id", L"1", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    writer->addDocument(doc);

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    TopDocsPtr docs = searcher->search(newLucene<MatchAllDocsQuery>(), 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    FilterPtr startFilter = newLucene<QueryWrapperFilter>(newLucene<TermQuery>(newLucene<Term>(L"id", L"1")));

    // ignore deletions
    CachingWrapperFilterPtr filter = newLucene<CachingWrapperFilter>(startFilter, CachingWrapperFilter::DELETES_IGNORE);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);
    ConstantScoreQueryPtr constantScore = newLucene<ConstantScoreQuery>(filter);
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    // now delete the doc, refresh the reader, and see that it's not there
    writer->deleteDocuments(newLucene<Term>(L"id", L"1"));

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(0, docs->totalHits);

    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    // force cache to regenerate
    filter = newLucene<CachingWrapperFilter>(startFilter, CachingWrapperFilter::DELETES_RECACHE);

    writer->addDocument(doc);
    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    constantScore = newLucene<ConstantScoreQuery>(filter);
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    // make sure we get a cache hit when we reopen reader that had no change to deletions
    IndexReaderPtr newReader = refreshReader(reader);
    BOOST_CHECK_NE(reader, newReader);
    reader = newReader;
    searcher = newLucene<IndexSearcher>(reader);
    int32_t missCount = filter->missCount;
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);
    BOOST_CHECK_EQUAL(missCount, filter->missCount);

    // now delete the doc, refresh the reader, and see that it's not there
    writer->deleteDocuments(newLucene<Term>(L"id", L"1"));

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    missCount = filter->missCount;
    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(missCount + 1, filter->missCount);
    BOOST_CHECK_EQUAL(0, docs->totalHits);
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(0, docs->totalHits);

    // apply deletions dynamically
    filter = newLucene<CachingWrapperFilter>(startFilter, CachingWrapperFilter::DELETES_DYNAMIC);

    writer->addDocument(doc);
    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);
    constantScore = newLucene<ConstantScoreQuery>(filter);
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(1, docs->totalHits);

    // now delete the doc, refresh the reader, and see that it's not there
    writer->deleteDocuments(newLucene<Term>(L"id", L"1"));

    reader = refreshReader(reader);
    searcher = newLucene<IndexSearcher>(reader);

    docs = searcher->search(newLucene<MatchAllDocsQuery>(), filter, 1);
    BOOST_CHECK_EQUAL(0, docs->totalHits);

    missCount = filter->missCount;
    docs = searcher->search(constantScore, 1);
    BOOST_CHECK_EQUAL(0, docs->totalHits);

    // doesn't count as a miss
    BOOST_CHECK_EQUAL(missCount, filter->missCount);
}

BOOST_AUTO_TEST_SUITE_END()
