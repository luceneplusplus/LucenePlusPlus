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
#include "IndexReader.h"
#include "PrefixQuery.h"
#include "Term.h"
#include "BooleanQuery.h"
#include "ConstantScoreQuery.h"
#include "Filter.h"
#include "DocIdSetIterator.h"
#include "DocIdSet.h"
#include "MiscUtils.h"

using namespace Lucene;

typedef LuceneTestFixture BooleanPrefixQueryTest;

static int32_t getCount(const IndexReaderPtr& r, const QueryPtr& q) {
    if (MiscUtils::typeOf<BooleanQuery>(q)) {
        return boost::dynamic_pointer_cast<BooleanQuery>(q)->getClauses().size();
    } else if (MiscUtils::typeOf<ConstantScoreQuery>(q)) {
        DocIdSetIteratorPtr iter = boost::dynamic_pointer_cast<ConstantScoreQuery>(q)->getFilter()->getDocIdSet(r)->iterator();
        int32_t count = 0;
        while (iter->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
            ++count;
        }
        return count;
    } else {
        boost::throw_exception(RuntimeException(L"unexpected query"));
        return 0;
    }
}

TEST_F(BooleanPrefixQueryTest, testMethod) {
    RAMDirectoryPtr directory = newLucene<RAMDirectory>();

    Collection<String> categories = newCollection<String>(L"food", L"foodanddrink", L"foodanddrinkandgoodtimes", L"food and drink");

    IndexWriterPtr writer = newLucene<IndexWriter>(directory, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    for (int32_t i = 0; i < categories.size(); ++i) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"category", categories[i], Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        writer->addDocument(doc);
    }
    writer->close();

    IndexReaderPtr reader = IndexReader::open(directory, true);
    PrefixQueryPtr query = newLucene<PrefixQuery>(newLucene<Term>(L"category", L"foo"));
    QueryPtr rw1 = query->rewrite(reader);

    BooleanQueryPtr bq = newLucene<BooleanQuery>();
    bq->add(query, BooleanClause::MUST);

    QueryPtr rw2 = bq->rewrite(reader);

    EXPECT_EQ(getCount(reader, rw1), getCount(reader, rw2));
}
