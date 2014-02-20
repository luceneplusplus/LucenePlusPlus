/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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

class KeywordAnalyzerTest : public BaseTokenStreamFixture {
public:
    KeywordAnalyzerTest() {
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<SimpleAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"partnum", L"Q36", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"description", L"Illidium Space Modulator", Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);

        writer->close();

        searcher = newLucene<IndexSearcher>(directory, true);
    }

    virtual ~KeywordAnalyzerTest() {
    }

protected:
    RAMDirectoryPtr directory;
    IndexSearcherPtr searcher;
};

TEST_F(KeywordAnalyzerTest, testPerFieldAnalyzer) {
    PerFieldAnalyzerWrapperPtr analyzer = newLucene<PerFieldAnalyzerWrapper>(newLucene<SimpleAnalyzer>());
    analyzer->addAnalyzer(L"partnum", newLucene<KeywordAnalyzer>());

    QueryParserPtr queryParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"description", analyzer);
    QueryPtr query = queryParser->parse(L"partnum:Q36 AND SPACE");

    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(L"+partnum:Q36 +space", query->toString(L"description"));
    EXPECT_EQ(1, hits.size());
}

TEST_F(KeywordAnalyzerTest, testMutipleDocument) {
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
    EXPECT_TRUE(td->next());
    td = reader->termDocs(newLucene<Term>(L"partnum", L"Q37"));
    EXPECT_TRUE(td->next());
}

TEST_F(KeywordAnalyzerTest, testOffsets) {
    TokenStreamPtr stream = newLucene<KeywordAnalyzer>()->tokenStream(L"field", newLucene<StringReader>(L"abcd"));
    OffsetAttributePtr offsetAtt = stream->addAttribute<OffsetAttribute>();
    EXPECT_TRUE(stream->incrementToken());
    EXPECT_EQ(0, offsetAtt->startOffset());
    EXPECT_EQ(4, offsetAtt->endOffset());
}
