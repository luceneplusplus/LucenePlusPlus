/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "WhitespaceAnalyzer.h"
#include "Document.h"
#include "Field.h"
#include "DateTools.h"
#include "IndexSearcher.h"
#include "Sort.h"
#include "SortField.h"
#include "QueryParser.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "TopFieldDocs.h"

using namespace Lucene;

class DateSortTest : public LuceneTestFixture {
public:
    DateSortTest() {
        // Create an index writer.
        directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);

        // oldest doc:
        // Add the first document.  text = "Document 1"  dateTime = Oct 10 03:25:22 EDT 2007
        writer->addDocument(createDocument(L"Document 1", 1192001122000LL));
        // Add the second document.  text = "Document 2"  dateTime = Oct 10 03:25:26 EDT 2007
        writer->addDocument(createDocument(L"Document 2", 1192001126000LL));
        // Add the third document.  text = "Document 3"  dateTime = Oct 11 07:12:13 EDT 2007
        writer->addDocument(createDocument(L"Document 3", 1192101133000LL));
        // Add the fourth document.  text = "Document 4"  dateTime = Oct 11 08:02:09 EDT 2007
        writer->addDocument(createDocument(L"Document 4", 1192104129000LL));
        // latest doc:
        // Add the fifth document.  text = "Document 5"  dateTime = Oct 12 13:25:43 EDT 2007
        writer->addDocument(createDocument(L"Document 5", 1192209943000LL));

        writer->optimize();
        writer->close();
    }

    virtual ~DateSortTest() {
    }

protected:
    static const String TEXT_FIELD;
    static const String DATE_TIME_FIELD;

    DirectoryPtr directory;

public:
    DocumentPtr createDocument(const String& text, int64_t time) {
        DocumentPtr document = newLucene<Document>();

        // Add the text field.
        FieldPtr textField = newLucene<Field>(TEXT_FIELD, text, Field::STORE_YES, Field::INDEX_ANALYZED);
        document->add(textField);

        // Add the date/time field.
        String dateTimeString = DateTools::timeToString(time, DateTools::RESOLUTION_SECOND);
        FieldPtr dateTimeField = newLucene<Field>(DATE_TIME_FIELD, dateTimeString, Field::STORE_YES, Field::INDEX_NOT_ANALYZED);
        document->add(dateTimeField);

        return document;
    }
};

const String DateSortTest::TEXT_FIELD = L"text";
const String DateSortTest::DATE_TIME_FIELD = L"dateTime";

TEST_F(DateSortTest, testReverseDateSort) {
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(directory, true);

    SortPtr sort = newLucene<Sort>(newLucene<SortField>(DATE_TIME_FIELD, SortField::STRING, true));

    QueryParserPtr queryParser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, TEXT_FIELD, newLucene<WhitespaceAnalyzer>());
    QueryPtr query = queryParser->parse(L"Document");

    // Execute the search and process the search results.
    Collection<String> actualOrder = Collection<String>::newInstance(5);
    Collection<ScoreDocPtr>hits = searcher->search(query, FilterPtr(), 1000, sort)->scoreDocs;
    for (int32_t i = 0; i < hits.size(); ++i) {
        DocumentPtr document = searcher->doc(hits[i]->doc);
        String text = document->get(TEXT_FIELD);
        actualOrder[i] = text;
    }
    searcher->close();

    // Set up the expected order (ie. Document 5, 4, 3, 2, 1).
    Collection<String> expectedOrder = Collection<String>::newInstance(5);
    expectedOrder[0] = L"Document 5";
    expectedOrder[1] = L"Document 4";
    expectedOrder[2] = L"Document 3";
    expectedOrder[3] = L"Document 2";
    expectedOrder[4] = L"Document 1";

    EXPECT_TRUE(expectedOrder.equals(actualOrder));
}
