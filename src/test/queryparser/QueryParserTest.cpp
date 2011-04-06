/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "QueryParser.h"
#include "WhitespaceAnalyzer.h"
#include "KeywordAnalyzer.h"
#include "Query.h"
#include "SimpleAnalyzer.h"
#include "TermQuery.h"
#include "PhraseQuery.h"
#include "FuzzyQuery.h"
#include "PrefixQuery.h"
#include "StandardAnalyzer.h"
#include "WildcardQuery.h"
#include "BooleanQuery.h"
#include "TokenFilter.h"
#include "TermAttribute.h"
#include "OffsetAttribute.h"
#include "LowerCaseTokenizer.h"
#include "MultiTermQuery.h"
#include "TermRangeQuery.h"
#include "MockRAMDirectory.h"
#include "IndexWriter.h"
#include "IndexSearcher.h"
#include "Document.h"
#include "Field.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "DateField.h"
#include "Term.h"
#include "StopAnalyzer.h"
#include "StopFilter.h"
#include "MatchAllDocsQuery.h"
#include "IndexReader.h"
#include "MiscUtils.h"

using namespace Lucene;
using namespace boost::posix_time;
using namespace boost::gregorian;

DECLARE_SHARED_PTR(TestAnalyzer)
DECLARE_SHARED_PTR(TestFilter)
DECLARE_SHARED_PTR(TestParser)

/// Filter which discards the token 'stop' and which expands the token 'phrase' into 'phrase1 phrase2'
class TestFilter : public TokenFilter
{
public:
    TestFilter(TokenStreamPtr in) : TokenFilter(in)
    {
        termAtt = addAttribute<TermAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        inPhrase = false;
        savedStart = 0;
        savedEnd = 0;
    }
    
    virtual ~TestFilter()
    {
    }
    
    LUCENE_CLASS(TestFilter);

public:
    bool inPhrase;
    int32_t savedStart;
    int32_t savedEnd;
    TermAttributePtr termAtt;
    OffsetAttributePtr offsetAtt;

public:
    virtual bool incrementToken()
    {
        if (inPhrase)
        {
            inPhrase = false;
            clearAttributes();
            termAtt->setTermBuffer(L"phrase2");
            offsetAtt->setOffset(savedStart, savedEnd);
            return true;
        }
        else
        {
            while (input->incrementToken())
            {
                if (termAtt->term() == L"phrase")
                {
                    inPhrase = true;
                    savedStart = offsetAtt->startOffset();
                    savedEnd = offsetAtt->endOffset();
                    termAtt->setTermBuffer(L"phrase1");
                    offsetAtt->setOffset(savedStart, savedEnd);
                    return true;
                }
                else if (termAtt->term() != L"stop")
                    return true;
            }
        }
        return false;
    }
};

class TestAnalyzer : public Analyzer
{
public:
    virtual ~TestAnalyzer()
    {
    }
    
    LUCENE_CLASS(TestAnalyzer);

public:
    // Filters LowerCaseTokenizer with StopFilter.
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<TestFilter>(newLucene<LowerCaseTokenizer>(reader));
    }
};

class TestParser : public QueryParser
{
public:
    TestParser(const String& f, AnalyzerPtr a) : QueryParser(LuceneVersion::LUCENE_CURRENT, f, a)
    {
    }
    
    virtual ~TestParser()
    {
    }
    
    LUCENE_CLASS(TestParser);

public:
    virtual QueryPtr getFuzzyQuery(const String& field, const String& termStr, double minSimilarity)
    {
        boost::throw_exception(QueryParserError(L"Fuzzy queries not allowed"));
        return QueryPtr();
    }
    
    virtual QueryPtr getWildcardQuery(const String& field, const String& termStr)
    {
        boost::throw_exception(QueryParserError(L"Wildcard queries not allowed"));
        return QueryPtr();
    }
};

class QueryParserTestFixture : public LuceneTestFixture
{
public:
    QueryParserTestFixture()
    {
        originalMaxClauses = BooleanQuery::getMaxClauseCount();
        DateTools::setDateOrder(DateTools::DATEORDER_LOCALE);
    }
    
    virtual ~QueryParserTestFixture()
    {
        BooleanQuery::setMaxClauseCount(originalMaxClauses);
    }

protected:
    int32_t originalMaxClauses;
    
    QueryParserPtr getParser(AnalyzerPtr a)
    {
        if (!a)
            a = newLucene<SimpleAnalyzer>();
        QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", a);
        qp->setDefaultOperator(QueryParser::OR_OPERATOR);
        return qp;
    }
    
    QueryPtr getQuery(const String& query, AnalyzerPtr a)
    {
        return getParser(a)->parse(query);
    }
    
    void checkQueryEquals(const String& query, AnalyzerPtr a, const String& result)
    {
        QueryPtr q = getQuery(query, a);
        String s = q->toString(L"field");
        if (s != result)
            BOOST_FAIL("Query \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(s) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    void checkQueryEquals(QueryParserPtr qp, const String& field, const String& query, const String& result)
    {
        QueryPtr q = qp->parse(query);
        String s = q->toString(field);
        if (s != result)
            BOOST_FAIL("Query \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(s) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    void checkParseException(const String& queryString)
    {
        BOOST_CHECK_EXCEPTION(getQuery(queryString, AnalyzerPtr()), QueryParserError, check_exception(LuceneException::QueryParser));
    }
    
    void checkWildcardQueryEquals(const String& query, bool lowercase, const String& result, bool allowLeadingWildcard = false)
    {
        QueryParserPtr qp = getParser(AnalyzerPtr());
        qp->setLowercaseExpandedTerms(lowercase);
        qp->setAllowLeadingWildcard(allowLeadingWildcard);
        QueryPtr q = qp->parse(query);
        String s = q->toString(L"field");
        if (s != result)
            BOOST_FAIL("WildcardQuery \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(s) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    void checkWildcardQueryEquals(const String& query, const String& result)
    {
        QueryParserPtr qp = getParser(AnalyzerPtr());
        QueryPtr q = qp->parse(query);
        String s = q->toString(L"field");
        if (s != result)
            BOOST_FAIL("WildcardQuery \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(s) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    void checkEscapedQueryEquals(const String& query, AnalyzerPtr a, const String& result)
    {
        class TestableQueryParser : public QueryParser
        {
        public:
            using QueryParser::escape;
        };
        
        String escapedQuery = TestableQueryParser::escape(query);
        if (escapedQuery != result)
            BOOST_FAIL("Query \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(escapedQuery) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    QueryPtr getQueryDOA(const String& query, AnalyzerPtr a)
    {
        if (!a)
            a = newLucene<SimpleAnalyzer>();
        QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", a);
        qp->setDefaultOperator(QueryParser::AND_OPERATOR);
        return qp->parse(query);
    }
    
    void checkQueryEqualsDOA(const String& query, AnalyzerPtr a, const String& result)
    {
        QueryPtr q = getQueryDOA(query, a);
        String s = q->toString(L"field");
        if (s != result)
            BOOST_FAIL("Query \"" << StringUtils::toUTF8(query) << "\" yielded \"" << StringUtils::toUTF8(s) << "\", expecting \"" << StringUtils::toUTF8(result) << "\"");
    }
    
    void addDateDoc(const String& content, boost::posix_time::ptime date, IndexWriterPtr iw)
    {
        DocumentPtr d = newLucene<Document>();
        d->add(newLucene<Field>(L"f", content, Field::STORE_YES, Field::INDEX_ANALYZED));
        d->add(newLucene<Field>(L"date", DateField::dateToString(date), Field::STORE_YES, Field::INDEX_ANALYZED));
        iw->addDocument(d);
    }
    
    void checkHits(int32_t expected, const String& query, IndexSearcherPtr is)
    {
        QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"date", newLucene<WhitespaceAnalyzer>());
        qp->setLocale(std::locale());
        QueryPtr q = qp->parse(query);
        Collection<ScoreDocPtr> hits = is->search(q, FilterPtr(), 1000)->scoreDocs;
        BOOST_CHECK_EQUAL(expected, hits.size());
    }
};

BOOST_FIXTURE_TEST_SUITE(QueryParserTest, QueryParserTestFixture)

BOOST_AUTO_TEST_CASE(testSimple)
{
    checkQueryEquals(L"term term term", AnalyzerPtr(), L"term term term");
    
    const uint8_t term[] = {0x74, 0xc3, 0xbc, 0x72, 0x6d, 0x20, 0x74, 0x65, 0x72, 0x6d, 0x20, 0x74, 0x65, 0x72, 0x6d};
    String termText = UTF8_TO_STRING(term);
    checkQueryEquals(termText, newLucene<WhitespaceAnalyzer>(), termText);
    
    const uint8_t umlaut[] = {0xc3, 0xbc, 0x6d, 0x6c, 0x61, 0x75, 0x74};
    String umlautText = UTF8_TO_STRING(umlaut);
    checkQueryEquals(umlautText, newLucene<WhitespaceAnalyzer>(), umlautText);

    checkQueryEquals(L"\"\"", newLucene<KeywordAnalyzer>(), L"");
    checkQueryEquals(L"foo:\"\"", newLucene<KeywordAnalyzer>(), L"foo:");

    checkQueryEquals(L"a AND b", AnalyzerPtr(), L"+a +b");
    checkQueryEquals(L"(a AND b)", AnalyzerPtr(), L"+a +b");
    checkQueryEquals(L"c OR (a AND b)", AnalyzerPtr(), L"c (+a +b)");
    checkQueryEquals(L"a AND NOT b", AnalyzerPtr(), L"+a -b");
    checkQueryEquals(L"a AND -b", AnalyzerPtr(), L"+a -b");
    checkQueryEquals(L"a AND !b", AnalyzerPtr(), L"+a -b");
    checkQueryEquals(L"a && b", AnalyzerPtr(), L"+a +b");
    checkQueryEquals(L"a && ! b", AnalyzerPtr(), L"+a -b");

    checkQueryEquals(L"a OR b", AnalyzerPtr(), L"a b");
    checkQueryEquals(L"a || b", AnalyzerPtr(), L"a b");
    checkQueryEquals(L"a OR !b", AnalyzerPtr(), L"a -b");
    checkQueryEquals(L"a OR ! b", AnalyzerPtr(), L"a -b");
    checkQueryEquals(L"a OR -b", AnalyzerPtr(), L"a -b");

    checkQueryEquals(L"+term -term term", AnalyzerPtr(), L"+term -term term");
    checkQueryEquals(L"foo:term AND field:anotherTerm", AnalyzerPtr(), L"+foo:term +anotherterm");
    checkQueryEquals(L"term AND \"phrase phrase\"", AnalyzerPtr(), L"+term +\"phrase phrase\"");
    checkQueryEquals(L"\"hello there\"", AnalyzerPtr(), L"\"hello there\"");
    BOOST_CHECK(MiscUtils::typeOf<BooleanQuery>(getQuery(L"a AND b", AnalyzerPtr())));
    BOOST_CHECK(MiscUtils::typeOf<TermQuery>(getQuery(L"hello", AnalyzerPtr())));
    BOOST_CHECK(MiscUtils::typeOf<PhraseQuery>(getQuery(L"\"hello there\"", AnalyzerPtr())));

    checkQueryEquals(L"germ term^2.0", AnalyzerPtr(), L"germ term^2.0");
    checkQueryEquals(L"(term)^2.0", AnalyzerPtr(), L"term^2.0");
    checkQueryEquals(L"(germ term)^2.0", AnalyzerPtr(), L"(germ term)^2.0");
    checkQueryEquals(L"term^2.0", AnalyzerPtr(), L"term^2.0");
    checkQueryEquals(L"term^2", AnalyzerPtr(), L"term^2.0");
    checkQueryEquals(L"\"germ term\"^2.0", AnalyzerPtr(), L"\"germ term\"^2.0");
    checkQueryEquals(L"\"term germ\"^2", AnalyzerPtr(), L"\"term germ\"^2.0");

    checkQueryEquals(L"(foo OR bar) AND (baz OR boo)", AnalyzerPtr(), L"+(foo bar) +(baz boo)");
    checkQueryEquals(L"((a OR b) AND NOT c) OR d", AnalyzerPtr(), L"(+(a b) -c) d");
    checkQueryEquals(L"+(apple \"steve jobs\") -(foo bar baz)", AnalyzerPtr(), L"+(apple \"steve jobs\") -(foo bar baz)");
    checkQueryEquals(L"+title:(dog OR cat) -author:\"bob dole\"", AnalyzerPtr(), L"+(title:dog title:cat) -author:\"bob dole\"");

    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    
    // make sure OR is the default
    BOOST_CHECK_EQUAL(QueryParser::OR_OPERATOR, qp->getDefaultOperator());
    qp->setDefaultOperator(QueryParser::AND_OPERATOR);
    BOOST_CHECK_EQUAL(QueryParser::AND_OPERATOR, qp->getDefaultOperator());
    qp->setDefaultOperator(QueryParser::OR_OPERATOR);
    BOOST_CHECK_EQUAL(QueryParser::OR_OPERATOR, qp->getDefaultOperator());
}

BOOST_AUTO_TEST_CASE(testPunct)
{
    AnalyzerPtr a = newLucene<WhitespaceAnalyzer>();
    checkQueryEquals(L"a&b", a, L"a&b");
    checkQueryEquals(L"a&&b", a, L"a&&b");
    checkQueryEquals(L".NET", a, L".NET");
}

BOOST_AUTO_TEST_CASE(testSlop)
{
    checkQueryEquals(L"\"term germ\"~2", AnalyzerPtr(), L"\"term germ\"~2");
    checkQueryEquals(L"\"term germ\"~2 flork", AnalyzerPtr(), L"\"term germ\"~2 flork");
    checkQueryEquals(L"\"term\"~2", AnalyzerPtr(), L"term");
    checkQueryEquals(L"\" \"~2 germ", AnalyzerPtr(), L"germ");
    checkQueryEquals(L"\"term germ\"~2^2", AnalyzerPtr(), L"\"term germ\"~2^2.0");
}

BOOST_AUTO_TEST_CASE(testNumber)
{
    // The numbers go away because SimpleAnalzyer ignores them
    checkQueryEquals(L"3", AnalyzerPtr(), L"");
    checkQueryEquals(L"term 1.0 1 2", AnalyzerPtr(), L"term");
    checkQueryEquals(L"term term1 term2", AnalyzerPtr(), L"term term term");

    AnalyzerPtr a = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkQueryEquals(L"3", a, L"3");
    checkQueryEquals(L"term 1.0 1 2", a, L"term 1.0 1 2");
    checkQueryEquals(L"term term1 term2", a, L"term term1 term2");
}

BOOST_AUTO_TEST_CASE(testWildcard)
{
    checkQueryEquals(L"term*", AnalyzerPtr(), L"term*");
    checkQueryEquals(L"term*^2", AnalyzerPtr(), L"term*^2.0");
    checkQueryEquals(L"term~", AnalyzerPtr(), L"term~0.5");
    checkQueryEquals(L"term~0.7", AnalyzerPtr(), L"term~0.7");
    checkQueryEquals(L"term~^2", AnalyzerPtr(), L"term~0.5^2.0");
    checkQueryEquals(L"term^2~", AnalyzerPtr(), L"term~0.5^2.0");
    checkQueryEquals(L"term*germ", AnalyzerPtr(), L"term*germ");
    checkQueryEquals(L"term*germ^3", AnalyzerPtr(), L"term*germ^3.0");

    BOOST_CHECK(MiscUtils::typeOf<PrefixQuery>(getQuery(L"term*", AnalyzerPtr())));
    BOOST_CHECK(MiscUtils::typeOf<PrefixQuery>(getQuery(L"term*^2", AnalyzerPtr())));
    BOOST_CHECK(MiscUtils::typeOf<FuzzyQuery>(getQuery(L"term~", AnalyzerPtr())));
    BOOST_CHECK(MiscUtils::typeOf<FuzzyQuery>(getQuery(L"term~0.7", AnalyzerPtr())));
    FuzzyQueryPtr fq = boost::dynamic_pointer_cast<FuzzyQuery>(getQuery(L"term~0.7", AnalyzerPtr()));
    BOOST_CHECK_CLOSE_FRACTION(0.7, fq->getMinSimilarity(), 0.1);
    BOOST_CHECK_EQUAL(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());
    fq = boost::dynamic_pointer_cast<FuzzyQuery>(getQuery(L"term~", AnalyzerPtr()));
    BOOST_CHECK_CLOSE_FRACTION(0.5, fq->getMinSimilarity(), 0.1);
    BOOST_CHECK_EQUAL(FuzzyQuery::defaultPrefixLength, fq->getPrefixLength());

    checkParseException(L"term~1.1"); // value > 1, throws exception

    BOOST_CHECK(MiscUtils::typeOf<WildcardQuery>(getQuery(L"term*germ", AnalyzerPtr())));

    // Tests to see that wild card terms are (or are not) properly lower-cased with propery parser configuration

    // First prefix queries
    // by default, convert to lowercase
    checkWildcardQueryEquals(L"Term*", true, L"term*");
    // explicitly set lowercase
    checkWildcardQueryEquals(L"term*", true, L"term*");
    checkWildcardQueryEquals(L"Term*", true, L"term*");
    checkWildcardQueryEquals(L"TERM*", true, L"term*");
    // explicitly disable lowercase conversion
    checkWildcardQueryEquals(L"term*", false, L"term*");
    checkWildcardQueryEquals(L"Term*", false, L"Term*");
    checkWildcardQueryEquals(L"TERM*", false, L"TERM*");
    // Then 'full' wildcard queries
    // by default, convert to lowercase
    checkWildcardQueryEquals(L"Te?m", L"te?m");
    // explicitly set lowercase
    checkWildcardQueryEquals(L"te?m", true, L"te?m");
    checkWildcardQueryEquals(L"Te?m", true, L"te?m");
    checkWildcardQueryEquals(L"TE?M", true, L"te?m");
    checkWildcardQueryEquals(L"Te?m*gerM", true, L"te?m*germ");
    // explicitly disable lowercase conversion
    checkWildcardQueryEquals(L"te?m", false, L"te?m");
    checkWildcardQueryEquals(L"Te?m", false, L"Te?m");
    checkWildcardQueryEquals(L"TE?M", false, L"TE?M");
    checkWildcardQueryEquals(L"Te?m*gerM", false, L"Te?m*gerM");
    //  Fuzzy queries
    checkWildcardQueryEquals(L"Term~", L"term~0.5");
    checkWildcardQueryEquals(L"Term~", true, L"term~0.5");
    checkWildcardQueryEquals(L"Term~", false, L"Term~0.5");
    //  Range queries
    checkWildcardQueryEquals(L"[A TO C]", L"[a TO c]");
    checkWildcardQueryEquals(L"[A TO C]", true, L"[a TO c]");
    checkWildcardQueryEquals(L"[A TO C]", false, L"[A TO C]");
    // Test suffix queries: first disallow
    BOOST_CHECK_EXCEPTION(checkWildcardQueryEquals(L"*Term", true, L"*term"), QueryParserError, check_exception(LuceneException::QueryParser));
    BOOST_CHECK_EXCEPTION(checkWildcardQueryEquals(L"?Term", true, L"?term"), QueryParserError, check_exception(LuceneException::QueryParser));

    // Test suffix queries: then allow
    checkWildcardQueryEquals(L"*Term", true, L"*term", true);
    checkWildcardQueryEquals(L"?Term", true, L"?term", true);
}

BOOST_AUTO_TEST_CASE(testLeadingWildcardType)
{
    QueryParserPtr qp = getParser(AnalyzerPtr());
    qp->setAllowLeadingWildcard(true);
    BOOST_CHECK(MiscUtils::typeOf<WildcardQuery>(qp->parse(L"t*erm*")));
    BOOST_CHECK(MiscUtils::typeOf<WildcardQuery>(qp->parse(L"?term*")));
    BOOST_CHECK(MiscUtils::typeOf<WildcardQuery>(qp->parse(L"*term*")));
}

BOOST_AUTO_TEST_CASE(testQPA)
{
    AnalyzerPtr qpAnalyzer = newLucene<TestAnalyzer>();
    
    checkQueryEquals(L"term term^3.0 term", qpAnalyzer, L"term term^3.0 term");
    checkQueryEquals(L"term stop^3.0 term", qpAnalyzer, L"term term");

    checkQueryEquals(L"term term term", qpAnalyzer, L"term term term");
    checkQueryEquals(L"term +stop term", qpAnalyzer, L"term term");
    checkQueryEquals(L"term -stop term", qpAnalyzer, L"term term");

    checkQueryEquals(L"drop AND (stop) AND roll", qpAnalyzer, L"+drop +roll");
    checkQueryEquals(L"term +(stop) term", qpAnalyzer, L"term term");
    checkQueryEquals(L"term -(stop) term", qpAnalyzer, L"term term");

    checkQueryEquals(L"drop AND stop AND roll", qpAnalyzer, L"+drop +roll");
    checkQueryEquals(L"term phrase term", qpAnalyzer, L"term \"phrase1 phrase2\" term");
    checkQueryEquals(L"term AND NOT phrase term", qpAnalyzer, L"+term -\"phrase1 phrase2\" term");
    checkQueryEquals(L"stop^3", qpAnalyzer, L"");
    checkQueryEquals(L"stop", qpAnalyzer, L"");
    checkQueryEquals(L"(stop)^3", qpAnalyzer, L"");
    checkQueryEquals(L"((stop))^3", qpAnalyzer, L"");
    checkQueryEquals(L"(stop^3)", qpAnalyzer, L"");
    checkQueryEquals(L"((stop)^3)", qpAnalyzer, L"");
    checkQueryEquals(L"(stop)", qpAnalyzer, L"");
    checkQueryEquals(L"((stop))", qpAnalyzer, L"");
    BOOST_CHECK(MiscUtils::typeOf<BooleanQuery>(getQuery(L"term term term", qpAnalyzer)));
    BOOST_CHECK(MiscUtils::typeOf<TermQuery>(getQuery(L"term +stop", qpAnalyzer)));
}

BOOST_AUTO_TEST_CASE(testRange)
{
    checkQueryEquals(L"[ a TO z]", AnalyzerPtr(), L"[a TO z]");
    BOOST_CHECK_EQUAL(MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT(), boost::dynamic_pointer_cast<TermRangeQuery>(getQuery(L"[ a TO z]", AnalyzerPtr()))->getRewriteMethod());

    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<SimpleAnalyzer>());
    qp->setMultiTermRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE());
    BOOST_CHECK_EQUAL(MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE(), boost::dynamic_pointer_cast<TermRangeQuery>(qp->parse(L"[ a TO z]"))->getRewriteMethod());

    checkQueryEquals(L"[ a TO z ]", AnalyzerPtr(), L"[a TO z]");
    checkQueryEquals(L"{ a TO z}", AnalyzerPtr(), L"{a TO z}");
    checkQueryEquals(L"{ a TO z }", AnalyzerPtr(), L"{a TO z}");
    checkQueryEquals(L"{ a TO z }^2.0", AnalyzerPtr(), L"{a TO z}^2.0");
    checkQueryEquals(L"[ a TO z] OR bar", AnalyzerPtr(), L"[a TO z] bar");
    checkQueryEquals(L"[ a TO z] AND bar", AnalyzerPtr(), L"+[a TO z] +bar");
    checkQueryEquals(L"( bar blar { a TO z}) ", AnalyzerPtr(), L"bar blar {a TO z}");
    checkQueryEquals(L"gack ( bar blar { a TO z}) ", AnalyzerPtr(), L"gack (bar blar {a TO z})");
}

BOOST_AUTO_TEST_CASE(testLegacyDateRange)
{
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    checkQueryEquals(L"[01/02/02 TO 04/02/02]", AnalyzerPtr(), L"[0cx597uo0 TO 0cxayz9bz]");
    checkQueryEquals(L"{01/02/02 04/02/02}", AnalyzerPtr(), L"{0cx597uo0 TO 0cx9jjeo0}");
}

BOOST_AUTO_TEST_CASE(testDateRange)
{
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<SimpleAnalyzer>());

    // Don't set any date resolution and verify if DateField is used
    checkQueryEquals(qp, L"default", L"default:[01/02/02 TO 04/02/02]", L"[0cx597uo0 TO 0cxayz9bz]");
    checkQueryEquals(qp, L"default", L"default:{01/02/02 TO 04/02/02}", L"{0cx597uo0 TO 0cx9jjeo0}");

    // set a field specific date resolution
    qp->setDateResolution(L"month", DateTools::RESOLUTION_MONTH);

    // DateField should still be used for defaultField
    checkQueryEquals(qp, L"default", L"default:[01/02/02 TO 04/02/02]", L"[0cx597uo0 TO 0cxayz9bz]");
    checkQueryEquals(qp, L"default", L"default:{01/02/02 TO 04/02/02}", L"{0cx597uo0 TO 0cx9jjeo0}");
    
    // set default date resolution to MILLISECOND
    qp->setDateResolution(DateTools::RESOLUTION_MILLISECOND);

    // set second field specific date resolution    
    qp->setDateResolution(L"hour", DateTools::RESOLUTION_HOUR);

    // for this field no field specific date resolution has been set, so verify if the default resolution is used
    checkQueryEquals(qp, L"default", L"default:[01/02/02 TO 04/02/02]", L"[20020201000000000 TO 20020204235959999]");
    checkQueryEquals(qp, L"default", L"default:{01/02/02 TO 04/02/02}", L"{20020201000000000 TO 20020204000000000}");

    // verify if field specific date resolutions are used for these two fields
    checkQueryEquals(qp, L"month", L"month:[01/02/02 TO 04/02/02]", L"[200202 TO 200202]");
    checkQueryEquals(qp, L"month", L"month:{01/02/02 TO 04/02/02}", L"{200202 TO 200202}");
    
    checkQueryEquals(qp, L"hour", L"hour:[01/02/02 TO 04/02/02]", L"[2002020100 TO 2002020423]");
    checkQueryEquals(qp, L"hour", L"hour:{01/02/02 TO 04/02/02}", L"{2002020100 TO 2002020400}");
}

BOOST_AUTO_TEST_CASE(testEscaped)
{
    AnalyzerPtr a = newLucene<WhitespaceAnalyzer>();

    checkQueryEquals(L"\\a", a, L"a");

    checkQueryEquals(L"a\\-b:c", a, L"a-b:c");
    checkQueryEquals(L"a\\+b:c", a, L"a+b:c");
    checkQueryEquals(L"a\\:b:c", a, L"a:b:c");
    checkQueryEquals(L"a\\\\b:c", a, L"a\\b:c");

    checkQueryEquals(L"a:b\\-c", a, L"a:b-c");
    checkQueryEquals(L"a:b\\+c", a, L"a:b+c");
    checkQueryEquals(L"a:b\\:c", a, L"a:b:c");
    checkQueryEquals(L"a:b\\\\c", a, L"a:b\\c");

    checkQueryEquals(L"a:b\\-c*", a, L"a:b-c*");
    checkQueryEquals(L"a:b\\+c*", a, L"a:b+c*");
    checkQueryEquals(L"a:b\\:c*", a, L"a:b:c*");

    checkQueryEquals(L"a:b\\\\c*", a, L"a:b\\c*");

    checkQueryEquals(L"a:b\\-?c", a, L"a:b-?c");
    checkQueryEquals(L"a:b\\+?c", a, L"a:b+?c");
    checkQueryEquals(L"a:b\\:?c", a, L"a:b:?c");

    checkQueryEquals(L"a:b\\\\?c", a, L"a:b\\?c");

    checkQueryEquals(L"a:b\\-c~", a, L"a:b-c~0.5");
    checkQueryEquals(L"a:b\\+c~", a, L"a:b+c~0.5");
    checkQueryEquals(L"a:b\\:c~", a, L"a:b:c~0.5");
    checkQueryEquals(L"a:b\\\\c~", a, L"a:b\\c~0.5");

    checkQueryEquals(L"[ a\\- TO a\\+ ]", AnalyzerPtr(), L"[a- TO a+]");
    checkQueryEquals(L"[ a\\: TO a\\~ ]", AnalyzerPtr(), L"[a: TO a~]");
    checkQueryEquals(L"[ a\\\\ TO a\\* ]", AnalyzerPtr(), L"[a\\ TO a*]");

    checkQueryEquals(L"[\"c\\:\\\\temp\\\\\\~foo0.txt\" TO \"c\\:\\\\temp\\\\\\~foo9.txt\"]", a, L"[c:\\temp\\~foo0.txt TO c:\\temp\\~foo9.txt]");

    checkQueryEquals(L"a\\\\\\+b", a, L"a\\+b");

    checkQueryEquals(L"a \\\"b c\\\" d", a, L"a \"b c\" d");
    checkQueryEquals(L"\"a \\\"b c\\\" d\"", a, L"\"a \"b c\" d\"");
    checkQueryEquals(L"\"a \\+b c d\"", a, L"\"a +b c d\"");

    checkQueryEquals(L"c\\:\\\\temp\\\\\\~foo.txt", a, L"c:\\temp\\~foo.txt");

    checkParseException(L"XY\\"); // there must be a character after the escape char

    // test unicode escaping
    checkQueryEquals(L"a\\u0062c", a, L"abc");
    checkQueryEquals(L"XY\\u005a", a, L"XYZ");
    checkQueryEquals(L"XY\\u005A", a, L"XYZ");
    checkQueryEquals(L"\"a \\\\\\u0028\\u0062\\\" c\"", a, L"\"a \\(b\" c\"");

    checkParseException(L"XY\\u005G"); // test non-hex character in escaped unicode sequence
    checkParseException(L"XY\\u005"); // test incomplete escaped unicode sequence

    checkQueryEquals(L"(item:\\\\ item:ABCD\\\\)", a, L"item:\\ item:ABCD\\");
    checkParseException(L"(item:\\\\ item:ABCD\\\\))"); // unmatched closing parenthesis 
    checkQueryEquals(L"\\*", a, L"*");
    checkQueryEquals(L"\\\\", a, L"\\"); // escaped backslash

    checkParseException(L"\\"); // a backslash must always be escaped

    checkQueryEquals(L"(\"a\\\\\") or (\"b\")", a, L"a\\ or b");
}

BOOST_AUTO_TEST_CASE(testQueryStringEscaping)
{
    AnalyzerPtr a = newLucene<WhitespaceAnalyzer>();

    checkEscapedQueryEquals(L"a-b:c", a, L"a\\-b\\:c");
    checkEscapedQueryEquals(L"a+b:c", a, L"a\\+b\\:c");
    checkEscapedQueryEquals(L"a:b:c", a, L"a\\:b\\:c");
    checkEscapedQueryEquals(L"a\\b:c", a, L"a\\\\b\\:c");

    checkEscapedQueryEquals(L"a:b-c", a, L"a\\:b\\-c");
    checkEscapedQueryEquals(L"a:b+c", a, L"a\\:b\\+c");
    checkEscapedQueryEquals(L"a:b:c", a, L"a\\:b\\:c");
    checkEscapedQueryEquals(L"a:b\\c", a, L"a\\:b\\\\c");

    checkEscapedQueryEquals(L"a:b-c*", a, L"a\\:b\\-c\\*");
    checkEscapedQueryEquals(L"a:b+c*", a, L"a\\:b\\+c\\*");
    checkEscapedQueryEquals(L"a:b:c*", a, L"a\\:b\\:c\\*");

    checkEscapedQueryEquals(L"a:b\\\\c*", a, L"a\\:b\\\\\\\\c\\*");

    checkEscapedQueryEquals(L"a:b-?c", a, L"a\\:b\\-\\?c");
    checkEscapedQueryEquals(L"a:b+?c", a, L"a\\:b\\+\\?c");
    checkEscapedQueryEquals(L"a:b:?c", a, L"a\\:b\\:\\?c");

    checkEscapedQueryEquals(L"a:b?c", a, L"a\\:b\\?c");

    checkEscapedQueryEquals(L"a:b-c~", a, L"a\\:b\\-c\\~");
    checkEscapedQueryEquals(L"a:b+c~", a, L"a\\:b\\+c\\~");
    checkEscapedQueryEquals(L"a:b:c~", a, L"a\\:b\\:c\\~");
    checkEscapedQueryEquals(L"a:b\\c~", a, L"a\\:b\\\\c\\~");

    checkEscapedQueryEquals(L"[ a - TO a+ ]", AnalyzerPtr(), L"\\[ a \\- TO a\\+ \\]");
    checkEscapedQueryEquals(L"[ a : TO a~ ]", AnalyzerPtr(), L"\\[ a \\: TO a\\~ \\]");
    checkEscapedQueryEquals(L"[ a\\ TO a* ]", AnalyzerPtr(), L"\\[ a\\\\ TO a\\* \\]");

    checkEscapedQueryEquals(L"|| abc ||", a, L"\\|\\| abc \\|\\|");
    checkEscapedQueryEquals(L"&& abc &&", a, L"\\&\\& abc \\&\\&");
}

BOOST_AUTO_TEST_CASE(testTabNewlineCarriageReturn)
{
    checkQueryEqualsDOA(L"+weltbank +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");

    checkQueryEqualsDOA(L"+weltbank\n+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \n+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \n +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");

    checkQueryEqualsDOA(L"+weltbank\r+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \r+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \r +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");

    checkQueryEqualsDOA(L"+weltbank\r\n+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \r\n+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \r\n +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \r \n +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");

    checkQueryEqualsDOA(L"+weltbank\t+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \t+worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
    checkQueryEqualsDOA(L"weltbank \t +worlbank", AnalyzerPtr(), L"+weltbank +worlbank");
}

BOOST_AUTO_TEST_CASE(testSimpleDAO)
{
    checkQueryEqualsDOA(L"term term term", AnalyzerPtr(), L"+term +term +term");
    checkQueryEqualsDOA(L"term +term term", AnalyzerPtr(), L"+term +term +term");
    checkQueryEqualsDOA(L"term term +term", AnalyzerPtr(), L"+term +term +term");
    checkQueryEqualsDOA(L"term +term +term", AnalyzerPtr(), L"+term +term +term");
    checkQueryEqualsDOA(L"-term term term", AnalyzerPtr(), L"-term +term +term");
}

BOOST_AUTO_TEST_CASE(testBoost)
{
    HashSet<String> stopWords = HashSet<String>::newInstance();
    stopWords.add(L"on");
    StandardAnalyzerPtr oneStopAnalyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT, stopWords);
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", oneStopAnalyzer);
    QueryPtr q = qp->parse(L"on^1.0");
    BOOST_CHECK(q);
    q = qp->parse(L"\"hello\"^2.0");
    BOOST_CHECK(q);
    BOOST_CHECK_CLOSE_FRACTION(q->getBoost(), 2.0, 0.5);
    q = qp->parse(L"hello^2.0");
    BOOST_CHECK(q);
    BOOST_CHECK_CLOSE_FRACTION(q->getBoost(), 2.0, 0.5);
    q = qp->parse(L"\"on\"^1.0");
    BOOST_CHECK(q);

    QueryParserPtr qp2 = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT));
    q = qp2->parse(L"the^3");
    
    // "the" is a stop word so the result is an empty query
    BOOST_CHECK(q);
    BOOST_CHECK(q->toString().empty());
    BOOST_CHECK_CLOSE_FRACTION(1.0, q->getBoost(), 0.01);
}

BOOST_AUTO_TEST_CASE(testException)
{
    checkParseException(L"\"some phrase");
    checkParseException(L"(foo bar");
    checkParseException(L"foo bar))");
    checkParseException(L"field:term:with:colon some more terms");
    checkParseException(L"(sub query)^5.0^2.0 plus more");
    checkParseException(L"secret AND illegal) AND access:confidential");
}

BOOST_AUTO_TEST_CASE(testCustomQueryParserWildcard)
{
    BOOST_CHECK_EXCEPTION(newLucene<TestParser>(L"contents", newLucene<WhitespaceAnalyzer>())->parse(L"a?t"), QueryParserError, check_exception(LuceneException::QueryParser));
}

BOOST_AUTO_TEST_CASE(testCustomQueryParserFuzzy)
{
    BOOST_CHECK_EXCEPTION(newLucene<TestParser>(L"contents", newLucene<WhitespaceAnalyzer>())->parse(L"xunit~"), QueryParserError, check_exception(LuceneException::QueryParser));
}

BOOST_AUTO_TEST_CASE(testBooleanQuery)
{
    BooleanQuery::setMaxClauseCount(2);
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<WhitespaceAnalyzer>());
    
    // too many boolean clauses, so ParseException is expected
    BOOST_CHECK_EXCEPTION(qp->parse(L"one two three"), QueryParserError, check_exception(LuceneException::QueryParser));
}

BOOST_AUTO_TEST_CASE(testPrecedence)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<WhitespaceAnalyzer>());
    QueryPtr query1 = qp->parse(L"A AND B OR C AND D");
    QueryPtr query2 = qp->parse(L"+A +B +C +D");
    BOOST_CHECK(query1->equals(query2));
}

BOOST_AUTO_TEST_CASE(testLocalDateFormat)
{
    DateTools::setDateOrder(DateTools::DATEORDER_DMY);
    RAMDirectoryPtr ramDir = newLucene<RAMDirectory>();
    IndexWriterPtr iw = newLucene<IndexWriter>(ramDir, newLucene<WhitespaceAnalyzer>(), true, IndexWriter::MaxFieldLengthLIMITED);
    addDateDoc(L"a", ptime(date(2005, Dec, 2), hours(10) + minutes(15) + seconds(33)), iw);
    addDateDoc(L"a", ptime(date(2005, Dec, 4), hours(22) + minutes(15) + seconds(00)), iw);
    iw->close();
    IndexSearcherPtr is = newLucene<IndexSearcher>(ramDir, true);
    checkHits(1, L"[1/12/2005 TO 3/12/2005]", is);
    checkHits(2, L"[1/12/2005 TO 4/12/2005]", is);
    checkHits(1, L"[3/12/2005 TO 4/12/2005]", is);
    checkHits(1, L"{1/12/2005 TO 3/12/2005}", is);
    checkHits(1, L"{1/12/2005 TO 4/12/2005}", is);
    checkHits(0, L"{3/12/2005 TO 4/12/2005}", is);
    is->close();
}

namespace TestStarParsing
{
    DECLARE_SHARED_PTR(StarParser)
    
    class StarParser : public QueryParser
    {
    public:
        StarParser(const String& f, AnalyzerPtr a) : QueryParser(LuceneVersion::LUCENE_CURRENT, f, a)
        {
            type = Collection<int32_t>::newInstance(1);
        }
        
        virtual ~StarParser()
        {
        }
        
        LUCENE_CLASS(StarParser);
    
    public:
        Collection<int32_t> type;

    public:
        virtual QueryPtr getWildcardQuery(const String& field, const String& termStr)
        {
            // override error checking of superclass
            type[0] = 1;
            return newLucene<TermQuery>(newLucene<Term>(field, termStr));
        }
        
        virtual QueryPtr getPrefixQuery(const String& field, const String& termStr)
        {
            // override error checking of superclass
            type[0] = 2;
            return newLucene<TermQuery>(newLucene<Term>(field, termStr));
        }
        
        virtual QueryPtr getFieldQuery(const String& field, const String& queryText)
        {
            type[0] = 3;
            return QueryParser::getFieldQuery(field, queryText);
        }
    };
}

BOOST_AUTO_TEST_CASE(testStarParsing)
{
    TestStarParsing::StarParserPtr qp = newLucene<TestStarParsing::StarParser>(L"field", newLucene<WhitespaceAnalyzer>());

    TermQueryPtr tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"foo:zoo*"));
    BOOST_CHECK_EQUAL(L"zoo", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(2, qp->type[0]);

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"foo:zoo*^2"));
    BOOST_CHECK_EQUAL(L"zoo", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(2, qp->type[0]);
    BOOST_CHECK_EQUAL(tq->getBoost(), 2);

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"foo:*"));
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(1, qp->type[0]);  // could be a valid prefix query in the future too

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"foo:*^2"));
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(1, qp->type[0]);
    BOOST_CHECK_EQUAL(tq->getBoost(), 2);    

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"*:foo"));
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->field());
    BOOST_CHECK_EQUAL(L"foo", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(3, qp->type[0]);

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"*:*"));
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->field());
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(1, qp->type[0]);  // could be handled as a prefix query in the future

    tq = boost::dynamic_pointer_cast<TermQuery>(qp->parse(L"(*:*)"));
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->field());
    BOOST_CHECK_EQUAL(L"*", tq->getTerm()->text());
    BOOST_CHECK_EQUAL(1, qp->type[0]);
}

BOOST_AUTO_TEST_CASE(testStopwords)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"a", newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT, StopFilter::makeStopSet(newCollection<String>(L"the", L"foo"))));
    QueryPtr result = qp->parse(L"a:the OR a:foo");
    BOOST_CHECK(result);
    BOOST_CHECK(MiscUtils::typeOf<BooleanQuery>(result));
    BOOST_CHECK(boost::dynamic_pointer_cast<BooleanQuery>(result)->getClauses().empty());
    result = qp->parse(L"a:woo OR a:the");
    BOOST_CHECK(result);
    BOOST_CHECK(MiscUtils::typeOf<TermQuery>(result));
    result = qp->parse(L"(fieldX:xxxxx OR fieldy:xxxxxxxx)^2 AND (fieldx:the OR fieldy:foo)");
    BOOST_CHECK(result);
    BOOST_CHECK(MiscUtils::typeOf<BooleanQuery>(result));
    BOOST_CHECK_EQUAL(boost::dynamic_pointer_cast<BooleanQuery>(result)->getClauses().size(), 2);
}

BOOST_AUTO_TEST_CASE(testPositionIncrement)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"a", newLucene<StopAnalyzer>(LuceneVersion::LUCENE_CURRENT, StopFilter::makeStopSet(newCollection<String>(L"the", L"in", L"are", L"this"))));
    qp->setEnablePositionIncrements(true);
    String qtxt = L"\"the words in positions pos02578 are stopped in this phrasequery\"";
    //                0         2                     5           7  8
    Collection<int32_t> expectedPositions = newCollection<int32_t>(1, 3, 4, 6, 9);
    PhraseQueryPtr pq = boost::dynamic_pointer_cast<PhraseQuery>(qp->parse(qtxt));
    Collection<TermPtr> t = pq->getTerms();
    Collection<int32_t> pos = pq->getPositions();
    for (int32_t i = 0; i < t.size(); ++i)
        BOOST_CHECK_EQUAL(expectedPositions[i], pos[i]);
}

BOOST_AUTO_TEST_CASE(testMatchAllDocs)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"field", newLucene<WhitespaceAnalyzer>());
    BOOST_CHECK(newLucene<MatchAllDocsQuery>()->equals(qp->parse(L"*:*")));
    BOOST_CHECK(newLucene<MatchAllDocsQuery>()->equals(qp->parse(L"(*:*)")));
    BooleanQueryPtr bq = boost::dynamic_pointer_cast<BooleanQuery>(qp->parse(L"+*:* -*:*"));
    BOOST_CHECK(MiscUtils::typeOf<MatchAllDocsQuery>(bq->getClauses()[0]->getQuery()));
    BOOST_CHECK(MiscUtils::typeOf<MatchAllDocsQuery>(bq->getClauses()[1]->getQuery()));
}

BOOST_AUTO_TEST_CASE(testPositionIncrements)
{
    DirectoryPtr dir = newLucene<MockRAMDirectory>();
    AnalyzerPtr a = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    IndexWriterPtr w = newLucene<IndexWriter>(dir, a, IndexWriter::MaxFieldLengthUNLIMITED);
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"f", L"the wizard of ozzy", Field::STORE_NO, Field::INDEX_ANALYZED));
    w->addDocument(doc);
    IndexReaderPtr r = w->getReader();
    w->close();
    IndexSearcherPtr s = newLucene<IndexSearcher>(r);
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"f", a);
    QueryPtr q = qp->parse(L"\"wizard of ozzy\"");
    BOOST_CHECK_EQUAL(1, s->search(q, 1)->totalHits);
    r->close();
    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
