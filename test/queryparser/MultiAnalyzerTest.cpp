/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "QueryParser.h"
#include "Analyzer.h"
#include "StandardTokenizer.h"
#include "LowerCaseFilter.h"
#include "TokenFilter.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "TypeAttribute.h"
#include "Query.h"

using namespace Lucene;

/// Test QueryParser's ability to deal with Analyzers that return more than one token per 
/// position or that return tokens with a position increment > 1.
BOOST_FIXTURE_TEST_SUITE(MultiAnalyzerTest, BaseTokenStreamFixture)

static int32_t multiToken = 0;

DECLARE_SHARED_PTR(MultiAnalyzer)
DECLARE_SHARED_PTR(TestFilter)
DECLARE_SHARED_PTR(DumbQueryWrapper)
DECLARE_SHARED_PTR(DumbQueryParser)
DECLARE_SHARED_PTR(TestPosIncrementFilter)

class TestFilter : public TokenFilter
{
public:
    TestFilter(TokenStreamPtr in) : TokenFilter(in)
    {
        prevStartOffset = 0;
        prevEndOffset = 0;
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
        offsetAtt = addAttribute<OffsetAttribute>();
        typeAtt = addAttribute<TypeAttribute>();
    }
    
    virtual ~TestFilter()
    {
    }
    
    LUCENE_CLASS(TestFilter);

protected:
    String prevType;
    int32_t prevStartOffset;
    int32_t prevEndOffset;

    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;
    OffsetAttributePtr offsetAtt;
    TypeAttributePtr typeAtt;

public:
    virtual bool incrementToken()
    {
        if (multiToken > 0)
        {
            termAtt->setTermBuffer(L"multi" + StringUtils::toString(multiToken + 1));
            offsetAtt->setOffset(prevStartOffset, prevEndOffset);
            typeAtt->setType(prevType);
            posIncrAtt->setPositionIncrement(0);
            --multiToken;
            return true;
        }
        else
        {
            bool next = input->incrementToken();
            if (!next)
                return false;
            prevType = typeAtt->type();
            prevStartOffset = offsetAtt->startOffset();
            prevEndOffset = offsetAtt->endOffset();
            String text = termAtt->term();
            if (text == L"triplemulti")
            {
                multiToken = 2;
                return true;
            }
            else if (text == L"multi")
            {
                multiToken = 1;
                return true;
            }
            else
                return true;
        }
    }
};

class MultiAnalyzer : public Analyzer
{
public:
    virtual ~MultiAnalyzer()
    {
    }
    
    LUCENE_CLASS(MultiAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenStreamPtr result = newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, reader);
        result = newLucene<TestFilter>(result);
        result = newLucene<LowerCaseFilter>(result);
        return result;
    }
};

/// A very simple wrapper to prevent typeOf checks but uses the toString of the query it wraps.
class DumbQueryWrapper : public Query
{
public:
    DumbQueryWrapper(QueryPtr q)
    {
        this->q = q;
    }
    
    virtual ~DumbQueryWrapper()
    {
    }
    
    LUCENE_CLASS(DumbQueryWrapper);

protected:
    QueryPtr q;

public:
    virtual String toString(const String& field)
    {
        return q->toString(field);
    }
};

/// A very simple subclass of QueryParser
class DumbQueryParser : public QueryParser
{
public:
    DumbQueryParser(const String& f, AnalyzerPtr a) : QueryParser(LuceneVersion::LUCENE_CURRENT, f, a)
    {
    }
    
    virtual ~DumbQueryParser()
    {
    }
    
    LUCENE_CLASS(DumbQueryParser);

public:
    virtual QueryPtr getFieldQuery(const String& field, const String& queryText)
    {
        return newLucene<DumbQueryWrapper>(QueryParser::getFieldQuery(field, queryText));
    }
};

class TestPosIncrementFilter : public TokenFilter
{
public:
    TestPosIncrementFilter(TokenStreamPtr in) : TokenFilter(in)
    {
        termAtt = addAttribute<TermAttribute>();
        posIncrAtt = addAttribute<PositionIncrementAttribute>();
    }
    
    virtual ~TestPosIncrementFilter()
    {
    }
    
    LUCENE_CLASS(TestPosIncrementFilter);

protected:
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncrAtt;

public:
    virtual bool incrementToken()
    {
        while (input->incrementToken())
        {
            if (termAtt->term() == L"the")
            {
                // stopword, do nothing
            }
            else if (termAtt->term() == L"quick")
            {
                posIncrAtt->setPositionIncrement(2);
                return true;
            }
            else
            {
                posIncrAtt->setPositionIncrement(1);
                return true;
            }
        }
        return false;
    }
};

/// Analyzes "the quick brown" as: quick(incr=2) brown(incr=1).
/// Does not work correctly for input other than "the quick brown ...".
class PosIncrementAnalyzer : public Analyzer
{
public:
    virtual ~PosIncrementAnalyzer()
    {
    }
    
    LUCENE_CLASS(PosIncrementAnalyzer);

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenStreamPtr result = newLucene<StandardTokenizer>(LuceneVersion::LUCENE_CURRENT, reader);
        result = newLucene<TestPosIncrementFilter>(result);
        result = newLucene<LowerCaseFilter>(result);
        return result;
    }
};

BOOST_AUTO_TEST_CASE(testMultiAnalyzer)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"", newLucene<MultiAnalyzer>());

    // trivial, no multiple tokens
    BOOST_CHECK_EQUAL(L"foo", qp->parse(L"foo")->toString());
    BOOST_CHECK_EQUAL(L"foo", qp->parse(L"\"foo\"")->toString());
    BOOST_CHECK_EQUAL(L"foo foobar", qp->parse(L"foo foobar")->toString());
    BOOST_CHECK_EQUAL(L"\"foo foobar\"", qp->parse(L"\"foo foobar\"")->toString());
    BOOST_CHECK_EQUAL(L"\"foo foobar blah\"", qp->parse(L"\"foo foobar blah\"")->toString());

    // two tokens at the same position
    BOOST_CHECK_EQUAL(L"(multi multi2) foo", qp->parse(L"multi foo")->toString());
    BOOST_CHECK_EQUAL(L"foo (multi multi2)", qp->parse(L"foo multi")->toString());
    BOOST_CHECK_EQUAL(L"(multi multi2) (multi multi2)", qp->parse(L"multi multi")->toString());
    BOOST_CHECK_EQUAL(L"+(foo (multi multi2)) +(bar (multi multi2))", qp->parse(L"+(foo multi) +(bar multi)")->toString());
    BOOST_CHECK_EQUAL(L"+(foo (multi multi2)) field:\"bar (multi multi2)\"", qp->parse(L"+(foo multi) field:\"bar multi\"")->toString());

    // phrases
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"", qp->parse(L"\"multi foo\"")->toString());
    BOOST_CHECK_EQUAL(L"\"foo (multi multi2)\"", qp->parse(L"\"foo multi\"")->toString());
    BOOST_CHECK_EQUAL(L"\"foo (multi multi2) foobar (multi multi2)\"", qp->parse(L"\"foo multi foobar multi\"")->toString());

    // fields
    BOOST_CHECK_EQUAL(L"(field:multi field:multi2) field:foo", qp->parse(L"field:multi field:foo")->toString());
    BOOST_CHECK_EQUAL(L"field:\"(multi multi2) foo\"", qp->parse(L"field:\"multi foo\"")->toString());

    // three tokens at one position
    BOOST_CHECK_EQUAL(L"triplemulti multi3 multi2", qp->parse(L"triplemulti")->toString());
    BOOST_CHECK_EQUAL(L"foo (triplemulti multi3 multi2) foobar", qp->parse(L"foo triplemulti foobar")->toString());

    // phrase with non-default slop
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"~10", qp->parse(L"\"multi foo\"~10")->toString());
    
    // phrase with non-default boost
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"^2.0", qp->parse(L"\"multi foo\"^2")->toString());

    // phrase after changing default slop
    qp->setPhraseSlop(99);
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"~99 bar", qp->parse(L"\"multi foo\" bar")->toString());
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"~99 \"foo bar\"~2", qp->parse(L"\"multi foo\" \"foo bar\"~2")->toString());
    qp->setPhraseSlop(0);

    // non-default operator
    qp->setDefaultOperator(QueryParser::AND_OPERATOR);
    BOOST_CHECK_EQUAL(L"+(multi multi2) +foo", qp->parse(L"multi foo")->toString());
}

BOOST_AUTO_TEST_CASE(testMultiAnalyzerWithSubclassOfQueryParser)
{
    DumbQueryParserPtr qp = newLucene<DumbQueryParser>(L"", newLucene<MultiAnalyzer>());

    qp->setPhraseSlop(99); // modified default slop

    // direct call to getFieldQuery to demonstrate difference between phrase and multiphrase with modified default slop
    BOOST_CHECK_EQUAL(L"\"foo bar\"~99", qp->getFieldQuery(L"", L"foo bar")->toString());
    BOOST_CHECK_EQUAL(L"\"(multi multi2) bar\"~99", qp->getFieldQuery(L"", L"multi bar")->toString());

    // ask subclass to parse phrase with modified default slop
    BOOST_CHECK_EQUAL(L"\"(multi multi2) foo\"~99 bar", qp->parse(L"\"multi foo\" bar")->toString());
}

BOOST_AUTO_TEST_CASE(testPosIncrementAnalyzer)
{
    QueryParserPtr qp = newLucene<QueryParser>(LuceneVersion::LUCENE_24, L"", newLucene<PosIncrementAnalyzer>());
    BOOST_CHECK_EQUAL(L"quick brown", qp->parse(L"the quick brown")->toString());
    BOOST_CHECK_EQUAL(L"\"quick brown\"", qp->parse(L"\"the quick brown\"")->toString());
    BOOST_CHECK_EQUAL(L"quick brown fox", qp->parse(L"the quick brown fox")->toString());
    BOOST_CHECK_EQUAL(L"\"quick brown fox\"", qp->parse(L"\"the quick brown fox\"")->toString());
}

BOOST_AUTO_TEST_SUITE_END()
