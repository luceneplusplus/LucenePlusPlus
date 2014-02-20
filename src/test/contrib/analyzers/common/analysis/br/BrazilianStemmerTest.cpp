/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "BrazilianAnalyzer.h"

using namespace Lucene;

class BrazilianStemmerTest : public BaseTokenStreamFixture {
public:
    virtual ~BrazilianStemmerTest() {
    }

public:
    void check(const String& input, const String& expected) {
        checkOneTerm(newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT), input, expected);
    }

    void checkReuse(const AnalyzerPtr& a, const String& input, const String& expected) {
        checkOneTermReuse(a, input, expected);
    }
};

/// Test the Brazilian Stem Filter, which only modifies the term text.
/// It is very similar to the snowball Portuguese algorithm but not exactly the same.

TEST_F(BrazilianStemmerTest, testWithSnowballExamples) {
    check(L"boa", L"boa");
    check(L"boainain", L"boainain");
    check(L"boas", L"boas");
    const uint8_t boas[] = {0x62, 0xc3, 0xb4, 0x61, 0x73};
    check(UTF8_TO_STRING(boas), L"boas"); // removes diacritic: different from snowball Portuguese
    check(L"boassu", L"boassu");
    check(L"boataria", L"boat");
    check(L"boate", L"boat");
    check(L"boates", L"boat");
    check(L"boatos", L"boat");
    check(L"bob", L"bob");
    check(L"boba", L"bob");
    check(L"bobagem", L"bobag");
    check(L"bobagens", L"bobagens");
    const uint8_t bobalho[] = {0x62, 0x6f, 0x62, 0x61, 0x6c, 0x68, 0xc3, 0xb5, 0x65, 0x73};
    check(UTF8_TO_STRING(bobalho), L"bobalho"); // removes diacritic: different from snowball Portuguese
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
    const uint8_t bocaiuv[] = {0x62, 0x6f, 0x63, 0x61, 0x69, 0xc3, 0xba, 0x76, 0x61};
    check(UTF8_TO_STRING(bocaiuv), L"bocaiuv"); // removes diacritic: different from snowball Portuguese
    const uint8_t bocal[] = {0x62, 0x6f, 0xc3, 0xa7, 0x61, 0x6c};
    check(UTF8_TO_STRING(bocal), L"bocal"); // removes diacritic: different from snowball Portuguese
    check(L"bocarra", L"bocarr");
    check(L"bocas", L"boc");
    check(L"bode", L"bod");
    check(L"bodoque", L"bodoqu");
    check(L"body", L"body");
    check(L"boeing", L"boeing");
    check(L"boem", L"boem");
    check(L"boemia", L"boem");
    const uint8_t boemi[] = {0x62, 0x6f, 0xc3, 0xaa, 0x6d, 0x69, 0x6f};
    check(UTF8_TO_STRING(boemi), L"boemi"); // removes diacritic: different from snowball Portuguese
    const uint8_t bogot[] = {0x62, 0x6f, 0x67, 0x6f, 0x74, 0xc3, 0xa1};
    check(UTF8_TO_STRING(bogot), L"bogot"); // removes diacritic: different from snowball Portuguese
    check(L"boi", L"boi");
    const uint8_t boi[] = {0x62, 0xc3, 0xb3, 0x69, 0x61};
    check(UTF8_TO_STRING(boi), L"boi"); // removes diacritic: different from snowball Portuguese
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
    const uint8_t quilometricas[] = {0x71, 0x75, 0x69, 0x6c, 0x6f, 0x6d, 0xc3, 0xa9, 0x74, 0x72, 0x69, 0x63, 0x61, 0x73};
    check(UTF8_TO_STRING(quilometricas), L"quilometr"); // removes diacritic: different from snowball Portuguese
    const uint8_t quilometricos[] = {0x71, 0x75, 0x69, 0x6c, 0x6f, 0x6d, 0xc3, 0xa9, 0x74, 0x72, 0x69, 0x63, 0x6f, 0x73};
    check(UTF8_TO_STRING(quilometricos), L"quilometr"); // removes diacritic: different from snowball Portuguese
    const uint8_t quilometro[] = {0x71, 0x75, 0x69, 0x6c, 0xc3, 0xb4, 0x6d, 0x65, 0x74, 0x72, 0x6f};
    check(UTF8_TO_STRING(quilometro), L"quilometr"); // removes diacritic: different from snowball Portuguese
    const uint8_t quilometros[] = {0x71, 0x75, 0x69, 0x6c, 0xc3, 0xb4, 0x6d, 0x65, 0x74, 0x72, 0x6f, 0x73};
    check(UTF8_TO_STRING(quilometros), L"quilometr"); // removes diacritic: different from snowball Portuguese
    check(L"quilos", L"quil");
    check(L"quimica", L"quimic");
    check(L"quilos", L"quil");
    check(L"quimica", L"quimic");
    check(L"quimicas", L"quimic");
    check(L"quimico", L"quimic");
    check(L"quimicos", L"quimic");
    check(L"quimioterapia", L"quimioterap");
    const uint8_t quimioterap[] = {0x71, 0x75, 0x69, 0x6d, 0x69, 0x6f, 0x74, 0x65, 0x72, 0xc3, 0xa1, 0x70, 0x69, 0x63, 0x6f, 0x73};
    check(UTF8_TO_STRING(quimioterap), L"quimioterap"); // removes diacritic: different from snowball Portuguese
    check(L"quimono", L"quimon");
    check(L"quincas", L"quinc");
    const uint8_t quinha[] = {0x71, 0x75, 0x69, 0x6e, 0x68, 0xc3, 0xa3, 0x6f};
    check(UTF8_TO_STRING(quinha), L"quinha"); // removes diacritic: different from snowball Portuguese
    check(L"quinhentos", L"quinhent");
    check(L"quinn", L"quinn");
    check(L"quino", L"quin");
    check(L"quinta", L"quint");
    check(L"quintal", L"quintal");
    check(L"quintana", L"quintan");
    check(L"quintanilha", L"quintanilh");
    const uint8_t quinta[] = {0x71, 0x75, 0x69, 0x6e, 0x74, 0xc3, 0xa3, 0x6f};
    check(UTF8_TO_STRING(quinta), L"quinta"); // removes diacritic: different from snowball Portuguese
    const uint8_t quintessente[] = {0x71, 0x75, 0x69, 0x6e, 0x74, 0x65, 0x73, 0x73, 0xc3, 0xaa, 0x6e, 0x63, 0x69, 0x61};
    check(UTF8_TO_STRING(quintessente), L"quintessente"); // removes diacritic: different from snowball Portuguese
    check(L"quintino", L"quintin");
    check(L"quinto", L"quint");
    check(L"quintos", L"quint");
    check(L"quintuplicou", L"quintuplic");
    check(L"quinze", L"quinz");
    check(L"quinzena", L"quinzen");
    check(L"quiosque", L"quiosqu");
}

TEST_F(BrazilianStemmerTest, testNormalization) {
    check(L"Brasil", L"brasil"); // lowercase by default
    const uint8_t brasil[] = {0x42, 0x72, 0x61, 0x73, 0xc3, 0xad, 0x6c, 0x69, 0x61};
    check(UTF8_TO_STRING(brasil), L"brasil"); // remove diacritics
    const uint8_t quimio5terapicos[] = {0x71, 0x75, 0x69, 0x6d, 0x69, 0x6f, 0x35, 0x74, 0x65, 0x72, 0xc3, 0xa1, 0x70, 0x69, 0x63, 0x6f, 0x73};
    check(UTF8_TO_STRING(quimio5terapicos), L"quimio5terapicos"); // contains non-letter, diacritic will still be removed
    const uint8_t aa[] = {0xc3, 0xa1, 0xc3, 0xa1};
    check(UTF8_TO_STRING(aa), UTF8_TO_STRING(aa)); // token is too short: diacritics are not removed
    const uint8_t aaa[] = {0xc3, 0xa1, 0xc3, 0xa1, 0xc3, 0xa1};
    check(UTF8_TO_STRING(aaa), L"aaa"); // normally, diacritics are removed
}

TEST_F(BrazilianStemmerTest, testReusableTokenStream) {
    AnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"boa", L"boa");
    checkReuse(a, L"boainain", L"boainain");
    checkReuse(a, L"boas", L"boas");
    const uint8_t boas[] = {0x62, 0xc3, 0xb4, 0x61, 0x73};
    checkReuse(a, UTF8_TO_STRING(boas), L"boas"); // removes diacritic: different from snowball Portuguese
}

TEST_F(BrazilianStemmerTest, testStemExclusionTable) {
    BrazilianAnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    HashSet<String> exclusions = HashSet<String>::newInstance();
    const uint8_t quintessencia[] = {0x71, 0x75, 0x69, 0x6e, 0x74, 0x65, 0x73, 0x73, 0xc3, 0xaa, 0x6e, 0x63, 0x69, 0x61};
    exclusions.add(UTF8_TO_STRING(quintessencia));
    a->setStemExclusionTable(exclusions);
    checkReuse(a, UTF8_TO_STRING(quintessencia), UTF8_TO_STRING(quintessencia)); // excluded words will be completely unchanged
}

/// Test that changes to the exclusion table are applied immediately when using reusable token streams.
TEST_F(BrazilianStemmerTest, testExclusionTableReuse) {
    BrazilianAnalyzerPtr a = newLucene<BrazilianAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t quintessencia[] = {0x71, 0x75, 0x69, 0x6e, 0x74, 0x65, 0x73, 0x73, 0xc3, 0xaa, 0x6e, 0x63, 0x69, 0x61};
    checkReuse(a, UTF8_TO_STRING(quintessencia), L"quintessente");
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(UTF8_TO_STRING(quintessencia));
    a->setStemExclusionTable(exclusions);
    checkReuse(a, UTF8_TO_STRING(quintessencia), UTF8_TO_STRING(quintessencia));
}
