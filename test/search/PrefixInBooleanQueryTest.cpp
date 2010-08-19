/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "PrefixQuery.h"
#include "Term.h"
#include "TopDocs.h"
#include "TermQuery.h"
#include "BooleanQuery.h"

using namespace Lucene;

class PrefixInBooleanQueryFixture : public LuceneTestFixture
{
public:
    PrefixInBooleanQueryFixture()
    {
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        
        for (int32_t i = 0; i < 5137; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, L"meaninglessnames", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, L"tangfulin", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        
        for (int32_t i = 5138; i < 11377; ++i)
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, L"meaninglessnames", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        {
            DocumentPtr doc = newLucene<Document>();
            doc->add(newLucene<Field>(FIELD, L"tangfulin", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
            writer->addDocument(doc);
        }
        
        writer->close();
    }
    
    virtual ~PrefixInBooleanQueryFixture()
    {
    }

protected:
    RAMDirectoryPtr directory;

public:
    static const String FIELD;
};

const String PrefixInBooleanQueryFixture::FIELD = L"name";

BOOST_FIXTURE_TEST_SUITE(PrefixInBooleanQueryTest, PrefixInBooleanQueryFixture)

BOOST_AUTO_TEST_CASE(testPrefixQuery)
{
    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    QueryPtr query = newLucene<PrefixQuery>(newLucene<Term>(FIELD, L"tang"));
    BOOST_CHECK_EQUAL(2, indexSearcher->search(query, FilterPtr(), 1000)->totalHits);
}

BOOST_AUTO_TEST_CASE(testTermQuery)
{
    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    QueryPtr query = newLucene<TermQuery>(newLucene<Term>(FIELD, L"tangfulin"));
    BOOST_CHECK_EQUAL(2, indexSearcher->search(query, FilterPtr(), 1000)->totalHits);
}

BOOST_AUTO_TEST_CASE(testTermBooleanQuery)
{
    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"tangfulin")), BooleanClause::SHOULD);
    query->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"notexistnames")), BooleanClause::SHOULD);
    BOOST_CHECK_EQUAL(2, indexSearcher->search(query, FilterPtr(), 1000)->totalHits);
}

BOOST_AUTO_TEST_CASE(testPrefixBooleanQuery)
{
    IndexSearcherPtr indexSearcher = newLucene<IndexSearcher>(directory, true);
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    query->add(newLucene<PrefixQuery>(newLucene<Term>(FIELD, L"tang")), BooleanClause::SHOULD);
    query->add(newLucene<TermQuery>(newLucene<Term>(FIELD, L"notexistnames")), BooleanClause::SHOULD);
    BOOST_CHECK_EQUAL(2, indexSearcher->search(query, FilterPtr(), 1000)->totalHits);
}

BOOST_AUTO_TEST_SUITE_END()
