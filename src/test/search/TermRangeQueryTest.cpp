/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "RAMDirectory.h"
#include "IndexSearcher.h"
#include "TermRangeQuery.h"
#include "WhitespaceAnalyzer.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "Collator.h"
#include "Tokenizer.h"
#include "TermAttribute.h"
#include "Reader.h"
#include "VariantUtils.h"

using namespace Lucene;

class SingleCharTokenizer : public Tokenizer {
public:
    SingleCharTokenizer(const ReaderPtr& r) : Tokenizer(r) {
        termAtt = addAttribute<TermAttribute>();
        buffer = CharArray::newInstance(1);
        done = false;
    }

    virtual ~SingleCharTokenizer() {
    }

public:
    CharArray buffer;
    bool done;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken() {
        int32_t count = input->read(buffer.get(), 0, 1);
        if (done) {
            return false;
        } else {
            clearAttributes();
            done = true;
            if (count == 1) {
                termAtt->termBuffer()[0] = buffer[0];
                termAtt->setTermLength(1);
            } else {
                termAtt->setTermLength(0);
            }
            return true;
        }
    }

    virtual void reset(const ReaderPtr& input) {
        Tokenizer::reset(input);
        done = false;
    }
};

class SingleCharAnalyzer : public Analyzer {
public:
    virtual ~SingleCharAnalyzer() {
    }

public:
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenizerPtr tokenizer = boost::dynamic_pointer_cast<Tokenizer>(getPreviousTokenStream());
        if (!tokenizer) {
            tokenizer = newLucene<SingleCharTokenizer>(reader);
            setPreviousTokenStream(tokenizer);
        } else {
            tokenizer->reset(reader);
        }
        return tokenizer;
    }

    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<SingleCharTokenizer>(reader);
    }
};

class TermRangeQueryTest : public LuceneTestFixture {
public:
    TermRangeQueryTest() {
        docCount = 0;
        dir = newLucene<RAMDirectory>();
    }

    virtual ~TermRangeQueryTest() {
    }

protected:
    int32_t docCount;
    RAMDirectoryPtr dir;

public:
    void initializeIndex(Collection<String> values) {
        initializeIndex(values, newLucene<WhitespaceAnalyzer>());
    }

    void initializeIndex(Collection<String> values, const AnalyzerPtr& analyzer) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
        for (int32_t i = 0; i < values.size(); ++i) {
            insertDoc(writer, values[i]);
        }
        writer->close();
    }

    void addDoc(const String& content) {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, newLucene<WhitespaceAnalyzer>(), false, IndexWriter::MaxFieldLengthLIMITED);
        insertDoc(writer, content);
        writer->close();
    }

    void insertDoc(const IndexWriterPtr& writer, const String& content) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(L"id", L"id" + StringUtils::toString(docCount), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Field>(L"content", content, Field::STORE_NO, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
        ++docCount;
    }
};

TEST_F(TermRangeQueryTest, testExclusive) {
    QueryPtr query = newLucene<TermRangeQuery>(L"content", L"A", L"C", false, false);
    initializeIndex(newCollection<String>(L"A", L"B", L"C", L"D"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"D"));
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();
}

TEST_F(TermRangeQueryTest, testInclusive) {
    QueryPtr query = newLucene<TermRangeQuery>(L"content", L"A", L"C", true, true);

    initializeIndex(newCollection<String>(L"A", L"B", L"C", L"D"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"D"));
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    searcher->close();
}

TEST_F(TermRangeQueryTest, testEqualsHashcode) {
    QueryPtr query = newLucene<TermRangeQuery>(L"content", L"A", L"C", true, true);

    query->setBoost(1.0);
    QueryPtr other = newLucene<TermRangeQuery>(L"content", L"A", L"C", true, true);
    other->setBoost(1.0);

    EXPECT_TRUE(query->equals(query));
    EXPECT_TRUE(query->equals(other));
    EXPECT_EQ(query->hashCode(), other->hashCode());

    other->setBoost(2.0);
    EXPECT_TRUE(!query->equals(other));

    other = newLucene<TermRangeQuery>(L"notcontent", L"A", L"C", true, true);
    EXPECT_TRUE(!query->equals(other));

    other = newLucene<TermRangeQuery>(L"content", L"X", L"C", true, true);
    EXPECT_TRUE(!query->equals(other));

    other = newLucene<TermRangeQuery>(L"content", L"A", L"Z", true, true);
    EXPECT_TRUE(!query->equals(other));

    query = newLucene<TermRangeQuery>(L"content", L"", L"C", true, true);
    other = newLucene<TermRangeQuery>(L"content", L"", L"C", true, true);
    EXPECT_TRUE(query->equals(other));
    EXPECT_EQ(query->hashCode(), other->hashCode());

    query = newLucene<TermRangeQuery>(L"content", L"C", L"", true, true);
    other = newLucene<TermRangeQuery>(L"content", L"C", L"", true, true);
    EXPECT_TRUE(query->equals(other));
    EXPECT_EQ(query->hashCode(), other->hashCode());

    query = newLucene<TermRangeQuery>(L"content", L"", L"C", true, true);
    other = newLucene<TermRangeQuery>(L"content", L"C", L"", true, true);
    EXPECT_TRUE(!query->equals(other));

    query = newLucene<TermRangeQuery>(L"content", L"A", L"C", false, false);
    other = newLucene<TermRangeQuery>(L"content", L"A", L"C", true, true);
    EXPECT_TRUE(!query->equals(other));

    query = newLucene<TermRangeQuery>(L"content", L"A", L"C", false, false);
    other = newLucene<TermRangeQuery>(L"content", L"A", L"C", false, false, newLucene<Collator>(std::locale()));
    EXPECT_TRUE(!query->equals(other));
}

TEST_F(TermRangeQueryTest, testExclusiveCollating) {
    QueryPtr query = newLucene<TermRangeQuery>(L"content", L"A", L"C", false, false, newLucene<Collator>(std::locale()));
    initializeIndex(newCollection<String>(L"A", L"B", L"C", L"D"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"D"));
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    searcher->close();
}

TEST_F(TermRangeQueryTest, testInclusiveCollating) {
    QueryPtr query = newLucene<TermRangeQuery>(L"content", L"A", L"C", true, true, newLucene<Collator>(std::locale()));
    initializeIndex(newCollection<String>(L"A", L"B", L"C", L"D"));
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    Collection<ScoreDocPtr> hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"D"));
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(2, hits.size());
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    hits = searcher->search(query, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(3, hits.size());
    searcher->close();
}

TEST_F(TermRangeQueryTest, testExclusiveLowerNull) {
    AnalyzerPtr analyzer = newLucene<SingleCharAnalyzer>();
    QueryPtr query = newLucene<TermRangeQuery>(L"content", VariantUtils::null(), L"C", false, false);
    initializeIndex(newCollection<String>(L"A", L"B", L"", L"C", L"D"), analyzer);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    int32_t numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(3, numHits);
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"", L"D"), analyzer);
    searcher = newLucene<IndexSearcher>(dir, true);
    numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(3, numHits);
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(3, numHits);
    searcher->close();
}

TEST_F(TermRangeQueryTest, testInclusiveLowerNull) {
    AnalyzerPtr analyzer = newLucene<SingleCharAnalyzer>();
    QueryPtr query = newLucene<TermRangeQuery>(L"content", VariantUtils::null(), L"C", true, true);
    initializeIndex(newCollection<String>(L"A", L"B", L"", L"C", L"D"), analyzer);
    IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
    int32_t numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(4, numHits);
    searcher->close();

    initializeIndex(newCollection<String>(L"A", L"B", L"", L"D"), analyzer);
    searcher = newLucene<IndexSearcher>(dir, true);
    numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(3, numHits);
    searcher->close();

    addDoc(L"C");
    searcher = newLucene<IndexSearcher>(dir, true);
    numHits = searcher->search(query, FilterPtr(), 1000)->totalHits;
    EXPECT_EQ(4, numHits);
    searcher->close();
}
