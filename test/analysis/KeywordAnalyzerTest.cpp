/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "RAMDirectory.h"
#include "IndexSearcher.h"
#include "IndexWriter.h"
#include "SimpleAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "PerFieldAnalyzerWrapper.h"
#include "KeywordAnalyzer.h"
#include "QueryParser.h"
#include "Query.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TermDocs.h"
#include "Term.h"
#include "TokenStream.h"
#include "OffsetAttribute.h"
#include "IndexReader.h"
#include "StringReader.h"

using namespace Lucene;

class KeywordAnalyzerTestFixture : public BaseTokenStreamFixture
{
public:
    KeywordAnalyzerTestFixture()
    {
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"partnum", L"Q36", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"description", L"Illidium Space Modulator", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
    }
    
    virtual ~KeywordAnalyzerTestFixture()
    {
    }

protected:
    RAMDirectoryPtr directory;
    IndexSearcherPtr searcher;
};

BOOST_FIXTURE_TEST_SUITE(KeywordAnalyzerTest, KeywordAnalyzerTestFixture)

BOOST_AUTO_TEST_CASE(testPerFieldAnalyzer)
{
    PerFieldAnalyzerWrapperPtr analyzer = newLucene<PerFieldAnalyzerWrapper>(newLucene<SimpleAnalyzer>());
    analyzer->addAnalyzer(L"partnum", newLucene<KeywordAnalyzer>());

    QueryParserPtr queryParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"description", analyzer);
    QueryPtr query = queryParser->parse(L"partnum:Q36 AND SPACE");

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    BOOST_CHECK_EQUAL(L"+partnum:Q36 +space", query->toString(L"description"));
    BOOST_CHECK_EQUAL(1, hits.size());
}

BOOST_AUTO_TEST_CASE(testMutipleDocument)
{
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<KeywordAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"partnum", L"Q36", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"partnum", L"Q37", Field::STORE_YES, Field::INDEX_ANALYZED));
    writer->addDocument(doc);
    writer->close();

    IndexReaderPtr reader = IndexReader::open(dir, true);
    TermDocsPtr td = reader->termDocs(newLucene<Term>(L"partnum", L"Q36"));
    BOOST_CHECK(td->next());
    td = reader->termDocs(newLucene<Term>(L"partnum", L"Q37"));
    BOOST_CHECK(td->next());
}

BOOST_AUTO_TEST_CASE(testOffsets)
{
    TokenStreamPtr stream = newLucene<KeywordAnalyzer>()->tokenStream(L"field", newLucene<StringReader>(L"abcd"));
    OffsetAttributePtr offsetAtt = stream->addAttribute<OffsetAttribute>();
    BOOST_CHECK(stream->incrementToken());
    BOOST_CHECK_EQUAL(0, offsetAtt->startOffset());
    BOOST_CHECK_EQUAL(4, offsetAtt->endOffset());
}

BOOST_AUTO_TEST_SUITE_END()
