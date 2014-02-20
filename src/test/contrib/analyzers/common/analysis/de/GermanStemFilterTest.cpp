/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "GermanAnalyzer.h"
#include "WhitespaceTokenizer.h"

using namespace Lucene;

class GermanStemFilterTest : public BaseTokenStreamFixture {
public:
    virtual ~GermanStemFilterTest() {
    }

public:
    void check(const String& input, const String& expected) {
        checkOneTerm(newLucene<GermanAnalyzer>(LuceneVersion::LUCENE_CURRENT), input, expected);
    }

    void checkReuse(const AnalyzerPtr& a, const String& input, const String& expected) {
        checkOneTermReuse(a, input, expected);
    }
};

/// Test the German stemmer. The stemming algorithm is known to work less than perfect, as it doesn't
/// use any word lists with exceptions. We also check some of the cases where the algorithm is wrong.

TEST_F(GermanStemFilterTest, testStemming) {
    const uint8_t haufig[] = {0x68, 0xc3, 0xa4, 0x75, 0x66, 0x69, 0x67};
    check(UTF8_TO_STRING(haufig), L"haufig"); // German special characters are replaced

    const uint8_t abschliess1[] = {0x61, 0x62, 0x73, 0x63, 0x68, 0x6c, 0x69, 0x65, 0xc3, 0x9f, 0x65, 0x6e};
    check(UTF8_TO_STRING(abschliess1), L"abschliess"); // here the stemmer works okay, it maps related words to the same stem

    const uint8_t abschliess2[] = {0x61, 0x62, 0x73, 0x63, 0x68, 0x6c, 0x69, 0x65, 0xc3, 0x9f, 0x65, 0x6e, 0x64, 0x65, 0x72};
    check(UTF8_TO_STRING(abschliess2), L"abschliess"); // here the stemmer works okay, it maps related words to the same stem

    const uint8_t abschliess3[] = {0x61, 0x62, 0x73, 0x63, 0x68, 0x6c, 0x69, 0x65, 0xc3, 0x9f, 0x65, 0x6e, 0x64, 0x65, 0x73};
    check(UTF8_TO_STRING(abschliess3), L"abschliess"); // here the stemmer works okay, it maps related words to the same stem

    const uint8_t abschliess4[] = {0x61, 0x62, 0x73, 0x63, 0x68, 0x6c, 0x69, 0x65, 0xc3, 0x9f, 0x65, 0x6e, 0x64, 0x65, 0x6e};
    check(UTF8_TO_STRING(abschliess4), L"abschliess"); // here the stemmer works okay, it maps related words to the same stem

    check(L"Tisch", L"tisch");
    check(L"Tische", L"tisch");
    check(L"Tischen", L"tisch");

    check(L"Haus", L"hau");
    check(L"Hauses", L"hau");

    const uint8_t hau1[] = {0x48, 0xc3, 0xa4, 0x75, 0x73, 0x65, 0x72};
    check(UTF8_TO_STRING(hau1), L"hau");

    const uint8_t hau2[] = {0x48, 0xc3, 0xa4, 0x75, 0x73, 0x65, 0x72, 0x6e};
    check(UTF8_TO_STRING(hau2), L"hau");

    // Here's a case where overstemming occurs, ie. a word is mapped to the same stem as unrelated words
    check(L"hauen", L"hau");

    // Here's a case where understemming occurs, i.e. two related words are not mapped to the same stem.
    // This is the case with basically all irregular forms
    check(L"Drama", L"drama");
    check(L"Dramen", L"dram");

    const uint8_t ausmass[] = {0x41, 0x75, 0x73, 0x6d, 0x61, 0xc3, 0x9f};
    check(UTF8_TO_STRING(ausmass), L"ausmass");

    // Fake words to test if suffixes are cut off
    check(L"xxxxxe", L"xxxxx");
    check(L"xxxxxs", L"xxxxx");
    check(L"xxxxxn", L"xxxxx");
    check(L"xxxxxt", L"xxxxx");
    check(L"xxxxxem", L"xxxxx");
    check(L"xxxxxet", L"xxxxx");
    check(L"xxxxxnd", L"xxxxx");

    // The suffixes are also removed when combined
    check(L"xxxxxetende", L"xxxxx");

    // Words that are shorter than four charcters are not changed
    check(L"xxe", L"xxe");

    // -em and -er are not removed from words shorter than five characters
    check(L"xxem", L"xxem");
    check(L"xxer", L"xxer");

    // -nd is not removed from words shorter than six characters
    check(L"xxxnd", L"xxxnd");
}

TEST_F(GermanStemFilterTest, testReusableTokenStream) {
    AnalyzerPtr a = newLucene<GermanAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"Tisch", L"tisch");
    checkReuse(a, L"Tische", L"tisch");
    checkReuse(a, L"Tischen", L"tisch");
}

/// Test that changes to the exclusion table are applied immediately when using reusable token streams.
TEST_F(GermanStemFilterTest, testExclusionTableReuse) {
    GermanAnalyzerPtr a = newLucene<GermanAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"tischen", L"tisch");
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(L"tischen");
    a->setStemExclusionTable(exclusions);
    checkReuse(a, L"tischen", L"tischen");
}
