/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "ArabicAnalyzer.h"

using namespace Lucene;

BOOST_FIXTURE_TEST_SUITE(ArabicAnalyzerTest, BaseTokenStreamFixture)

/// Some simple tests showing some features of the analyzer, how some regular forms will conflate

BOOST_AUTO_TEST_CASE(testBasicFeatures1)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures2)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1, 0xd8, 0xa9};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures3)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x85, 0xd8, 0xb4, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8};
    const uint8_t second[] = {0xd9, 0x85, 0xd8, 0xb4, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures4)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x85, 0xd8, 0xb4, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8, 0xd8, 0xa7, 0xd8, 0xaa};
    const uint8_t second[] = {0xd9, 0x85, 0xd8, 0xb4, 0xd8, 0xb1, 0xd9, 0x88, 0xd8, 0xa8};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures5)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd8, 0xa3, 0xd9, 0x85, 0xd8, 0xb1, 0xd9, 0x8a, 0xd9, 0x83, 0xd9, 0x8a, 0xd9, 0x8a, 0xd9, 0x86};
    const uint8_t second[] = {0xd8, 0xa7, 0xd9, 0x85, 0xd8, 0xb1, 0xd9, 0x8a, 0xd9, 0x83};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures6)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd8, 0xa7, 0xd9, 0x85, 0xd8, 0xb1, 0xd9, 0x8a, 0xd9, 0x83, 0xd9, 0x8a};
    const uint8_t second[] = {0xd8, 0xa7, 0xd9, 0x85, 0xd8, 0xb1, 0xd9, 0x8a, 0xd9, 0x83};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures7)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures8)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xaa, 0xd8, 0xa7, 0xd8, 0xa8};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures9)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x85, 0xd8, 0xa7, 0x20, 0xd9, 0x85, 0xd9, 0x84, 0xd9, 0x83, 0xd8, 0xaa, 0x20, 0xd8, 0xa3, 0xd9, 0x8a, 0xd9, 0x85, 0xd8, 0xa7, 0xd9, 0x86, 0xd9, 0x83, 0xd9, 0x85};
    const uint8_t second[] = {0xd9, 0x85, 0xd9, 0x84, 0xd9, 0x83, 0xd8, 0xaa};
    const uint8_t third[] = {0xd8, 0xa7, 0xd9, 0x8a, 0xd9, 0x85, 0xd8, 0xa7, 0xd9, 0x86, 0xd9, 0x83, 0xd9, 0x85};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second), UTF8_TO_STRING(third)));
}

BOOST_AUTO_TEST_CASE(testBasicFeatures10)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd8, 0xa7, 0xd9, 0x84, 0xd8, 0xb0, 0xd9, 0x8a, 0xd9, 0x86, 0x20, 0xd9, 0x85, 0xd9, 0x84, 0xd9, 0x83, 0xd8, 0xaa, 0x20, 0xd8, 0xa3, 0xd9, 0x8a, 0xd9, 0x85, 0xd8, 0xa7, 0xd9, 0x86, 0xd9, 0x83, 0xd9, 0x85};
    const uint8_t second[] = {0xd9, 0x85, 0xd9, 0x84, 0xd9, 0x83, 0xd8, 0xaa};
    const uint8_t third[] = {0xd8, 0xa7, 0xd9, 0x8a, 0xd9, 0x85, 0xd8, 0xa7, 0xd9, 0x86, 0xd9, 0x83, 0xd9, 0x85};
    checkAnalyzesTo(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second), UTF8_TO_STRING(third)));
}

/// Simple tests to show things are getting reset correctly, etc.
BOOST_AUTO_TEST_CASE(testReusableTokenStream1)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    checkAnalyzesToReuse(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

BOOST_AUTO_TEST_CASE(testReusableTokenStream2)
{
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    const uint8_t first[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1, 0xd8, 0xa9};
    const uint8_t second[] = {0xd9, 0x83, 0xd8, 0xa8, 0xd9, 0x8a, 0xd8, 0xb1};
    checkAnalyzesToReuse(a, UTF8_TO_STRING(first), newCollection<String>(UTF8_TO_STRING(second)));
}

/// Non-arabic text gets treated in a similar way as SimpleAnalyzer.
BOOST_AUTO_TEST_CASE(testEnglishInput)
{
    checkAnalyzesTo(newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT), L"English text.", newCollection<String>(L"english", L"text"));
}

/// Test that custom stopwords work, and are not case-sensitive.
BOOST_AUTO_TEST_CASE(testCustomStopwords)
{
    Collection<String> stopWords = newCollection<String>(L"the", L"and", L"a");
    ArabicAnalyzerPtr a = newLucene<ArabicAnalyzer>(LuceneVersion::LUCENE_CURRENT, HashSet<String>::newInstance(stopWords.begin(), stopWords.end()));
    checkAnalyzesTo(a, L"The quick brown fox.", newCollection<String>(L"quick", L"brown", L"fox"));
}

BOOST_AUTO_TEST_SUITE_END()
