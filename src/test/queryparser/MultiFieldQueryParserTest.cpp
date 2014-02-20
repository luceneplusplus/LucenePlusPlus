/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "Analyzer.h"
#include "LowerCaseTokenizer.h"
#include "TokenFilter.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "MultiFieldQueryParser.h"
#include "Query.h"
#include "StandardAnalyzer.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "Document.h"
#include "Field.h"
#include "IndexSearcher.h"
#include "ScoreDoc.h"
#include "TopDocs.h"

using namespace Lucene;

typedef LuceneTestFixture MultiFieldQueryParserTest;

DECLARE_SHARED_PTR(MultiFieldQueryParserTestAnalyzer)
DECLARE_SHARED_PTR(MultiFieldQueryParserTestFilter)

/// Filter which discards the token 'stop' and which expands the token 'phrase' into 'phrase1 phrase2'
class MultiFieldQueryParserTestFilter : public TokenFilter {
public:
    MultiFieldQueryParserTestFilter(const TokenStreamPtr& in) : TokenFilter(in) {
        termAtt = addAttribute<TermAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        inPhrase = false;
        savedStart = 0;
        savedEnd = 0;
    }

    virtual ~MultiFieldQueryParserTestFilter() {
    }

    LUCENE_CLASS(MultiFieldQueryParserTestFilter);

public:
    bool inPhrase;
    int32_t savedStart;
    int32_t savedEnd;
    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken() {
        if (inPhrase) {
            inPhrase = false;
            termAtt->setTermBuffer(L"phrase2");
            offsetAtt->setOffset(savedStart, savedEnd);
            return true;
        } else {
            while (input->incrementToken()) {
                if (termAtt->term() == L"phrase") {
                    inPhrase = true;
                    savedStart = offsetAtt->startOffset();
                    savedEnd = offsetAtt->endOffset();
                    termAtt->setTermBuffer(L"phrase1");
                    offsetAtt->setOffset(savedStart, savedEnd);
                    return true;
                } else if (termAtt->term() != L"stop") {
                    return true;
                }
            }
        }
        return false;
    }
};

class MultiFieldQueryParserTestAnalyzer : public Analyzer {
public:
    virtual ~MultiFieldQueryParserTestAnalyzer() {
    }

    LUCENE_CLASS(MultiFieldQueryParserTestAnalyzer);

public:
    // Filters LowerCaseTokenizer with StopFilter.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<MultiFieldQueryParserTestFilter>(newLucene<LowerCaseTokenizer>(reader));
    }
};

class EmptyTokenStream : public TokenStream {
public:
    virtual ~EmptyTokenStream() {
    }

    LUCENE_CLASS(EmptyTokenStream);

public:
    virtual bool incrementToken() {
        return false;
    }
};

/// Return empty tokens for field "f1".
class AnalyzerReturningNull : public Analyzer {
public:
    AnalyzerReturningNull() {
        standardAnalyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    }

    virtual ~AnalyzerReturningNull() {
    }

    LUCENE_CLASS(AnalyzerReturningNull);

protected:
    StandardAnalyzerPtr standardAnalyzer;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        if (fieldName == L"f1") {
            return newLucene<EmptyTokenStream>();
        } else {
            return standardAnalyzer->tokenStream(fieldName, reader);
        }
    }
};

/// verify parsing of query using a stopping analyzer
static void checkStopQueryEquals(const String& qtxt, const String& expectedRes) {
    Collection<String> fields = newCollection<String>(L"b", L"t");
    Collection<BooleanClause::Occur> occur = newCollection<BooleanClause::Occur>(BooleanClause::SHOULD, BooleanClause::SHOULD);
    MultiFieldQueryParserTestAnalyzerPtr a = newLucene<MultiFieldQueryParserTestAnalyzer>();
    MultiFieldQueryParserPtr mfqp = newLucene<MultiFieldQueryParser>(LuceneVersion::LUCENE_CURRENT, fields, a);

    QueryPtr q = mfqp->parse(qtxt);
    EXPECT_EQ(expectedRes, q->toString());

    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, qtxt, fields, occur, a);
    EXPECT_EQ(expectedRes, q->toString());
}

/// test stop words arising for both the non static form, and for the corresponding static form (qtxt, fields[]).
TEST_F(MultiFieldQueryParserTest, testStopwordsParsing) {
    checkStopQueryEquals(L"one", L"b:one t:one");
    checkStopQueryEquals(L"one stop", L"b:one t:one");
    checkStopQueryEquals(L"one (stop)", L"b:one t:one");
    checkStopQueryEquals(L"one ((stop))", L"b:one t:one");
    checkStopQueryEquals(L"stop", L"");
    checkStopQueryEquals(L"(stop)", L"");
    checkStopQueryEquals(L"((stop))", L"");
}

TEST_F(MultiFieldQueryParserTest, testSimple) {
    Collection<String> fields = newCollection<String>(L"b", L"t");
    MultiFieldQueryParserPtr mfqp = newLucene<MultiFieldQueryParser>(LuceneVersion::LUCENE_CURRENT, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));

    QueryPtr q = mfqp->parse(L"one");
    EXPECT_EQ(L"b:one t:one", q->toString());

    q = mfqp->parse(L"one two");
    EXPECT_EQ(L"(b:one t:one) (b:two t:two)", q->toString());

    q = mfqp->parse(L"+one +two");
    EXPECT_EQ(L"+(b:one t:one) +(b:two t:two)", q->toString());

    q = mfqp->parse(L"+one -two -three");
    EXPECT_EQ(L"+(b:one t:one) -(b:two t:two) -(b:three t:three)", q->toString());

    q = mfqp->parse(L"one^2 two");
    EXPECT_EQ(L"((b:one t:one)^2.0) (b:two t:two)", q->toString());

    q = mfqp->parse(L"one~ two");
    EXPECT_EQ(L"(b:one~0.5 t:one~0.5) (b:two t:two)", q->toString());

    q = mfqp->parse(L"one~0.8 two^2");
    EXPECT_EQ(L"(b:one~0.8 t:one~0.8) ((b:two t:two)^2.0)", q->toString());

    q = mfqp->parse(L"one* two*");
    EXPECT_EQ(L"(b:one* t:one*) (b:two* t:two*)", q->toString());

    q = mfqp->parse(L"[a TO c] two");
    EXPECT_EQ(L"(b:[a TO c] t:[a TO c]) (b:two t:two)", q->toString());

    q = mfqp->parse(L"w?ldcard");
    EXPECT_EQ(L"b:w?ldcard t:w?ldcard", q->toString());

    q = mfqp->parse(L"\"foo bar\"");
    EXPECT_EQ(L"b:\"foo bar\" t:\"foo bar\"", q->toString());

    q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"");
    EXPECT_EQ(L"(b:\"aa bb cc\" t:\"aa bb cc\") (b:\"dd ee\" t:\"dd ee\")", q->toString());

    q = mfqp->parse(L"\"foo bar\"~4");
    EXPECT_EQ(L"b:\"foo bar\"~4 t:\"foo bar\"~4", q->toString());

    q = mfqp->parse(L"b:\"foo bar\"~4");
    EXPECT_EQ(L"b:\"foo bar\"~4", q->toString());

    // make sure that terms which have a field are not touched
    q = mfqp->parse(L"one f:two");
    EXPECT_EQ(L"(b:one t:one) f:two", q->toString());

    // AND mode
    mfqp->setDefaultOperator(QueryParser::AND_OPERATOR);
    q = mfqp->parse(L"one two");
    EXPECT_EQ(L"+(b:one t:one) +(b:two t:two)", q->toString());
    q = mfqp->parse(L"\"aa bb cc\" \"dd ee\"");
    EXPECT_EQ(L"+(b:\"aa bb cc\" t:\"aa bb cc\") +(b:\"dd ee\" t:\"dd ee\")", q->toString());
}

TEST_F(MultiFieldQueryParserTest, testBoostsSimple) {
    MapStringDouble boosts = MapStringDouble::newInstance();
    boosts.put(L"b", 5.0);
    boosts.put(L"t", 10.0);
    Collection<String> fields = newCollection<String>(L"b", L"t");
    MultiFieldQueryParserPtr mfqp = newLucene<MultiFieldQueryParser>(LuceneVersion::LUCENE_CURRENT, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), boosts);

    // Check for simple
    QueryPtr q = mfqp->parse(L"one");
    EXPECT_EQ(L"b:one^5.0 t:one^10.0", q->toString());

    // Check for AND
    q = mfqp->parse(L"one AND two");
    EXPECT_EQ(L"+(b:one^5.0 t:one^10.0) +(b:two^5.0 t:two^10.0)", q->toString());

    // Check for OR
    q = mfqp->parse(L"one OR two");
    EXPECT_EQ(L"(b:one^5.0 t:one^10.0) (b:two^5.0 t:two^10.0)", q->toString());

    // Check for AND and a field
    q = mfqp->parse(L"one AND two AND foo:test");
    EXPECT_EQ(L"+(b:one^5.0 t:one^10.0) +(b:two^5.0 t:two^10.0) +foo:test", q->toString());

    q = mfqp->parse(L"one^3 AND two^4");
    EXPECT_EQ(L"+((b:one^5.0 t:one^10.0)^3.0) +((b:two^5.0 t:two^10.0)^4.0)", q->toString());
}

TEST_F(MultiFieldQueryParserTest, testStaticMethod1) {
    Collection<String> fields = newCollection<String>(L"b", L"t");
    Collection<String> queries = newCollection<String>(L"one", L"two");
    QueryPtr q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"b:one t:two", q->toString());

    Collection<String> queries2 = newCollection<String>(L"+one", L"+two");
    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries2, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"(+b:one) (+t:two)", q->toString());

    Collection<String> queries3 = newCollection<String>(L"one", L"+two");
    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries3, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"b:one (+t:two)", q->toString());

    Collection<String> queries4 = newCollection<String>(L"one +more", L"+two");
    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries4, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"(b:one +b:more) (+t:two)", q->toString());

    Collection<String> queries5 = newCollection<String>(L"blah");

    // expected exception, array length differs
    try {
        q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries5, fields, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }

    // check also with stop words for this static form (qtxts[], fields[]).
    MultiFieldQueryParserTestAnalyzerPtr stopA = newLucene<MultiFieldQueryParserTestAnalyzer>();

    Collection<String> queries6 = newCollection<String>(L"((+stop))", L"+((stop))");
    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries6, fields, stopA);
    EXPECT_EQ(L"", q->toString());

    Collection<String> queries7 = newCollection<String>(L"one ((+stop)) +more", L"+((stop)) +two");
    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries7, fields, stopA);
    EXPECT_EQ(L"(b:one +b:more) (+t:two)", q->toString());
}

TEST_F(MultiFieldQueryParserTest, testStaticMethod2) {
    Collection<String> fields = newCollection<String>(L"b", L"t");
    Collection<BooleanClause::Occur> flags = newCollection<BooleanClause::Occur>(BooleanClause::MUST, BooleanClause::MUST_NOT);
    QueryPtr q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, L"one", fields, flags, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"+b:one -t:one", q->toString());

    q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, L"one two", fields, flags, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"+(b:one b:two) -(t:one t:two)", q->toString());

    Collection<BooleanClause::Occur> flags2 = newCollection<BooleanClause::Occur>(BooleanClause::MUST);

    // expected exception, array length differs
    try {
        q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, L"blah", fields, flags2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
}

TEST_F(MultiFieldQueryParserTest, testStaticMethod3) {
    Collection<String> queries = newCollection<String>(L"one", L"two", L"three");
    Collection<String> fields = newCollection<String>(L"f1", L"f2", L"f3");
    Collection<BooleanClause::Occur> flags = newCollection<BooleanClause::Occur>(BooleanClause::MUST, BooleanClause::MUST_NOT, BooleanClause::SHOULD);
    QueryPtr q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries, fields, flags, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"+f1:one -f2:two f3:three", q->toString());

    Collection<BooleanClause::Occur> flags2 = newCollection<BooleanClause::Occur>(BooleanClause::MUST);

    // expected exception, array length differs
    try {
        q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries, fields, flags2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
}

TEST_F(MultiFieldQueryParserTest, testStaticMethod3Old) {
    Collection<String> queries = newCollection<String>(L"one", L"two");
    Collection<String> fields = newCollection<String>(L"b", L"t");
    Collection<BooleanClause::Occur> flags = newCollection<BooleanClause::Occur>(BooleanClause::MUST, BooleanClause::MUST_NOT);
    QueryPtr q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries, fields, flags, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    EXPECT_EQ(L"+b:one -t:two", q->toString());

    Collection<BooleanClause::Occur> flags2 = newCollection<BooleanClause::Occur>(BooleanClause::MUST);

    // expected exception, array length differs
    try {
        q = MultiFieldQueryParser::parse(LuceneVersion::LUCENE_CURRENT, queries, fields, flags2, newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    } catch (IllegalArgumentException& e) {
        EXPECT_TRUE(check_exception(LuceneException::IllegalArgument)(e));
    }
}

TEST_F(MultiFieldQueryParserTest, testAnalyzerReturningNull) {
    Collection<String> fields = newCollection<String>(L"f1", L"f2", L"f3");
    MultiFieldQueryParserPtr parser = newLucene<MultiFieldQueryParser>(LuceneVersion::LUCENE_CURRENT, fields, newLucene<AnalyzerReturningNull>());
    QueryPtr q = parser->parse(L"bla AND blo");
    EXPECT_EQ(L"+(f2:bla f3:bla) +(f2:blo f3:blo)", q->toString());

    // the following queries are not affected as their terms are not analyzed anyway
    q = parser->parse(L"bla*");
    EXPECT_EQ(L"f1:bla* f2:bla* f3:bla*", q->toString());
    q = parser->parse(L"bla~");
    EXPECT_EQ(L"f1:bla~0.5 f2:bla~0.5 f3:bla~0.5", q->toString());
    q = parser->parse(L"[a TO c]");
    EXPECT_EQ(L"f1:[a TO c] f2:[a TO c] f3:[a TO c]", q->toString());
}

TEST_F(MultiFieldQueryParserTest, testStopWordSearching) {
    AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    DirectoryPtr ramDir = newLucene<RAMDirectory>();
    IndexWriterPtr iw =  newLucene<IndexWriter>(ramDir, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"body", L"blah the footest blah", Field::STORE_NO, Field::INDEX_ANALYZED));
    iw->addDocument(doc);
    iw->close();

    MultiFieldQueryParserPtr mfqp = newLucene<MultiFieldQueryParser>(LuceneVersion::LUCENE_CURRENT, newCollection<String>(L"body"), analyzer);
    mfqp->setDefaultOperator(QueryParser::AND_OPERATOR);
    QueryPtr q = mfqp->parse(L"the footest");
    IndexSearcherPtr is = newLucene<IndexSearcher>(ramDir, true);
    Collection<ScoreDocPtr> hits = is->search(q, FilterPtr(), 1000)->scoreDocs;
    EXPECT_EQ(1, hits.size());
    is->close();
}
