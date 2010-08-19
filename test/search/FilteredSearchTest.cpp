/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "Filter.h"
#include "OpenBitSet.h"
#include "IndexWriter.h"
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

class SimpleDocIdSetFilter : public Filter
{
public:
    SimpleDocIdSetFilter(Collection<int32_t> docs)
    {
        bits = newLucene<OpenBitSet>();
        for (int32_t i = 0; i < docs.size(); ++i)
            bits->set(docs[i]);
    }
    
    virtual ~SimpleDocIdSetFilter()
    {
    }

protected:
    OpenBitSetPtr bits;

public:
    virtual DocIdSetPtr getDocIdSet(IndexReaderPtr reader)
    {
        return bits;
    }
};

BOOST_FIXTURE_TEST_SUITE(FilteredSearchTest, LuceneTestFixture)

BOOST_AUTO_TEST_CASE(testFilteredSearch)
{
    String FIELD = L"category";
    
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();
    Collection<int32_t> filterBits = newCollection<int32_t>(1, 36);
    FilterPtr filter = newLucene<SimpleDocIdSetFilter>(filterBits);

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < 60; ++i)
    {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(FIELD, StringUtils::toString(i), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    BooleanQueryPtr booleanQuery = newLucene<BooleanQuery>();
    booleanQuery->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"36")), BooleanClause::SHOULD);

    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    Collection<ScoreDocPtr> hits = indexSearcher->search(booleanQuery, filter, 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(1, hits.size());
}

BOOST_AUTO_TEST_SUITE_END()
