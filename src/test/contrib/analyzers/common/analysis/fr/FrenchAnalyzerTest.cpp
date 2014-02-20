/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "BaseTokenStreamFixture.h"
#include "FrenchAnalyzer.h"

using namespace Lucene;

typedef BaseTokenStreamFixture FrenchAnalyzerTest;

TEST_F(FrenchAnalyzerTest, testAnalyzer) {
    AnalyzerPtr fa = newLucene<FrenchAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    checkAnalyzesTo(fa, L"", Collection<String>::newInstance());

    checkAnalyzesTo(fa, L"chien chat cheval", newCollection<String>(L"chien", L"chat", L"cheval"));

    checkAnalyzesTo(fa, L"chien CHAT CHEVAL", newCollection<String>(L"chien", L"chat", L"cheval"));

    checkAnalyzesTo(fa, L"  chien  ,? + = -  CHAT /: > CHEVAL", newCollection<String>(L"chien", L"chat", L"cheval"));

    checkAnalyzesTo(fa, L"chien++", newCollection<String>(L"chien"));

    checkAnalyzesTo(fa, L"mot \"entreguillemet\"", newCollection<String>(L"mot", L"entreguillemet"));

    // let's do some French specific tests now

    /// I would expect this to stay one term as in French the minus sign is often used for composing words
    checkAnalyzesTo(fa, L"Jean-Fran\u00e7ois", newCollection<String>(L"jean", L"fran\u00e7ois"));

    // stopwords
    checkAnalyzesTo(fa, L"le la chien les aux chat du des \u00e0 cheval", newCollection<String>(L"chien", L"chat", L"cheval"));

    // some nouns and adjectives
    checkAnalyzesTo(fa, L"lances chismes habitable chiste \u00e9l\u00e9ments captifs",
                    newCollection<String>( L"lanc", L"chism", L"habit", L"chist", L"\u00e9l\u00e9ment", L"captif"));

    // some verbs
    checkAnalyzesTo(fa, L"finissions souffrirent rugissante", newCollection<String>(L"fin", L"souffr", L"rug"));

    // aujourd'hui stays one term which is OK
    checkAnalyzesTo(fa, L"C3PO aujourd\'hui oeuf \u00ef\u00e2\u00f6\u00fb\u00e0\u00e4 anticonstitutionnellement Java++ ",
                    newCollection<String>(L"c3po", L"aujourd\'hui", L"oeuf", L"\u00ef\u00e2\u00f6\u00fb\u00e0\u00e4", L"anticonstitutionnel", L"jav"));

    // here 1940-1945 stays as one term, 1940:1945 not ?
    checkAnalyzesTo(fa, L"33Bis 1940-1945 1940:1945 (---i+++)*",
                    newCollection<String>(L"33bis", L"1940-1945", L"1940", L"1945", L"i"));
}

TEST_F(FrenchAnalyzerTest, testReusableTokenStream) {
    AnalyzerPtr fa = newLucene<FrenchAnalyzer>(LuceneVersion::LUCENE_CURRENT);

    // stopwords
    checkAnalyzesToReuse(fa, L"le la chien les aux chat du des \u00e0 cheval",
                         newCollection<String>(L"chien", L"chat", L"cheval"));

    // some nouns and adjectives
    checkAnalyzesToReuse(fa, L"lances chismes habitable chiste \u00e9l\u00e9ments captifs",
                         newCollection<String>(L"lanc", L"chism", L"habit", L"chist", L"\u00e9l\u00e9ment", L"captif"));
}

/// Test that changes to the exclusion table are applied immediately when using reusable token streams.
TEST_F(FrenchAnalyzerTest, testExclusionTableReuse) {
    FrenchAnalyzerPtr fa = newLucene<FrenchAnalyzer>(LuceneVersion::LUCENE_CURRENT);
    checkAnalyzesToReuse(fa, L"habitable", newCollection<String>(L"habit"));
    HashSet<String> exclusions = HashSet<String>::newInstance();
    exclusions.add(L"habitable");
    fa->setStemExclusionTable(exclusions);
    checkAnalyzesToReuse(fa, L"habitable", newCollection<String>(L"habitable"));
}
