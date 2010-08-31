/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseTokenStreamFixture.h"
#include "BrazilianAnalyzer.h"

using namespace Lucene;

class BrazilianStemmerFixture : public BaseTokenStreamFixture
{
public:
	virtual ~BrazilianStemmerFixture()
	{
	}

public:
	void check(const String& input, const String& expected)
    {
        checkOneTerm(newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT), input, expected);
    }
    
    void checkReuse(AnalyzerPtr a, const String& input, const String& expected)
    {
        checkOneTermReuse(a, input, expected);
    }
};

BOOST_FIXTURE_TEST_SUITE(BrazilianStemmerTest, BrazilianStemmerFixture)

/// Test the Brazilian Stem Filter, which only modifies the term text.
/// It is very similar to the snowball Portuguese algorithm but not exactly the same.

BOOST_AUTO_TEST_CASE(testWithSnowballExamples)
{
    check(L"boa", L"boa");
    check(L"boainain", L"boainain");
    check(L"boas", L"boas");
    check(L"bôas", L"boas"); // removes diacritic: different from snowball Portuguese
    check(L"boassu", L"boassu");
    check(L"boataria", L"boat");
    check(L"boate", L"boat");
    check(L"boates", L"boat");
    check(L"boatos", L"boat");
    check(L"bob", L"bob");
    check(L"boba", L"bob");
    check(L"bobagem", L"bobag");
    check(L"bobagens", L"bobagens");
    check(L"bobalhões", L"bobalho"); // removes diacritic: different from snowball Portuguese
    check(L"bobear", L"bob");
    check(L"bobeira", L"bobeir");
    check(L"bobinho", L"bobinh");
    check(L"bobinhos", L"bobinh");
    check(L"bobo", L"bob");
    check(L"bobs", L"bobs");
    check(L"boca", L"boc");
    check(L"bocadas", L"boc");
    check(L"bocadinho", L"bocadinh");
    check(L"bocado", L"boc");
    check(L"bocaiúva", L"bocaiuv"); // removes diacritic: different from snowball Portuguese
    check(L"boçal", L"bocal"); // removes diacritic: different from snowball Portuguese
    check(L"bocarra", L"bocarr");
    check(L"bocas", L"boc");
    check(L"bode", L"bod");
    check(L"bodoque", L"bodoqu");
    check(L"body", L"body");
    check(L"boeing", L"boeing");
    check(L"boem", L"boem");
    check(L"boemia", L"boem");
    check(L"boêmio", L"boemi"); // removes diacritic: different from snowball Portuguese
    check(L"bogotá", L"bogot");
    check(L"boi", L"boi");
    check(L"bóia", L"boi"); // removes diacritic: different from snowball Portuguese
    check(L"boiando", L"boi");
    check(L"quiabo", L"quiab");
    check(L"quicaram", L"quic");
    check(L"quickly", L"quickly");
    check(L"quieto", L"quiet");
    check(L"quietos", L"quiet");
    check(L"quilate", L"quilat");
    check(L"quilates", L"quilat");
    check(L"quilinhos", L"quilinh");
    check(L"quilo", L"quil");
    check(L"quilombo", L"quilomb");
    check(L"quilométricas", L"quilometr"); // removes diacritic: different from snowball Portuguese
    check(L"quilométricos", L"quilometr"); // removes diacritic: different from snowball Portuguese
    check(L"quilômetro", L"quilometr"); // removes diacritic: different from snowball Portuguese
    check(L"quilômetros", L"quilometr"); // removes diacritic: different from snowball Portuguese
    check(L"quilos", L"quil");
    check(L"quimica", L"quimic");
    check(L"quilos", L"quil");
    check(L"quimica", L"quimic");
    check(L"quimicas", L"quimic");
    check(L"quimico", L"quimic");
    check(L"quimicos", L"quimic");
    check(L"quimioterapia", L"quimioterap");
    check(L"quimioterápicos", L"quimioterap"); // removes diacritic: different from snowball Portuguese
    check(L"quimono", L"quimon");
    check(L"quincas", L"quinc");
    check(L"quinhão", L"quinha"); // removes diacritic: different from snowball Portuguese
    check(L"quinhentos", L"quinhent");
    check(L"quinn", L"quinn");
    check(L"quino", L"quin");
    check(L"quinta", L"quint");
    check(L"quintal", L"quintal");
    check(L"quintana", L"quintan");
    check(L"quintanilha", L"quintanilh");
    check(L"quintão", L"quinta"); // removes diacritic: different from snowball Portuguese
    check(L"quintessência", L"quintessente"); // versus snowball Portuguese 'quintessent'
    check(L"quintino", L"quintin");
    check(L"quinto", L"quint");
    check(L"quintos", L"quint");
    check(L"quintuplicou", L"quintuplic");
    check(L"quinze", L"quinz");
    check(L"quinzena", L"quinzen");
    check(L"quiosque", L"quiosqu");
}

BOOST_AUTO_TEST_CASE(testNormalization)
{
    check(L"Brasil", L"brasil"); // lowercase by default
    check(L"Brasília", L"brasil"); // remove diacritics
    check(L"quimio5terápicos", L"quimio5terapicos"); // contains non-letter, diacritic will still be removed
    check(L"áá", L"áá"); // token is too short: diacritics are not removed
    check(L"ááá", L"aaa"); // normally, diacritics are removed
}

BOOST_AUTO_TEST_CASE(testReusableTokenStream)
{
    AnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"boa", L"boa");
    checkReuse(a, L"boainain", L"boainain");
    checkReuse(a, L"boas", L"boas");
    checkReuse(a, L"bôas", L"boas"); // removes diacritic: different from snowball Portuguese
}

BOOST_AUTO_TEST_CASE(testStemExclusionTable)
{
    BrazilianAnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(L"quintessência");
    a->setStemExclusionTable(exclusions);
    checkReuse(a, L"quintessência", L"quintessência"); // excluded words will be completely unchanged
}

/// Test that changes to the exclusion table are applied immediately when using reusable token streams.
BOOST_AUTO_TEST_CASE(testExclusionTableReuse)
{
    BrazilianAnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"quintessência", L"quintessente");
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(L"quintessência");
    a->setStemExclusionTable(exclusions);
    checkReuse(a, L"quintessência", L"quintessência");
}

BOOST_AUTO_TEST_SUITE_END()
