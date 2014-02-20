/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include <boost/algorithm/string.hpp>
#include "BaseTokenStreamFixture.h"
#include "Highlighter.h"
#include "RAMDirectory.h"
#include "IndexWriter.h"
#include "IndexReader.h"
#include "IndexSearcher.h"
#include "StandardAnalyzer.h"
#include "Document.h"
#include "NumericField.h"
#include "SimpleAnalyzer.h"
#include "QueryParser.h"
#include "TopDocs.h"
#include "QueryScorer.h"
#include "TokenStream.h"
#include "SimpleFragmenter.h"
#include "SimpleSpanFragmenter.h"
#include "SimpleHTMLFormatter.h"
#include "StringReader.h"
#include "TokenSources.h"
#include "MultiTermQuery.h"
#include "WhitespaceAnalyzer.h"
#include "TokenGroup.h"
#include "NumericRangeQuery.h"
#include "PhraseQuery.h"
#include "MultiPhraseQuery.h"
#include "SpanNearQuery.h"
#include "SpanNotQuery.h"
#include "SpanTermQuery.h"
#include "QueryTermScorer.h"
#include "WeightedSpanTerm.h"
#include "WeightedTerm.h"
#include "BooleanQuery.h"
#include "WildcardQuery.h"
#include "NullFragmenter.h"
#include "TermRangeFilter.h"
#include "LowerCaseTokenizer.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "TextFragment.h"
#include "SimpleHTMLEncoder.h"
#include "MultiSearcher.h"
#include "ScoreDoc.h"
#include "Term.h"
#include "FilteredQuery.h"
#include "Token.h"
#include "TermQuery.h"

using namespace Lucene;
class HighlighterTest;

namespace HighlighterTestNS {

class TestFormatter : public Formatter, public LuceneObject {
public:
    TestFormatter(HighlighterTest* fixture);
    virtual ~TestFormatter();

    LUCENE_CLASS(TestFormatter);

protected:
    HighlighterTest* fixture;

public:
    virtual String highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup);
};

}

class HighlighterTest : public BaseTokenStreamFixture {
public:
    HighlighterTest() {
        numHighlights = 0;
        analyzer = newLucene<StandardAnalyzer>(TEST_VERSION);
        texts = newCollection<String>(
                    L"Hello this is a piece of text that is very long and contains too much preamble and the meat is really here which says kennedy has been shot",
                    L"This piece of text refers to Kennedy at the beginning then has a longer piece of text that is very long in the middle and finally ends with another reference to Kennedy",
                    L"JFK has been shot",
                    L"John Kennedy has been shot",
                    L"This text has a typo in referring to Keneddy",
                    L"wordx wordy wordz wordx wordy wordx worda wordb wordy wordc",
                    L"y z x y z a b",
                    L"lets is a the lets is a the lets is a the lets"
                );

        ramDir = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(ramDir, newLucene<StandardAnalyzer>(TEST_VERSION), true, IndexWriter::MaxFieldLengthUNLIMITED);
        for (int32_t i = 0; i < texts.size(); ++i) {
            addDoc(writer, texts[i]);
        }
        DocumentPtr doc = newLucene<Document>();
        NumericFieldPtr nfield = newLucene<NumericField>(NUMERIC_FIELD_NAME, Field::STORE_YES, true);
        nfield->setIntValue(1);
        doc->add(nfield);
        writer->addDocument(doc, analyzer);
        nfield = newLucene<NumericField>(NUMERIC_FIELD_NAME, Field::STORE_YES, true);
        nfield->setIntValue(3);
        doc = newLucene<Document>();
        doc->add(nfield);
        writer->addDocument(doc, analyzer);
        nfield = newLucene<NumericField>(NUMERIC_FIELD_NAME, Field::STORE_YES, true);
        nfield->setIntValue(5);
        doc = newLucene<Document>();
        doc->add(nfield);
        writer->addDocument(doc, analyzer);
        nfield = newLucene<NumericField>(NUMERIC_FIELD_NAME, Field::STORE_YES, true);
        nfield->setIntValue(7);
        doc = newLucene<Document>();
        doc->add(nfield);
        writer->addDocument(doc, analyzer);
        writer->optimize();
        writer->close();
        reader = IndexReader::open(ramDir, true);

        dir = newLucene<RAMDirectory>();
        a = newLucene<WhitespaceAnalyzer>();
    }

    virtual ~HighlighterTest() {
    }

public:
    IndexReaderPtr reader;
    QueryPtr query;
    RAMDirectoryPtr ramDir;
    IndexSearcherPtr searcher;

public:
    int32_t numHighlights;
    AnalyzerPtr analyzer;
    TopDocsPtr hits;

    Collection<String> texts;

    DirectoryPtr dir;
    AnalyzerPtr a;

    static const LuceneVersion::Version TEST_VERSION;
    static const String FIELD_NAME;
    static const String NUMERIC_FIELD_NAME;

public:
    void addDoc(const IndexWriterPtr& writer, const String& text) {
        DocumentPtr doc = newLucene<Document>();
        FieldPtr field = newLucene<Field>(FIELD_NAME, text, Field::STORE_YES, Field::INDEX_ANALYZED);
        doc->add(field);
        writer->addDocument(doc);
    }

    String highlightField(const QueryPtr& query, const String& fieldName, const String& text) {
        TokenStreamPtr tokenStream = newLucene<StandardAnalyzer>(TEST_VERSION)->tokenStream(fieldName, newLucene<StringReader>(text));
        // Assuming "<B>", "</B>" used to highlight
        SimpleHTMLFormatterPtr formatter = newLucene<SimpleHTMLFormatter>();
        QueryScorerPtr scorer = newLucene<QueryScorer>(query, fieldName, FIELD_NAME);
        HighlighterPtr highlighter = newLucene<Highlighter>(formatter, scorer);
        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(INT_MAX));

        String rv = highlighter->getBestFragments(tokenStream, text, 1, L"(FIELD TEXT TRUNCATED)");
        return rv.empty() ? text : rv;
    }

    void doSearching(const String& queryString) {
        QueryParserPtr parser = newLucene<QueryParser>(TEST_VERSION, FIELD_NAME, analyzer);
        parser->setEnablePositionIncrements(true);
        parser->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
        query = parser->parse(queryString);
        doSearching(query);
    }

    void doSearching(const QueryPtr& unReWrittenQuery) {
        searcher = newLucene<IndexSearcher>(ramDir, true);
        // for any multi-term queries to work (prefix, wildcard, range,fuzzy etc) you must use a rewritten query
        query = unReWrittenQuery->rewrite(reader);
        hits = searcher->search(query, FilterPtr(), 1000);
    }

    void checkExpectedHighlightCount(int32_t maxNumFragmentsRequired, int32_t expectedHighlights, Collection<String> expected) {
        Collection<String> results = Collection<String>::newInstance();

        for (int32_t i = 0; i < hits->totalHits; ++i) {
            String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
            TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
            QueryScorerPtr scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
            HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

            highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

            results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));

            EXPECT_EQ(numHighlights, expectedHighlights);
        }

        EXPECT_EQ(results.size(), expected.size());
        for (int32_t i = 0; i < results.size(); ++i) {
            EXPECT_EQ(results[i], expected[i]);
        }
    }

    void makeIndex() {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, a, IndexWriter::MaxFieldLengthLIMITED);
        writer->addDocument(doc(L"t_text1", L"random words for highlighting tests del"));
        writer->addDocument(doc(L"t_text1", L"more random words for second field del"));
        writer->addDocument(doc(L"t_text1", L"random words for highlighting tests del"));
        writer->addDocument(doc(L"t_text1", L"more random words for second field"));
        writer->optimize();
        writer->close();
    }

    DocumentPtr doc(const String& f, const String& v) {
        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(f, v, Field::STORE_YES, Field::INDEX_ANALYZED));
        return doc;
    }

    void deleteDocument() {
        IndexWriterPtr writer = newLucene<IndexWriter>(dir, a, false, IndexWriter::MaxFieldLengthLIMITED);
        writer->deleteDocuments(newLucene<Term>(L"t_text1", L"del"));
        writer->close();
    }

    void searchIndex() {
        String q = L"t_text1:random";
        QueryParserPtr parser = newLucene<QueryParser>(TEST_VERSION, L"t_text1", a );
        QueryPtr query = parser->parse(q);
        IndexSearcherPtr searcher = newLucene<IndexSearcher>(dir, true);
        // This scorer can return negative idf -> null fragment
        HighlighterScorerPtr scorer = newLucene<QueryTermScorer>(query, searcher->getIndexReader(), L"t_text1");
        HighlighterPtr h = newLucene<Highlighter>(scorer);

        TopDocsPtr hits = searcher->search(query, FilterPtr(), 10);
        for (int32_t i = 0; i < hits->totalHits; ++i) {
            DocumentPtr doc = searcher->doc(hits->scoreDocs[i]->doc);
            String result = h->getBestFragment(a, L"t_text1", doc->get(L"t_text1"));
            EXPECT_EQ(L"more <B>random</B> words for second field", result);
        }
        searcher->close();
    }
};

const LuceneVersion::Version HighlighterTest::TEST_VERSION = LuceneVersion::LUCENE_CURRENT;
const String HighlighterTest::FIELD_NAME = L"contents";
const String HighlighterTest::NUMERIC_FIELD_NAME = L"nfield";

namespace HighlighterTestNS {

TestFormatter::TestFormatter(HighlighterTest* fixture) {
    this->fixture = fixture;
}

TestFormatter::~TestFormatter() {
}

String TestFormatter::highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup) {
    if (tokenGroup->getTotalScore() <= 0) {
        return originalText;
    }
    ++fixture->numHighlights; // update stats used in assertions
    return L"<B>" + originalText + L"</B>";
}

DECLARE_SHARED_PTR(TestHighlightRunner)

class TestHighlightRunner : public LuceneObject {
public:
    TestHighlightRunner(HighlighterTest* fixture) {
        this->fixture = fixture;
        mode = QUERY;
        frag = newLucene<SimpleFragmenter>(20);
    }

    virtual ~TestHighlightRunner() {
    }

    LUCENE_CLASS(TestHighlightRunner);

protected:
    HighlighterTest* fixture;

    static const int32_t QUERY;
    static const int32_t QUERY_TERM;

public:
    int32_t mode;
    FragmenterPtr frag;

public:
    virtual HighlighterPtr getHighlighter(const QueryPtr& query, const String& fieldName, const TokenStreamPtr& stream, const FormatterPtr& formatter) {
        return getHighlighter(query, fieldName, stream, formatter, true);
    }

    virtual HighlighterPtr getHighlighter(const QueryPtr& query, const String& fieldName, const TokenStreamPtr& stream, const FormatterPtr& formatter, bool expanMultiTerm) {
        HighlighterScorerPtr scorer;
        if (mode == QUERY) {
            scorer = newLucene<QueryScorer>(query, fieldName);
            if (!expanMultiTerm) {
                boost::dynamic_pointer_cast<QueryScorer>(scorer)->setExpandMultiTermQuery(false);
            }
        } else if (mode == QUERY_TERM) {
            scorer = newLucene<QueryTermScorer>(query);
        } else {
            boost::throw_exception(IllegalArgumentException(L"Unknown highlight mode"));
        }

        return newLucene<Highlighter>(formatter, scorer);
    }

    virtual HighlighterPtr getHighlighter(Collection<WeightedTermPtr> weightedTerms, const FormatterPtr& formatter) {
        if (mode == QUERY) {
            Collection<WeightedSpanTermPtr> weightedSpanTerms = Collection<WeightedSpanTermPtr>::newInstance(weightedTerms.size());
            for (int32_t i = 0; i < weightedTerms.size(); ++i) {
                weightedSpanTerms[i] = boost::dynamic_pointer_cast<WeightedSpanTerm>(weightedTerms[i]);
            }
            return newLucene<Highlighter>(formatter, newLucene<QueryScorer>(weightedSpanTerms));
        } else if (mode == QUERY_TERM) {
            return newLucene<Highlighter>(formatter, newLucene<QueryTermScorer>(weightedTerms));
        } else {
            boost::throw_exception(IllegalArgumentException(L"Unknown highlight mode"));
        }
        return HighlighterPtr();
    }

    virtual void doStandardHighlights(const AnalyzerPtr& analyzer, const IndexSearcherPtr& searcher, const TopDocsPtr& hits, const QueryPtr& query, const FormatterPtr& formatter, Collection<String> expected, bool expandMT = false) {
        Collection<String> results = Collection<String>::newInstance();

        for (int32_t i = 0; i < hits->totalHits; ++i) {
            String text = searcher->doc(hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            int32_t maxNumFragmentsRequired = 2;
            String fragmentSeparator = L"...";
            HighlighterScorerPtr scorer;
            TokenStreamPtr tokenStream = analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            if (mode == QUERY) {
                scorer = newLucene<QueryScorer>(query);
            } else if (mode == QUERY_TERM) {
                scorer = newLucene<QueryTermScorer>(query);
            }
            HighlighterPtr highlighter = newLucene<Highlighter>(formatter, scorer);
            highlighter->setTextFragmenter(frag);
            results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, fragmentSeparator));
        }

        EXPECT_EQ(results.size(), expected.size());
        for (int32_t i = 0; i < results.size(); ++i) {
            EXPECT_EQ(results[i], expected[i]);
        }
    }

    virtual void run(Collection<String> expected) = 0;

    virtual void start(Collection<String> expected = Collection<String>()) {
        run(expected);
        mode = QUERY_TERM;
        run(expected);
    }
};

const int32_t TestHighlightRunner::QUERY = 0;
const int32_t TestHighlightRunner::QUERY_TERM = 1;
}

TEST_F(HighlighterTest, testQueryScorerHits) {
    AnalyzerPtr analyzer = newLucene<SimpleAnalyzer>();
    QueryParserPtr qp = newLucene<QueryParser>(TEST_VERSION, FIELD_NAME, analyzer);
    query = qp->parse(L"\"very long\"");
    searcher = newLucene<IndexSearcher>(ramDir, true);
    TopDocsPtr hits = searcher->search(query, 10);

    QueryScorerPtr scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->scoreDocs.size(); ++i) {
        DocumentPtr doc = searcher->doc(hits->scoreDocs[i]->doc);
        String storedField = doc->get(FIELD_NAME);

        TokenStreamPtr stream = TokenSources::getAnyTokenStream(searcher->getIndexReader(), hits->scoreDocs[i]->doc, FIELD_NAME, doc, analyzer);
        FragmenterPtr fragmenter = newLucene<SimpleSpanFragmenter>(scorer);

        highlighter->setTextFragmenter(fragmenter);

        results.add(highlighter->getBestFragment(stream, storedField));
    }

    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], L"Hello this is a piece of text that is <B>very</B> <B>long</B> and contains too much preamble and the meat is really here which says kennedy has been shot");
    EXPECT_EQ(results[1], L"This piece of text refers to Kennedy at the beginning then has a longer piece of text that is <B>very</B>");
}

TEST_F(HighlighterTest, testHighlightingWithDefaultField) {
    String s1 = L"I call our world Flatland, not because we call it so,";

    QueryParserPtr parser = newLucene<QueryParser>(TEST_VERSION, FIELD_NAME, newLucene<StandardAnalyzer>(TEST_VERSION));

    // Verify that a query against the default field results in text being highlighted regardless of the field name.
    QueryPtr q = parser->parse(L"\"world Flatland\"~3");
    String expected = L"I call our <B>world</B> <B>Flatland</B>, not because we call it so,";

    String observed = highlightField(q, L"SOME_FIELD_NAME", s1);
    EXPECT_EQ(expected, observed);

    // Verify that a query against a named field does not result in any ighlighting when the query field name differs
    // from the name of the field being highlighted, which in this example happens to be the default field name.
    q = parser->parse(L"text:\"world Flatland\"~3");
    expected = s1;
    observed = highlightField(q, FIELD_NAME, s1);
    EXPECT_EQ(s1, highlightField(q, FIELD_NAME, s1));
}

TEST_F(HighlighterTest, testSimpleSpanHighlighter) {
    doSearching(L"Kennedy");

    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
    EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B>... to <B>Kennedy</B>");
    EXPECT_EQ(results[2], L" <B>kennedy</B> has been shot");
}

TEST_F(HighlighterTest, testRepeatingTermsInMultBooleans) {
    String content = L"x y z a b c d e f g b c g";
    String ph1 = L"\"a b c d\"";
    String ph2 = L"\"b c g\"";
    String f1 = L"f1";
    String f2 = L"f2";
    String f1c = f1 + L":";
    String f2c = f2 + L":";
    String q = L"(" + f1c + ph1 + L" OR " + f2c + ph1 + L") AND (" + f1c + ph2 + L" OR " + f2c + ph2 + L")";
    AnalyzerPtr analyzer = newLucene<WhitespaceAnalyzer>();
    QueryParserPtr qp = newLucene<QueryParser>(TEST_VERSION, f1, analyzer);
    QueryPtr query = qp->parse(q);

    QueryScorerPtr scorer = newLucene<QueryScorer>(query, f1);
    scorer->setExpandMultiTermQuery(false);

    HighlighterPtr h = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

    h->getBestFragment(analyzer, f1, content);

    EXPECT_EQ(numHighlights, 7);
}

TEST_F(HighlighterTest, testSimpleQueryScorerPhraseHighlighting) {
    doSearching(L"\"very long and contains\"");

    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], L"Hello this is a piece of text that is <B>very</B> <B>long</B> and <B>contains</B> too much preamble");

    EXPECT_EQ(numHighlights, 3);

    numHighlights = 0;
    doSearching(L"\"This piece of text refers to Kennedy\"");

    maxNumFragmentsRequired = 2;

    scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], L"This <B>piece</B> of <B>text</B> <B>refers</B> to <B>Kennedy</B> at the beginning then has a longer piece");

    EXPECT_EQ(numHighlights, 4);

    numHighlights = 0;
    doSearching(L"\"lets is a the lets is a the lets is a the lets\"");

    maxNumFragmentsRequired = 2;

    scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], L"<B>lets</B> is a the <B>lets</B> is a the <B>lets</B> is a the <B>lets</B>");

    EXPECT_EQ(numHighlights, 4);
}

TEST_F(HighlighterTest, testSpanRegexQuery) {
    // todo
}

TEST_F(HighlighterTest, testRegexQuery) {
    // todo
}

TEST_F(HighlighterTest, testNumericRangeQuery) {
    // doesn't currently highlight, but make sure it doesn't cause exception either
    query = NumericRangeQuery::newIntRange(NUMERIC_FIELD_NAME, 2, 6, true, true);
    searcher = newLucene<IndexSearcher>(ramDir, true);
    hits = searcher->search(query, 100);
    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer = newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(NUMERIC_FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], L"");
    EXPECT_EQ(results[1], L"");

    EXPECT_EQ(numHighlights, 0);
}

TEST_F(HighlighterTest, testSimpleQueryScorerPhraseHighlighting2) {
    doSearching(L"\"text piece long\"~5");

    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], L"Hello this is a <B>piece</B> of <B>text</B> that is very <B>long</B> and contains too much preamble");
    EXPECT_EQ(results[1], L" at the beginning then has a longer <B>piece</B> of <B>text</B> that is very <B>long</B> in the middle");

    EXPECT_EQ(numHighlights, 6);
}

TEST_F(HighlighterTest, testSimpleQueryScorerPhraseHighlighting3) {
    doSearching(L"\"x y z\"");

    int32_t maxNumFragmentsRequired = 2;
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        QueryScorerPtr scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
        HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));

        EXPECT_EQ(numHighlights, 3);
    }

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], L"y z <B>x</B> <B>y</B> <B>z</B> a b");
}

TEST_F(HighlighterTest, testSimpleSpanFragmenter) {
    doSearching(L"\"piece of text that is very long\"");

    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleSpanFragmenter>(scorer, 5));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], L" this is a <B>piece</B> of <B>text</B>");
    EXPECT_EQ(results[1], L" <B>piece</B> of <B>text</B> that is <B>very</B> <B>long</B>");

    doSearching(L"\"been shot\"");

    maxNumFragmentsRequired = 2;

    scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
    highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleSpanFragmenter>(scorer, 20));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(numHighlights, 14);

    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], L"JFK has <B>been</B> <B>shot</B>");
    EXPECT_EQ(results[1], L"John Kennedy has <B>been</B> <B>shot</B>");
    EXPECT_EQ(results[2], L" kennedy has <B>been</B> <B>shot</B>");
}

/// position sensitive query added after position insensitive query
TEST_F(HighlighterTest, testPosTermStdTerm) {
    doSearching(L"y \"x y z\"");

    int32_t maxNumFragmentsRequired = 2;

    QueryScorerPtr scorer =  newLucene<QueryScorer>(query, FIELD_NAME);
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));

        EXPECT_EQ(numHighlights, 4);
    }

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0], L"<B>y</B> z <B>x</B> <B>y</B> <B>z</B> a b");
}

TEST_F(HighlighterTest, testQueryScorerMultiPhraseQueryHighlighting) {
    MultiPhraseQueryPtr mpq = newLucene<MultiPhraseQuery>();

    mpq->add(newCollection<TermPtr>(newLucene<Term>(FIELD_NAME, L"wordx"), newLucene<Term>(FIELD_NAME, L"wordb")));
    mpq->add(newLucene<Term>(FIELD_NAME, L"wordy"));

    doSearching(mpq);

    int32_t maxNumFragmentsRequired = 2;
    Collection<String> expected = newCollection<String>(L"<B>wordx</B> <B>wordy</B> wordz <B>wordx</B> <B>wordy</B> wordx worda <B>wordb</B> <B>wordy</B> wordc");

    checkExpectedHighlightCount(maxNumFragmentsRequired, 6, expected);
}

TEST_F(HighlighterTest, testQueryScorerMultiPhraseQueryHighlightingWithGap) {
    MultiPhraseQueryPtr mpq = newLucene<MultiPhraseQuery>();

    // The toString of MultiPhraseQuery doesn't work so well with these out-of-order additions, but the Query itself seems to match accurately.

    mpq->add(newCollection<TermPtr>(newLucene<Term>(FIELD_NAME, L"wordz")), 2);
    mpq->add(newCollection<TermPtr>(newLucene<Term>(FIELD_NAME, L"wordx")), 0);

    doSearching(mpq);

    int32_t maxNumFragmentsRequired = 1;
    int32_t expectedHighlights = 2;

    Collection<String> expected = newCollection<String>(L"<B>wordx</B> wordy <B>wordz</B> wordx wordy wordx");

    checkExpectedHighlightCount(maxNumFragmentsRequired, expectedHighlights, expected);
}

namespace TestNearSpanSimpleQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        mode = QUERY;
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
    }
};

}

TEST_F(HighlighterTest, testNearSpanSimpleQuery) {
    doSearching(newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(
            newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"beginning")),
            newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"kennedy"))), 3, false));

    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestNearSpanSimpleQuery::HelperHighlightRunner>(this);

    Collection<String> expected = newCollection<String>(L" refers to <B>Kennedy</B> at the <B>beginning</B>");
    helper->run(expected);

    EXPECT_EQ(numHighlights, 2);
}

TEST_F(HighlighterTest, testSimpleQueryTermScorerHighlighter) {
    doSearching(L"Kennedy");
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<QueryTermScorer>(query));
    highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

    int32_t maxNumFragmentsRequired = 2;
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"..."));
    }

    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
    EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B>... to <B>Kennedy</B>");
    EXPECT_EQ(results[2], L" <B>kennedy</B> has been shot");
}

namespace TestSpanHighlighting {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        mode = QUERY;
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
    }
};

}

TEST_F(HighlighterTest, testSpanHighlighting) {
    QueryPtr query1 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(
                          newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"wordx")),
                          newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"wordy"))), 1, false);
    QueryPtr query2 = newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(
                          newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"wordy")),
                          newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"wordc"))), 1, false);
    BooleanQueryPtr bquery = newLucene<BooleanQuery>();
    bquery->add(query1, BooleanClause::SHOULD);
    bquery->add(query2, BooleanClause::SHOULD);
    doSearching(bquery);

    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestSpanHighlighting::HelperHighlightRunner>(this);

    Collection<String> expected = newCollection<String>(L"<B>wordx</B> <B>wordy</B> wordz <B>wordx</B> <B>wordy</B> <B>wordx</B>");
    helper->run(expected);

    EXPECT_EQ(numHighlights, 7);
}

namespace TestNotSpanSimpleQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        mode = QUERY;
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
    }
};

}

TEST_F(HighlighterTest, testNotSpanSimpleQuery) {
    doSearching(newLucene<SpanNotQuery>(newLucene<SpanNearQuery>(newCollection<SpanQueryPtr>(
                                            newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"shot")),
                                            newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"kennedy"))), 3, false),
                                        newLucene<SpanTermQuery>(newLucene<Term>(FIELD_NAME, L"john"))));

    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestNotSpanSimpleQuery::HelperHighlightRunner>(this);

    Collection<String> expected = newCollection<String>(
                                      L"John <B>Kennedy</B> has been <B>shot</B>",
                                      L" <B>kennedy</B> has been <B>shot</B>"
                                  );
    helper->run(expected);

    EXPECT_EQ(numHighlights, 4);
}

namespace TestGetBestFragmentsSimpleQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"Kennedy");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 4);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsSimpleQuery) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsSimpleQuery::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

namespace TestGetFuzzyFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"Kinnedy~");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected, true);
        EXPECT_EQ(fixture->numHighlights, 5);
    }
};

}

TEST_F(HighlighterTest, testGetFuzzyFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetFuzzyFragments::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot",
            L" to <B>Keneddy</B>"
        )
    );
}

namespace TestGetWildCardFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"K?nnedy");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 4);
    }
};

}

TEST_F(HighlighterTest, testGetWildCardFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetWildCardFragments::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

namespace TestGetMidWildCardFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"K*dy");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 5);
    }
};

}

TEST_F(HighlighterTest, testGetMidWildCardFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetMidWildCardFragments::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L" to <B>Keneddy</B>",
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

namespace TestGetRangeFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        String queryString = HighlighterTest::FIELD_NAME + L":[kannedy TO kznnedy]";

        // Need to explicitly set the QueryParser property to use TermRangeQuery rather than RangeFilters
        QueryParserPtr parser = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, HighlighterTest::FIELD_NAME, fixture->analyzer);
        parser->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
        fixture->query = parser->parse(queryString);
        fixture->doSearching(fixture->query);

        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 5);
    }
};

}

TEST_F(HighlighterTest, testGetRangeFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetRangeFragments::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L" to <B>Keneddy</B>",
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

TEST_F(HighlighterTest, testConstantScoreMultiTermQuery) {
    numHighlights = 0;

    query = newLucene<WildcardQuery>(newLucene<Term>(FIELD_NAME, L"ken*"));
    boost::dynamic_pointer_cast<WildcardQuery>(query)->setRewriteMethod(MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE());
    searcher = newLucene<IndexSearcher>(ramDir, true);
    // can't rewrite ConstantScore if you want to highlight it - it rewrites to ConstantScoreQuery which cannot be highlighted
    // query = unReWrittenQuery.rewrite(reader);
    hits = searcher->search(query, FilterPtr(), 1000);

    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        int32_t maxNumFragmentsRequired = 2;
        String fragmentSeparator = L"...";

        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        QueryScorerPtr scorer = newLucene<QueryScorer>(query, FIELD_NAME);

        HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(20));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, fragmentSeparator));
    }

    EXPECT_EQ(numHighlights, 5);

    EXPECT_EQ(results.size(), 4);
    EXPECT_EQ(results[0], L" <B>kennedy</B> has been shot");
    EXPECT_EQ(results[1], L" refers to <B>Kennedy</B>... to <B>Kennedy</B>");
    EXPECT_EQ(results[2], L"John <B>Kennedy</B> has been shot");
    EXPECT_EQ(results[3], L" to <B>Keneddy</B>");

    // try null field

    hits = searcher->search(query, FilterPtr(), 1000);

    numHighlights = 0;

    results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        int32_t maxNumFragmentsRequired = 2;
        String fragmentSeparator = L"...";

        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        QueryScorerPtr scorer = newLucene<QueryScorer>(query, L"");

        HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(20));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, fragmentSeparator));
    }

    EXPECT_EQ(numHighlights, 5);

    EXPECT_EQ(results.size(), 4);
    EXPECT_EQ(results[0], L" <B>kennedy</B> has been shot");
    EXPECT_EQ(results[1], L" refers to <B>Kennedy</B>... to <B>Kennedy</B>");
    EXPECT_EQ(results[2], L"John <B>Kennedy</B> has been shot");
    EXPECT_EQ(results[3], L" to <B>Keneddy</B>");

    // try default field

    hits = searcher->search(query, FilterPtr(), 1000);

    numHighlights = 0;

    results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = searcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        int32_t maxNumFragmentsRequired = 2;
        String fragmentSeparator = L"...";

        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        QueryScorerPtr scorer = newLucene<QueryScorer>(query, L"random_field", FIELD_NAME);

        HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), scorer);

        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(20));

        results.add(highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, fragmentSeparator));
    }

    EXPECT_EQ(numHighlights, 5);

    EXPECT_EQ(results.size(), 4);
    EXPECT_EQ(results[0], L" <B>kennedy</B> has been shot");
    EXPECT_EQ(results[1], L" refers to <B>Kennedy</B>... to <B>Kennedy</B>");
    EXPECT_EQ(results[2], L"John <B>Kennedy</B> has been shot");
    EXPECT_EQ(results[3], L" to <B>Keneddy</B>");
}

namespace TestGetBestFragmentsPhrase {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"\"John Kennedy\"");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);

        // Currently highlights "John" and "Kennedy" separately
        EXPECT_EQ(fixture->numHighlights, 2);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsPhrase) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsPhrase::HelperHighlightRunner>(this);
    helper->start(newCollection<String>(L"<B>John</B> <B>Kennedy</B> has been shot"));
}

namespace TestGetBestFragmentsQueryScorer {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                               newLucene<SpanTermQuery>(newLucene<Term>(L"contents", L"john")),
                                               newLucene<SpanTermQuery>(newLucene<Term>(L"contents", L"kennedy"))
                                           );

        SpanNearQueryPtr snq = newLucene<SpanNearQuery>(clauses, 1, true);
        fixture->doSearching(snq);
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);

        // Currently highlights "John" and "Kennedy" separately
        EXPECT_EQ(fixture->numHighlights, 2);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsQueryScorer) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsQueryScorer::HelperHighlightRunner>(this);
    helper->start(newCollection<String>(L"<B>John</B> <B>Kennedy</B> has been shot"));
}

namespace TestOffByOne {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"data", L"help"));
        HighlighterPtr hg = newLucene<Highlighter>(newLucene<SimpleHTMLFormatter>(), newLucene<QueryTermScorer>(query));
        hg->setTextFragmenter(newLucene<NullFragmenter>());

        String match = hg->getBestFragment(fixture->analyzer, L"data", L"help me [54-65]");
        EXPECT_EQ(L"<B>help</B> me [54-65]", match);
    }
};

}

TEST_F(HighlighterTest, testOffByOne) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestOffByOne::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestGetBestFragmentsFilteredQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        TermRangeFilterPtr rf = newLucene<TermRangeFilter>(L"contents", L"john", L"john", true, true);
        Collection<SpanQueryPtr> clauses = newCollection<SpanQueryPtr>(
                                               newLucene<SpanTermQuery>(newLucene<Term>(L"contents", L"john")),
                                               newLucene<SpanTermQuery>(newLucene<Term>(L"contents", L"kennedy"))
                                           );
        SpanNearQueryPtr snq = newLucene<SpanNearQuery>(clauses, 1, true);
        FilteredQueryPtr fq = newLucene<FilteredQuery>(snq, rf);

        fixture->doSearching(fq);
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);

        // Currently highlights "John" and "Kennedy" separately
        EXPECT_EQ(fixture->numHighlights, 2);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsFilteredQuery) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsFilteredQuery::HelperHighlightRunner>(this);
    helper->start(newCollection<String>(L"<B>John</B> <B>Kennedy</B> has been shot"));
}

namespace TestGetBestFragmentsFilteredPhraseQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        TermRangeFilterPtr rf = newLucene<TermRangeFilter>(L"contents", L"john", L"john", true, true);
        PhraseQueryPtr pq = newLucene<PhraseQuery>();
        pq->add(newLucene<Term>(L"contents", L"john"));
        pq->add(newLucene<Term>(L"contents", L"kennedy"));
        FilteredQueryPtr fq = newLucene<FilteredQuery>(pq, rf);

        fixture->doSearching(fq);
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);

        // Currently highlights "John" and "Kennedy" separately
        EXPECT_EQ(fixture->numHighlights, 2);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsFilteredPhraseQuery) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsFilteredPhraseQuery::HelperHighlightRunner>(this);
    helper->start(newCollection<String>(L"<B>John</B> <B>Kennedy</B> has been shot"));
}

namespace TestGetBestFragmentsMultiTerm {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"John Kenn*");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 5);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsMultiTerm) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsMultiTerm::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L"<B>John</B> <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

namespace TestGetBestFragmentsWithOr {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"JFK OR Kennedy");
        doStandardHighlights(fixture->analyzer, fixture->searcher, fixture->hits, fixture->query, newLucene<HighlighterTestNS::TestFormatter>(fixture), expected);
        EXPECT_EQ(fixture->numHighlights, 5);
    }
};

}

TEST_F(HighlighterTest, testGetBestFragmentsWithOr) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestFragmentsWithOr::HelperHighlightRunner>(this);

    helper->start(
        newCollection<String>(
            L"<B>JFK</B> has been shot",
            L"John <B>Kennedy</B> has been shot",
            L" refers to <B>Kennedy</B>... to <B>Kennedy</B>",
            L" <B>kennedy</B> has been shot"
        )
    );
}

namespace TestGetBestSingleFragment {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->doSearching(L"Kennedy");
        fixture->numHighlights = 0;
        Collection<String> results = Collection<String>::newInstance();

        for (int32_t i = 0; i < fixture->hits->totalHits; ++i) {
            String text = fixture->searcher->doc(fixture->hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));
            results.add(highlighter->getBestFragment(tokenStream, text));
        }
        EXPECT_EQ(fixture->numHighlights, 4);

        EXPECT_EQ(results.size(), 3);
        EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
        EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B>");
        EXPECT_EQ(results[2], L" <B>kennedy</B> has been shot");

        fixture->numHighlights = 0;
        results = Collection<String>::newInstance();

        for (int32_t i = 0; i < fixture->hits->totalHits; ++i) {
            String text = fixture->searcher->doc(fixture->hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            results.add(highlighter->getBestFragment(fixture->analyzer, HighlighterTest::FIELD_NAME, text));
        }
        EXPECT_EQ(fixture->numHighlights, 4);

        EXPECT_EQ(results.size(), 3);
        EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
        EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B> at the beginning then has a longer piece of text that is very");
        EXPECT_EQ(results[2], L" is really here which says <B>kennedy</B> has been shot");

        fixture->numHighlights = 0;
        results = Collection<String>::newInstance();

        for (int32_t i = 0; i < fixture->hits->totalHits; ++i) {
            String text = fixture->searcher->doc(fixture->hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));
            Collection<String> result = highlighter->getBestFragments(fixture->analyzer, HighlighterTest::FIELD_NAME, text, 10);
            results.addAll(result.begin(), result.end());
        }
        EXPECT_EQ(fixture->numHighlights, 4);

        EXPECT_EQ(results.size(), 3);
        EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
        EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B> at the beginning then has a longer piece of text that is very long in the middle and finally ends with another reference to <B>Kennedy</B>");
        EXPECT_EQ(results[2], L"Hello this is a piece of text that is very long and contains too much preamble and the meat is really here which says <B>kennedy</B> has been shot");
    }
};

}

TEST_F(HighlighterTest, testGetBestSingleFragment) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestSingleFragment::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestGetBestSingleFragmentWithWeights {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        Collection<WeightedTermPtr> wTerms = Collection<WeightedTermPtr>::newInstance(2);
        wTerms[0] = newLucene<WeightedSpanTerm>(10.0, L"hello");

        Collection<PositionSpanPtr> positionSpans = newCollection<PositionSpanPtr>(newLucene<PositionSpan>(0, 0));
        boost::dynamic_pointer_cast<WeightedSpanTerm>(wTerms[0])->addPositionSpans(positionSpans);

        wTerms[1] = newLucene<WeightedSpanTerm>(1.0, L"kennedy");
        positionSpans = newCollection<PositionSpanPtr>(newLucene<PositionSpan>(14, 14));
        boost::dynamic_pointer_cast<WeightedSpanTerm>(wTerms[1])->addPositionSpans(positionSpans);

        HighlighterPtr highlighter = getHighlighter(wTerms, newLucene<HighlighterTestNS::TestFormatter>(fixture));
        TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(fixture->texts[0]));
        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(2));

        String result = highlighter->getBestFragment(tokenStream, fixture->texts[0]);
        boost::trim(result);

        EXPECT_EQ(L"<B>Hello</B>", result);

        wTerms[1]->setWeight(50.0);
        tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(fixture->texts[0]));
        highlighter = getHighlighter(wTerms, newLucene<HighlighterTestNS::TestFormatter>(fixture));
        highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(2));

        result = highlighter->getBestFragment(tokenStream, fixture->texts[0]);
        boost::trim(result);

        EXPECT_EQ(L"<B>kennedy</B>", result);
    }
};

}

TEST_F(HighlighterTest, testGetBestSingleFragmentWithWeights) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetBestSingleFragmentWithWeights::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestOverlapAnalyzer {

class SynonymTokenizer : public TokenStream {
public:
    SynonymTokenizer(const TokenStreamPtr& realStream, MapStringString synonyms) {
        this->realStream = realStream;
        this->synonyms = synonyms;
        this->synonymToken = 0;
        this->realTermAtt = realStream->addAttribute<TermAttribute>();
        this->realPosIncrAtt = realStream->addAttribute<PositionIncrementAttribute>();
        this->realOffsetAtt = realStream->addAttribute<OffsetAttribute>();

        this->termAtt = addAttribute<TermAttribute>();
        this->posIncrAtt = addAttribute<PositionIncrementAttribute>();
        this->offsetAtt = addAttribute<OffsetAttribute>();
    }

    virtual ~SynonymTokenizer() {
    }

protected:
    TokenStreamPtr realStream;
    TokenPtr currentRealToken;
    TokenPtr cRealToken;
    MapStringString synonyms;
    Collection<String> synonymTokens;
    int32_t synonymToken;
    TermAttributePtr realTermAtt;
    PositionIncrementAttributePtr realPosIncrAtt;
    OffsetAttributePtr realOffsetAtt;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken() {
        if (!currentRealToken) {
            bool next = realStream->incrementToken();
            if (!next) {
                return false;
            }
            clearAttributes();
            termAtt->setTermBuffer(realTermAtt->term());
            offsetAtt->setOffset(realOffsetAtt->startOffset(), realOffsetAtt->endOffset());
            posIncrAtt->setPositionIncrement(realPosIncrAtt->getPositionIncrement());

            if (!synonyms.contains(realTermAtt->term())) {
                return true;
            }
            String expansions = synonyms.get(realTermAtt->term());
            synonymTokens = StringUtils::split(expansions, L",");
            synonymToken = 0;
            if (!synonymTokens.empty()) {
                currentRealToken = newLucene<Token>(realOffsetAtt->startOffset(), realOffsetAtt->endOffset());
                currentRealToken->setTermBuffer(realTermAtt->term());
            }
            return true;
        } else {
            String tok = synonymTokens[synonymToken++];
            clearAttributes();
            termAtt->setTermBuffer(tok);
            offsetAtt->setOffset(currentRealToken->startOffset(), currentRealToken->endOffset());
            posIncrAtt->setPositionIncrement(0);
            if (synonymToken == synonymTokens.size()) {
                currentRealToken.reset();
                synonymTokens.reset();
                synonymToken = 0;
            }
            return true;
        }
    }
};

class SynonymAnalyzer : public Analyzer {
public:
    SynonymAnalyzer(MapStringString synonyms) {
        this->synonyms = synonyms;
    }

    virtual ~SynonymAnalyzer() {
    }

protected:
    MapStringString synonyms;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        LowerCaseTokenizerPtr stream = newLucene<LowerCaseTokenizer>(reader);
        stream->addAttribute<TermAttribute>();
        stream->addAttribute<PositionIncrementAttribute>();
        stream->addAttribute<OffsetAttribute>();
        return newLucene<SynonymTokenizer>(stream, synonyms);
    }
};

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        MapStringString synonyms = MapStringString::newInstance();
        synonyms.put(L"football", L"soccer,footie");
        AnalyzerPtr analyzer = newLucene<SynonymAnalyzer>(synonyms);
        String srchkey = L"football";

        String s = L"football-soccer in the euro 2004 footie competition";
        QueryParserPtr parser = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"bookid", analyzer);
        QueryPtr query = parser->parse(srchkey);

        TokenStreamPtr tokenStream = analyzer->tokenStream(L"", newLucene<StringReader>(s));

        HighlighterPtr highlighter = getHighlighter(query, L"", tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));

        // Get 3 best fragments and separate with a "..."
        tokenStream = analyzer->tokenStream(L"", newLucene<StringReader>(s));

        String result = highlighter->getBestFragments(tokenStream, s, 3, L"...");
        String expectedResult = L"<B>football</B>-<B>soccer</B> in the euro 2004 <B>footie</B> competition";

        EXPECT_EQ(expectedResult, result);
    }
};

}

/// tests a "complex" analyzer that produces multiple overlapping tokens
TEST_F(HighlighterTest, testOverlapAnalyzer) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestOverlapAnalyzer::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestGetSimpleHighlight {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"Kennedy");

        Collection<String> results = Collection<String>::newInstance();

        for (int32_t i = 0; i < fixture->hits->totalHits; ++i) {
            String text = fixture->searcher->doc(fixture->hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            results.add(highlighter->getBestFragment(tokenStream, text));
        }
        EXPECT_EQ(fixture->numHighlights, 4);

        EXPECT_EQ(results.size(), 3);
        EXPECT_EQ(results[0], L"John <B>Kennedy</B> has been shot");
        EXPECT_EQ(results[1], L"This piece of text refers to <B>Kennedy</B> at the beginning then has a longer piece of text that is very");
        EXPECT_EQ(results[2], L" is really here which says <B>kennedy</B> has been shot");
    }
};

}

TEST_F(HighlighterTest, testGetSimpleHighlight) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetSimpleHighlight::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestGetTextFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->doSearching(L"Kennedy");

        for (int32_t i = 0; i < fixture->hits->totalHits; ++i) {
            String text = fixture->searcher->doc(fixture->hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));

            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(20));
            Collection<String> stringResults = highlighter->getBestFragments(tokenStream, text, 10);

            tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            Collection<TextFragmentPtr> fragmentResults = highlighter->getBestTextFragments(tokenStream, text, true, 10);

            EXPECT_EQ(fragmentResults.size(), stringResults.size());
            for (int32_t j = 0; j < stringResults.size(); ++j) {
                EXPECT_EQ(fragmentResults[j]->toString(), stringResults[j]);
            }
        }
    }
};

}

TEST_F(HighlighterTest, testGetTextFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestGetTextFragments::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestMaxSizeHighlight {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        fixture->doSearching(L"meat");

        TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(fixture->texts[0]));
        HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
        highlighter->setMaxDocCharsToAnalyze(30);

        highlighter->getBestFragment(tokenStream, fixture->texts[0]);
        EXPECT_EQ(fixture->numHighlights, 0);
    }
};

}

TEST_F(HighlighterTest, testMaxSizeHighlight) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestMaxSizeHighlight::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestMaxSizeHighlightTruncates {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        String goodWord = L"goodtoken";
        HashSet<String> stopWords = HashSet<String>::newInstance();
        stopWords.add(L"stoppedtoken");

        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"data", goodWord));

        StringStream buffer;
        buffer << goodWord;

        for (int32_t i = 0; i < 10000; ++i) {
            // only one stopword
            buffer << L" " << *stopWords.begin();
        }
        SimpleHTMLFormatterPtr fm = newLucene<SimpleHTMLFormatter>();
        HighlighterPtr hg = getHighlighter(query, L"data", newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION, stopWords)->tokenStream(L"data", newLucene<StringReader>(buffer.str())), fm);

        hg->setTextFragmenter(newLucene<NullFragmenter>());
        hg->setMaxDocCharsToAnalyze(100);
        String match = hg->getBestFragment(newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION, stopWords), L"data", buffer.str());
        EXPECT_TRUE((int32_t)match.length() < hg->getMaxDocCharsToAnalyze());

        // add another tokenized word to the overall length - but set way beyond the length of text under consideration
        // (after a large slug of stop words + whitespace)
        buffer << L" " << goodWord;
        match = hg->getBestFragment(newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION, stopWords), L"data", buffer.str());
        EXPECT_TRUE((int32_t)match.length() < hg->getMaxDocCharsToAnalyze());
    }
};

}

TEST_F(HighlighterTest, testMaxSizeHighlightTruncates) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestMaxSizeHighlightTruncates::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestMaxSizeEndHighlight {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        HashSet<String> stopWords = HashSet<String>::newInstance();
        stopWords.add(L"in");
        stopWords.add(L"it");

        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"text", L"searchterm"));

        String text = L"this is a text with searchterm in it";

        SimpleHTMLFormatterPtr fm = newLucene<SimpleHTMLFormatter>();
        HighlighterPtr hg = getHighlighter(query, L"text", newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION, stopWords)->tokenStream(L"text", newLucene<StringReader>(text)), fm);

        hg->setTextFragmenter(newLucene<NullFragmenter>());
        hg->setMaxDocCharsToAnalyze(36);
        String match = hg->getBestFragment(newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION, stopWords), L"text", text);
        EXPECT_TRUE(boost::ends_with(match, L"in it"));
    }
};

}

TEST_F(HighlighterTest, testMaxSizeEndHighlight) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestMaxSizeEndHighlight::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestUnRewrittenQuery {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->numHighlights = 0;
        // test to show how rewritten query can still be used
        fixture->searcher = newLucene<IndexSearcher>(fixture->ramDir, true);
        AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(HighlighterTest::TEST_VERSION);

        QueryParserPtr parser = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, HighlighterTest::FIELD_NAME, analyzer);
        QueryPtr query = parser->parse(L"JF? or Kenned*");
        TopDocsPtr hits = fixture->searcher->search(query, FilterPtr(), 1000);

        int32_t maxNumFragmentsRequired = 3;

        for (int32_t i = 0; i < hits->totalHits; ++i) {
            String text = fixture->searcher->doc(hits->scoreDocs[i]->doc)->get(HighlighterTest::FIELD_NAME);
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture), false);

            highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(40));

            highlighter->getBestFragments(tokenStream, text, maxNumFragmentsRequired, L"...");
        }

        // We expect to have zero highlights if the query is multi-terms and is not rewritten
        EXPECT_EQ(fixture->numHighlights, 0);
    }
};

}

TEST_F(HighlighterTest, testUnRewrittenQuery) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestUnRewrittenQuery::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestNoFragments {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        fixture->doSearching(L"AnInvalidQueryWhichShouldYieldNoResults");

        for (int32_t i = 0; i < fixture->texts.size(); ++i) {
            String text = fixture->texts[i];
            TokenStreamPtr tokenStream = fixture->analyzer->tokenStream(HighlighterTest::FIELD_NAME, newLucene<StringReader>(text));
            HighlighterPtr highlighter = getHighlighter(fixture->query, HighlighterTest::FIELD_NAME, tokenStream, newLucene<HighlighterTestNS::TestFormatter>(fixture));
            String result = highlighter->getBestFragment(tokenStream, text);
            EXPECT_TRUE(result.empty());
        }
    }
};

}

TEST_F(HighlighterTest, testNoFragments) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestNoFragments::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestEncoding {

class NullScorer : public HighlighterScorer, public LuceneObject {
public:
    virtual ~NullScorer() {
    }

public:
    virtual void startFragment(const TextFragmentPtr& newFragment) {
    }

    virtual double getTokenScore() {
        return 0.0;
    }

    virtual double getFragmentScore() {
        return 1.0;
    }

    virtual TokenStreamPtr init(const TokenStreamPtr& tokenStream) {
        return TokenStreamPtr();
    }
};

}

/// Demonstrates creation of an XHTML compliant doc using new encoding facilities.
TEST_F(HighlighterTest, testEncoding) {
    String rawDocContent = L"\"Smith & sons' prices < 3 and >4\" claims article";

    // run the highlighter on the raw content (scorer does not score any tokens for
    // highlighting but scores a single fragment for selection
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), newLucene<SimpleHTMLEncoder>(), newLucene<TestEncoding::NullScorer>());

    highlighter->setTextFragmenter(newLucene<SimpleFragmenter>(2000));
    TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(rawDocContent));

    String encodedSnippet = highlighter->getBestFragments(tokenStream, rawDocContent, 1, L"");
    EXPECT_EQ(encodedSnippet, L"&quot;Smith &amp; sons' prices &lt; 3 and &gt;4&quot; claims article");
}

TEST_F(HighlighterTest, testMultiSearcher) {
    // setup index 1
    RAMDirectoryPtr ramDir1 = newLucene<RAMDirectory>();
    IndexWriterPtr writer1 = newLucene<IndexWriter>(ramDir1, newLucene<StandardAnalyzer>(TEST_VERSION), true, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr d = newLucene<Document>();
    FieldPtr f = newLucene<Field>(FIELD_NAME, L"multiOne", Field::STORE_YES, Field::INDEX_ANALYZED);
    d->add(f);
    writer1->addDocument(d);
    writer1->optimize();
    writer1->close();
    IndexReaderPtr reader1 = IndexReader::open(ramDir1, true);

    // setup index 2
    RAMDirectoryPtr ramDir2 = newLucene<RAMDirectory>();
    IndexWriterPtr writer2 = newLucene<IndexWriter>(ramDir2, newLucene<StandardAnalyzer>(TEST_VERSION), true, IndexWriter::MaxFieldLengthUNLIMITED);
    d = newLucene<Document>();
    f = newLucene<Field>(FIELD_NAME, L"multiTwo", Field::STORE_YES, Field::INDEX_ANALYZED);
    d->add(f);
    writer2->addDocument(d);
    writer2->optimize();
    writer2->close();
    IndexReaderPtr reader2 = IndexReader::open(ramDir2, true);

    Collection<SearchablePtr> searchers = newCollection<SearchablePtr>(
            newLucene<IndexSearcher>(ramDir1, true),
            newLucene<IndexSearcher>(ramDir2, true)
                                          );
    MultiSearcherPtr multiSearcher = newLucene<MultiSearcher>(searchers);
    QueryParserPtr parser = newLucene<QueryParser>(TEST_VERSION, FIELD_NAME, newLucene<StandardAnalyzer>(TEST_VERSION));
    parser->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
    query = parser->parse(L"multi*");
    // at this point the multisearcher calls combine(query[])
    hits = multiSearcher->search(query, FilterPtr(), 1000);

    Collection<QueryPtr> expandedQueries = newCollection<QueryPtr>(
            query->rewrite(reader1),
            query->rewrite(reader2)
                                           );
    query = query->combine(expandedQueries);

    // create an instance of the highlighter with the tags used to surround highlighted text
    HighlighterPtr highlighter = newLucene<Highlighter>(newLucene<HighlighterTestNS::TestFormatter>(this), newLucene<QueryTermScorer>(query));
    Collection<String> results = Collection<String>::newInstance();

    for (int32_t i = 0; i < hits->totalHits; ++i) {
        String text = multiSearcher->doc(hits->scoreDocs[i]->doc)->get(FIELD_NAME);
        TokenStreamPtr tokenStream = analyzer->tokenStream(FIELD_NAME, newLucene<StringReader>(text));
        String highlightedText = highlighter->getBestFragment(tokenStream, text);
        results.add(highlightedText);
    }

    EXPECT_EQ(results.size(), 2);
    EXPECT_EQ(results[0], L"<B>multiOne</B>");
    EXPECT_EQ(results[1], L"<B>multiTwo</B>");

    EXPECT_EQ(numHighlights, 2);
}

namespace TestFieldSpecificHighlighting {

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        String docMainText = L"fred is one of the people";
        QueryParserPtr parser = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, HighlighterTest::FIELD_NAME, fixture->analyzer);
        QueryPtr query = parser->parse(L"fred category:people");

        // highlighting respects fieldnames used in query

        HighlighterScorerPtr fieldSpecificScorer;
        if (mode == QUERY) {
            fieldSpecificScorer = newLucene<QueryScorer>(query, HighlighterTest::FIELD_NAME);
        } else if (mode == QUERY_TERM) {
            fieldSpecificScorer = newLucene<QueryTermScorer>(query, L"contents");
        }

        HighlighterPtr fieldSpecificHighlighter = newLucene<Highlighter>(newLucene<SimpleHTMLFormatter>(), fieldSpecificScorer);
        fieldSpecificHighlighter->setTextFragmenter(newLucene<NullFragmenter>());
        String result = fieldSpecificHighlighter->getBestFragment(fixture->analyzer, HighlighterTest::FIELD_NAME, docMainText);
        EXPECT_EQ(result, L"<B>fred</B> is one of the people");

        // highlighting does not respect fieldnames used in query
        HighlighterScorerPtr fieldInSpecificScorer;
        if (mode == QUERY) {
            fieldInSpecificScorer = newLucene<QueryScorer>(query, L"");
        } else if (mode == QUERY_TERM) {
            fieldInSpecificScorer = newLucene<QueryTermScorer>(query);
        }

        HighlighterPtr fieldInSpecificHighlighter = newLucene<Highlighter>(newLucene<SimpleHTMLFormatter>(), fieldInSpecificScorer);
        fieldInSpecificHighlighter->setTextFragmenter(newLucene<NullFragmenter>());
        result = fieldInSpecificHighlighter->getBestFragment(fixture->analyzer, HighlighterTest::FIELD_NAME, docMainText);
        EXPECT_EQ(result, L"<B>fred</B> is one of the <B>people</B>");

        fixture->reader->close();
    }
};

}

TEST_F(HighlighterTest, testFieldSpecificHighlighting) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestFieldSpecificHighlighting::HelperHighlightRunner>(this);
    helper->start();
}

namespace TestOverlapAnalyzer2 {

class TS2 : public TokenStream {
public:
    TS2() {
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        lst = Collection<TokenPtr>::newInstance();
        TokenPtr t = createToken(L"hi", 0, 2);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"hispeed", 0, 8);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"speed", 3, 8);
        t->setPositionIncrement(0);
        lst.add(t);
        t = createToken(L"10", 8, 10);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"foo", 11, 14);
        t->setPositionIncrement(1);
        lst.add(t);
        tokenPos = 0;
    }

    virtual ~TS2() {
    }

protected:
    Collection<TokenPtr> lst;
    int32_t tokenPos;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken() {
        if (tokenPos < (int32_t)lst.size()) {
            TokenPtr token = lst[tokenPos++];
            clearAttributes();
            termAtt->setTermBuffer(token->term());
            posIncrAtt->setPositionIncrement(token->getPositionIncrement());
            offsetAtt->setOffset(token->startOffset(), token->endOffset());
            return true;
        }
        return false;
    }

protected:
    TokenPtr createToken(const String& term, int32_t start, int32_t offset) {
        TokenPtr token = newLucene<Token>(start, offset);
        token->setTermBuffer(term);
        return token;
    }
};

/// same token-stream as above, but the bigger token comes first this time
class TS2a : public TokenStream {
public:
    TS2a() {
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        lst = Collection<TokenPtr>::newInstance();
        TokenPtr t = createToken(L"hispeed", 0, 8);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"hi", 0, 2);
        t->setPositionIncrement(0);
        lst.add(t);
        t = createToken(L"speed", 3, 8);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"10", 8, 10);
        t->setPositionIncrement(1);
        lst.add(t);
        t = createToken(L"foo", 11, 14);
        t->setPositionIncrement(1);
        lst.add(t);
        tokenPos = 0;
    }

    virtual ~TS2a() {
    }

protected:
    Collection<TokenPtr> lst;
    int32_t tokenPos;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken() {
        if (tokenPos < (int32_t)lst.size()) {
            TokenPtr token = lst[tokenPos++];
            clearAttributes();
            termAtt->setTermBuffer(token->term());
            posIncrAtt->setPositionIncrement(token->getPositionIncrement());
            offsetAtt->setOffset(token->startOffset(), token->endOffset());
            return true;
        }
        return false;
    }

protected:
    TokenPtr createToken(const String& term, int32_t start, int32_t offset) {
        TokenPtr token = newLucene<Token>(start, offset);
        token->setTermBuffer(term);
        return token;
    }
};

class HelperHighlightRunner : public HighlighterTestNS::TestHighlightRunner {
public:
    HelperHighlightRunner(HighlighterTest* fixture) : HighlighterTestNS::TestHighlightRunner(fixture) {
    }

    virtual ~HelperHighlightRunner() {
    }

public:
    virtual void run(Collection<String> expected) {
        String s = L"Hi-Speed10 foo";

        QueryPtr query;
        HighlighterPtr highlighter;
        String result;

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"foo");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"Hi-Speed10 <B>foo</B>", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"10");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"Hi-Speed<B>10</B> foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hi");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi</B>-Speed10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"speed");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"Hi-<B>Speed</B>10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hispeed");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi-Speed</B>10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hi speed");
        highlighter = getHighlighter(query, L"text", getTS2(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi-Speed</B>10 foo", result);

        // same tests, just put the bigger overlapping token first
        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"foo");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"Hi-Speed10 <B>foo</B>", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"10");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"Hi-Speed<B>10</B> foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hi");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi</B>-Speed10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"speed");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"Hi-<B>Speed</B>10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hispeed");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi-Speed</B>10 foo", result);

        query = newLucene<QueryParser>(HighlighterTest::TEST_VERSION, L"text", newLucene<WhitespaceAnalyzer>())->parse(L"hi speed");
        highlighter = getHighlighter(query, L"text", getTS2a(), newLucene<HighlighterTestNS::TestFormatter>(fixture));
        result = highlighter->getBestFragments(getTS2a(), s, 3, L"...");
        EXPECT_EQ(L"<B>Hi-Speed</B>10 foo", result);
    }

    TokenStreamPtr getTS2() {
        return newLucene<TS2>();
    }

    TokenStreamPtr getTS2a() {
        return newLucene<TS2a>();
    }
};

}

TEST_F(HighlighterTest, testOverlapAnalyzer2) {
    HighlighterTestNS::TestHighlightRunnerPtr helper = newLucene<TestOverlapAnalyzer2::HelperHighlightRunner>(this);
    helper->start();
}

TEST_F(HighlighterTest, testWeightedTermsWithDeletes) {
    makeIndex();
    deleteDocument();
    searchIndex();
}
