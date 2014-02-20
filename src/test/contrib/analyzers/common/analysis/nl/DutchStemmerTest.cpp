/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "DutchAnalyzer.h"
#include "WhitespaceTokenizer.h"

using namespace Lucene;

class DutchStemmerTest : public BaseTokenStreamFixture {
public:
    virtual ~DutchStemmerTest() {
    }

public:
    void check(const String& input, const String& expected) {
        checkOneTerm(newLucene<DutchAnalyzer>(LuceneVersion::LUCENE_CURRENT), input, expected);
    }

    void checkReuse(const AnalyzerPtr& a, const String& input, const String& expected) {
        checkOneTermReuse(a, input, expected);
    }
};

/// Test the Dutch Stem Filter, which only modifies the term text.
/// The code states that it uses the snowball algorithm, but tests reveal some differences.

TEST_F(DutchStemmerTest, testWithSnowballExamples) {
    check(L"lichaamsziek", L"lichaamsziek");
    check(L"lichamelijk", L"licham");
    check(L"lichamelijke", L"licham");
    check(L"lichamelijkheden", L"licham");
    check(L"lichamen", L"licham");
    check(L"lichere", L"licher");
    check(L"licht", L"licht");
    check(L"lichtbeeld", L"lichtbeeld");
    check(L"lichtbruin", L"lichtbruin");
    check(L"lichtdoorlatende", L"lichtdoorlat");
    check(L"lichte", L"licht");
    check(L"lichten", L"licht");
    check(L"lichtende", L"lichtend");
    check(L"lichtenvoorde", L"lichtenvoord");
    check(L"lichter", L"lichter");
    check(L"lichtere", L"lichter");
    check(L"lichters", L"lichter");
    check(L"lichtgevoeligheid", L"lichtgevoel");
    check(L"lichtgewicht", L"lichtgewicht");
    check(L"lichtgrijs", L"lichtgrijs");
    check(L"lichthoeveelheid", L"lichthoevel");
    check(L"lichtintensiteit", L"lichtintensiteit");
    check(L"lichtje", L"lichtj");
    check(L"lichtjes", L"lichtjes");
    check(L"lichtkranten", L"lichtkrant");
    check(L"lichtkring", L"lichtkring");
    check(L"lichtkringen", L"lichtkring");
    check(L"lichtregelsystemen", L"lichtregelsystem");
    check(L"lichtste", L"lichtst");
    check(L"lichtstromende", L"lichtstrom");
    check(L"lichtte", L"licht");
    check(L"lichtten", L"licht");
    check(L"lichttoetreding", L"lichttoetred");
    check(L"lichtverontreinigde", L"lichtverontreinigd");
    check(L"lichtzinnige", L"lichtzinn");
    check(L"lid", L"lid");
    check(L"lidia", L"lidia");
    check(L"lidmaatschap", L"lidmaatschap");
    check(L"lidstaten", L"lidstat");
    check(L"lidvereniging", L"lidveren");
    check(L"opgingen", L"opging");
    check(L"opglanzing", L"opglanz");
    check(L"opglanzingen", L"opglanz");
    check(L"opglimlachten", L"opglimlacht");
    check(L"opglimpen", L"opglimp");
    check(L"opglimpende", L"opglimp");
    check(L"opglimping", L"opglimp");
    check(L"opglimpingen", L"opglimp");
    check(L"opgraven", L"opgrav");
    check(L"opgrijnzen", L"opgrijnz");
    check(L"opgrijzende", L"opgrijz");
    check(L"opgroeien", L"opgroei");
    check(L"opgroeiende", L"opgroei");
    check(L"opgroeiplaats", L"opgroeiplat");
    check(L"ophaal", L"ophal");
    check(L"ophaaldienst", L"ophaaldienst");
    check(L"ophaalkosten", L"ophaalkost");
    check(L"ophaalsystemen", L"ophaalsystem");
    check(L"ophaalt", L"ophaalt");
    check(L"ophaaltruck", L"ophaaltruck");
    check(L"ophalen", L"ophal");
    check(L"ophalend", L"ophal");
    check(L"ophalers", L"ophaler");
    check(L"ophef", L"ophef");
    check(L"opheffen", L"ophef"); // versus snowball 'opheff'
    check(L"opheffende", L"ophef"); // versus snowball 'opheff'
    check(L"opheffing", L"ophef"); // versus snowball 'opheff'
    check(L"opheldering", L"ophelder");
    check(L"ophemelde", L"ophemeld");
    check(L"ophemelen", L"ophemel");
    check(L"opheusden", L"opheusd");
    check(L"ophief", L"ophief");
    check(L"ophield", L"ophield");
    check(L"ophieven", L"ophiev");
    check(L"ophoepelt", L"ophoepelt");
    check(L"ophoog", L"ophog");
    check(L"ophoogzand", L"ophoogzand");
    check(L"ophopen", L"ophop");
    check(L"ophoping", L"ophop");
    check(L"ophouden", L"ophoud");
}

TEST_F(DutchStemmerTest, testReusableTokenStream) {
    AnalyzerPtr a = newLucene<DutchAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"lichaamsziek", L"lichaamsziek");
    checkReuse(a, L"lichamelijk", L"licham");
    checkReuse(a, L"lichamelijke", L"licham");
    checkReuse(a, L"lichamelijkheden", L"licham");
}

/// Test that changes to the exclusion table are applied immediately when using reusable token streams.
TEST_F(DutchStemmerTest, testExclusionTableReuse) {
    DutchAnalyzerPtr a = newLucene<DutchAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkReuse(a, L"lichamelijk", L"licham");
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(L"lichamelijk");
    a->setStemExclusionTable(exclusions);
    checkReuse(a, L"lichamelijk", L"lichamelijk");
}
