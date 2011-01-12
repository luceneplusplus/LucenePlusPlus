/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TermQuery.h"
#include "IndexSearcher.h"
#include "Term.h"
#include "BooleanQuery.h"
#include "QueryUtils.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "TopDocs.h"

using namespace Lucene;

class BooleanOrFixture : public LuceneTestFixture
{
public:
    BooleanOrFixture()
    {
        t1 = newLucene<TermQuery>(newLucene<Term>(FIELD_T, L"files"));
        t2 = newLucene<TermQuery>(newLucene<Term>(FIELD_T, L"deleting"));
        c1 = newLucene<TermQuery>(newLucene<Term>(FIELD_C, L"production"));
        c2 = newLucene<TermQuery>(newLucene<Term>(FIELD_C, L"optimize"));
        
        RAMDirectoryPtr rd = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(rd, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);

        DocumentPtr d = newLucene<Document>();
        d->add(newLucene<Field>(FIELD_T, L"Optimize not deleting all files", Field::STORE_YES, Field::INDEX_ANALYZED));
        d->add(newLucene<Field>(FIELD_C, L"Deleted when I run an optimize in our production environment.", Field::STORE_YES, Field::INDEX_ANALYZED));

        writer->addDocument(d);
        writer->close();
        searcher = newLucene<IndexSearcher>(rd, true);
    }
    
    virtual ~BooleanOrFixture()
    {
    }

protected:
    static const String FIELD_T;
    static const String FIELD_C;

    TermQueryPtr t1;
    TermQueryPtr t2;
    TermQueryPtr c1;
    TermQueryPtr c2;
    
    IndexSearcherPtr searcher;

public:
    int32_t search(QueryPtr q)
    {
        QueryUtils::check(q, searcher);
        return searcher->search(q, FilterPtr(), 1000)->totalHits;
    }
};

const String BooleanOrFixture::FIELD_T = L"T";
const String BooleanOrFixture::FIELD_C = L"C";

BOOST_FIXTURE_TEST_SUITE(BooleanOrTest, BooleanOrFixture)

BOOST_AUTO_TEST_CASE(testElements)
{
    BOOST_CHECK_EQUAL(1, search(t1));
    BOOST_CHECK_EQUAL(1, search(t2));
    BOOST_CHECK_EQUAL(1, search(c1));
    BOOST_CHECK_EQUAL(1, search(c2));
}

BOOST_AUTO_TEST_CASE(testFlat)
{
    BooleanQueryPtr q = newLucene<BooleanQuery>();
    q->add(newLucene<BooleanClause>(t1, BooleanClause::SHOULD));
    q->add(newLucene<BooleanClause>(t2, BooleanClause::SHOULD));
    q->add(newLucene<BooleanClause>(c1, BooleanClause::SHOULD));
    q->add(newLucene<BooleanClause>(c2, BooleanClause::SHOULD));
    BOOST_CHECK_EQUAL(1, search(q));
}

BOOST_AUTO_TEST_CASE(testParenthesisMust)
{
    BooleanQueryPtr q3 = newLucene<BooleanQuery>();
    q3->add(newLucene<BooleanClause>(t1, BooleanClause::SHOULD));
    q3->add(newLucene<BooleanClause>(t2, BooleanClause::SHOULD));
    BooleanQueryPtr q4 = newLucene<BooleanQuery>();
    q4->add(newLucene<BooleanClause>(c1, BooleanClause::MUST));
    q4->add(newLucene<BooleanClause>(c2, BooleanClause::MUST));
    BooleanQueryPtr q2 = newLucene<BooleanQuery>();
    q2->add(q3, BooleanClause::SHOULD);
    q2->add(q4, BooleanClause::SHOULD);
    BOOST_CHECK_EQUAL(1, search(q2));
}

BOOST_AUTO_TEST_CASE(testParenthesisMust2)
{
    BooleanQueryPtr q3 = newLucene<BooleanQuery>();
    q3->add(newLucene<BooleanClause>(t1, BooleanClause::SHOULD));
    q3->add(newLucene<BooleanClause>(t2, BooleanClause::SHOULD));
    BooleanQueryPtr q4 = newLucene<BooleanQuery>();
    q4->add(newLucene<BooleanClause>(c1, BooleanClause::SHOULD));
    q4->add(newLucene<BooleanClause>(c2, BooleanClause::SHOULD));
    BooleanQueryPtr q2 = newLucene<BooleanQuery>();
    q2->add(q3, BooleanClause::SHOULD);
    q2->add(q4, BooleanClause::MUST);
    BOOST_CHECK_EQUAL(1, search(q2));
}

BOOST_AUTO_TEST_CASE(testParenthesisShould)
{
    BooleanQueryPtr q3 = newLucene<BooleanQuery>();
    q3->add(newLucene<BooleanClause>(t1, BooleanClause::SHOULD));
    q3->add(newLucene<BooleanClause>(t2, BooleanClause::SHOULD));
    BooleanQueryPtr q4 = newLucene<BooleanQuery>();
    q4->add(newLucene<BooleanClause>(c1, BooleanClause::SHOULD));
    q4->add(newLucene<BooleanClause>(c2, BooleanClause::SHOULD));
    BooleanQueryPtr q2 = newLucene<BooleanQuery>();
    q2->add(q3, BooleanClause::SHOULD);
    q2->add(q4, BooleanClause::SHOULD);
    BOOST_CHECK_EQUAL(1, search(q2));
}

BOOST_AUTO_TEST_SUITE_END()
