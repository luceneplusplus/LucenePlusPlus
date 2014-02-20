/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "StandardAnalyzer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture StandardAnalyzerTest;

TEST_F(StandardAnalyzerTest, testMaxTermLength) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    sa->setMaxTokenLength(5);
    checkAnalyzesTo(sa, L"ab cd toolong xy z", newCollection<String>(L"ab", L"cd", L"xy", L"z"));
}

TEST_F(StandardAnalyzerTest, testMaxTermLength2) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    checkAnalyzesTo(sa, L"ab cd toolong xy z", newCollection<String>(L"ab", L"cd", L"toolong", L"xy", L"z"));
    sa->setMaxTokenLength(5);
    checkAnalyzesTo(sa, L"ab cd toolong xy z", newCollection<String>(L"ab", L"cd", L"xy", L"z"), newCollection<int32_t>(1, 1, 2, 1));
}

TEST_F(StandardAnalyzerTest, testMaxTermLength3) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    String longTerm(255, L'a');
    checkAnalyzesTo(sa, L"ab cd " + longTerm + L" xy z", newCollection<String>(L"ab", L"cd", longTerm, L"xy", L"z"));
    checkAnalyzesTo(sa, L"ab cd " + longTerm + L"a xy z", newCollection<String>(L"ab", L"cd", L"xy", L"z"));
}

TEST_F(StandardAnalyzerTest, testAlphanumeric) {
    // alphanumeric tokens
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"B2B", newCollection<String>(L"b2b"));
    checkAnalyzesTo(sa, L"2B", newCollection<String>(L"2b"));
}

TEST_F(StandardAnalyzerTest, testUnderscores) {
    // underscores are delimiters, but not in email addresses (below)
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"word_having_underscore", newCollection<String>(L"word", L"having", L"underscore"));
    checkAnalyzesTo(sa, L"word_with_underscore_and_stopwords", newCollection<String>(L"word", L"underscore", L"stopwords"));
}

TEST_F(StandardAnalyzerTest, testDelimiters) {
    // other delimiters: "-", "/", ","
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"some-dashed-phrase", newCollection<String>(L"some", L"dashed", L"phrase"));
    checkAnalyzesTo(sa, L"dogs,chase,cats", newCollection<String>(L"dogs", L"chase", L"cats"));
    checkAnalyzesTo(sa, L"ac/dc", newCollection<String>(L"ac", L"dc"));
}

TEST_F(StandardAnalyzerTest, testApostrophes) {
    // internal apostrophes: O'Reilly, you're, O'Reilly's possessives are actually removed by StardardFilter, not the tokenizer
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    checkAnalyzesTo(sa, L"O'Reilly", newCollection<String>(L"o'reilly"));
    checkAnalyzesTo(sa, L"you're", newCollection<String>(L"you're"));
    checkAnalyzesTo(sa, L"she's", newCollection<String>(L"she"));
    checkAnalyzesTo(sa, L"Jim's", newCollection<String>(L"jim"));
    checkAnalyzesTo(sa, L"don't", newCollection<String>(L"don't"));
    checkAnalyzesTo(sa, L"O'Reilly's", newCollection<String>(L"o'reilly"));
}

TEST_F(StandardAnalyzerTest, testTSADash) {
    // t and s had been stopwords in Lucene <= 2.0, which made it impossible to correctly search for these terms
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"s-class", newCollection<String>(L"s", L"class"));
    checkAnalyzesTo(sa, L"t-com", newCollection<String>(L"t", L"com"));

    // 'a' is still a stopword
    checkAnalyzesTo(sa, L"a-class", newCollection<String>(L"class"));
}

TEST_F(StandardAnalyzerTest, testCompanyNames) {
    // internal apostrophes: O'Reilly, you're, O'Reilly's possessives are actually removed by StardardFilter, not the tokenizer
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"AT&T", newCollection<String>(L"at&t"));
    checkAnalyzesTo(sa, L"Excite@Home", newCollection<String>(L"excite@home"));
}

TEST_F(StandardAnalyzerTest, testDomainNames) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    // domain names
    checkAnalyzesTo(sa, L"www.nutch.org", newCollection<String>(L"www.nutch.org"));

    // the following should be recognized as HOST
    EXPECT_NO_THROW(checkAnalyzesTo(sa, L"www.nutch.org.", newCollection<String>(L"www.nutch.org"), newCollection<String>(L"<HOST>")));

    // 2.3 should show the bug
    sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_23);
    EXPECT_NO_THROW(checkAnalyzesTo(sa, L"www.nutch.org.", newCollection<String>(L"wwwnutchorg"), newCollection<String>(L"<ACRONYM>")));

    // 2.4 should not show the bug
    sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_24);
    EXPECT_NO_THROW(checkAnalyzesTo(sa, L"www.nutch.org.", newCollection<String>(L"www.nutch.org"), newCollection<String>(L"<HOST>")));
}

TEST_F(StandardAnalyzerTest, testEMailAddresses) {
    // email addresses, possibly with underscores, periods, etc
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"test@example.com", newCollection<String>(L"test@example.com"));
    checkAnalyzesTo(sa, L"first.lastname@example.com", newCollection<String>(L"first.lastname@example.com"));
    checkAnalyzesTo(sa, L"first_lastname@example.com", newCollection<String>(L"first_lastname@example.com"));
}

TEST_F(StandardAnalyzerTest, testNumeric) {
    // floating point, serial, model numbers, ip addresses, etc.
    // every other segment must have at least one digit
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"21.35", newCollection<String>(L"21.35"));
    checkAnalyzesTo(sa, L"216.239.63.104", newCollection<String>(L"216.239.63.104"));
    checkAnalyzesTo(sa, L"1-2-3", newCollection<String>(L"1-2-3"));
    checkAnalyzesTo(sa, L"a1-b2-c3", newCollection<String>(L"a1-b2-c3"));
    checkAnalyzesTo(sa, L"a1-b-c3", newCollection<String>(L"a1-b-c3"));
    checkAnalyzesTo(sa, L"R2D2 C3PO", newCollection<String>(L"r2d2", L"c3po"));
}

TEST_F(StandardAnalyzerTest, testTextWithNumbers) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"David has 5000 bones", newCollection<String>(L"david", L"has", L"5000", L"bones"));
}

TEST_F(StandardAnalyzerTest, testVariousText) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"C embedded developers wanted", newCollection<String>(L"c", L"embedded", L"developers", L"wanted"));
    checkAnalyzesTo(sa, L"foo bar FOO BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(sa, L"foo      bar .  FOO <> BAR", newCollection<String>(L"foo", L"bar", L"foo", L"bar"));
    checkAnalyzesTo(sa, L"\"QUOTED\" word", newCollection<String>(L"quoted", L"word"));
}

TEST_F(StandardAnalyzerTest, testAcronyms) {
    // acronyms have their dots stripped
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"U.S.A.", newCollection<String>(L"usa"));
}

TEST_F(StandardAnalyzerTest, testCPlusPlusHash) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"C++", newCollection<String>(L"c"));
    checkAnalyzesTo(sa, L"C#", newCollection<String>(L"c"));
}

TEST_F(StandardAnalyzerTest, testComplianceFileName) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"2004.jpg", newCollection<String>(L"2004.jpg"), newCollection<String>(L"<HOST>"));
}

TEST_F(StandardAnalyzerTest, testComplianceNumericIncorrect) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"62.46", newCollection<String>(L"62.46"), newCollection<String>(L"<HOST>"));
}

TEST_F(StandardAnalyzerTest, testComplianceNumericLong) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"978-0-94045043-1", newCollection<String>(L"978-0-94045043-1"), newCollection<String>(L"<NUM>"));
}

TEST_F(StandardAnalyzerTest, testComplianceNumericFile) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"78academyawards/rules/rule02.html", newCollection<String>(L"78academyawards/rules/rule02.html"), newCollection<String>(L"<NUM>"));
}

TEST_F(StandardAnalyzerTest, testComplianceNumericWithUnderscores) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"2006-03-11t082958z_01_ban130523_rtridst_0_ozabs", newCollection<String>(L"2006-03-11t082958z_01_ban130523_rtridst_0_ozabs"), newCollection<String>(L"<NUM>"));
}

TEST_F(StandardAnalyzerTest, testComplianceNumericWithDash) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"mid-20th", newCollection<String>(L"mid-20th"), newCollection<String>(L"<NUM>"));
}

TEST_F(StandardAnalyzerTest, testComplianceManyTokens) {
    StandardAnalyzerPtr sa = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesTo(sa, L"/money.cnn.com/magazines/fortune/fortune_archive/2007/03/19/8402357/index.htm safari-0-sheikh-zayed-grand-mosque.jpg",
                    newCollection<String>(L"money.cnn.com", L"magazines", L"fortune", L"fortune", L"archive/2007/03/19/8402357",
                                          L"index.htm", L"safari-0-sheikh", L"zayed", L"grand", L"mosque.jpg"),
                    newCollection<String>(L"<HOST>", L"<ALPHANUM>", L"<ALPHANUM>", L"<ALPHANUM>", L"<NUM>", L"<HOST>", L"<NUM>",
                                          L"<ALPHANUM>", L"<ALPHANUM>", L"<HOST>"));
}
