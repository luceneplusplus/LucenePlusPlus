/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianAnalyzer.h"
#include "RussianLetterTokenizer.h"
#include "LowerCaseFilter.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "StopFilter.h"
#include "RussianStemFilter.h"
#include "SnowballFilter.h"
#include "KeywordMarkerFilter.h"
#include "StringUtils.h"

namespace Lucene
{
    const wchar_t* RussianAnalyzer::_RUSSIAN_STOP_WORDS_30[] =
    {
        L"\x0430", L"\x0431"L"\x0435"L"\x0437", L"\x0431"L"\x043e"L"\x043b"L"\x0435"L"\x0435", L"\x0431"L"\x044b",
        L"\x0431"L"\x044b"L"\x043b", L"\x0431"L"\x044b"L"\x043b"L"\x0430", L"\x0431"L"\x044b"L"\x043b"L"\x0438",
        L"\x0431"L"\x044b"L"\x043b"L"\x043e", L"\x0431"L"\x044b"L"\x0442"L"\x044c", L"\x0432",
        L"\x0432"L"\x0430"L"\x043c", L"\x0432"L"\x0430"L"\x0441", L"\x0432"L"\x0435"L"\x0441"L"\x044c",
        L"\x0432"L"\x043e", L"\x0432"L"\x043e"L"\x0442", L"\x0432"L"\x0441"L"\x0435", L"\x0432"L"\x0441"L"\x0435"L"\x0433"L"\x043e",
        L"\x0432"L"\x0441"L"\x0435"L"\x0445", L"\x0432"L"\x044b", L"\x0433"L"\x0434"L"\x0435", L"\x0434"L"\x0430",
        L"\x0434"L"\x0430"L"\x0436"L"\x0435", L"\x0434"L"\x043b"L"\x044f", L"\x0434"L"\x043e", L"\x0435"L"\x0433"L"\x043e",
        L"\x0435"L"\x0435", L"\x0435"L"\x0439", L"\x0435"L"\x044e", L"\x0435"L"\x0441"L"\x043b"L"\x0438",
        L"\x0435"L"\x0441"L"\x0442"L"\x044c", L"\x0435"L"\x0449"L"\x0435", L"\x0436"L"\x0435", L"\x0437"L"\x0430",
        L"\x0437"L"\x0434"L"\x0435"L"\x0441"L"\x044c", L"\x0438", L"\x0438"L"\x0437", L"\x0438"L"\x043b"L"\x0438",
        L"\x0438"L"\x043c", L"\x0438"L"\x0445", L"\x043a", L"\x043a"L"\x0430"L"\x043a", L"\x043a"L"\x043e",
        L"\x043a"L"\x043e"L"\x0433"L"\x0434"L"\x0430", L"\x043a"L"\x0442"L"\x043e", L"\x043b"L"\x0438",
        L"\x043b"L"\x0438"L"\x0431"L"\x043e", L"\x043c"L"\x043d"L"\x0435", L"\x043c"L"\x043e"L"\x0436"L"\x0435"L"\x0442",
        L"\x043c"L"\x044b", L"\x043d"L"\x0430", L"\x043d"L"\x0430"L"\x0434"L"\x043e", L"\x043d"L"\x0430"L"\x0448",
        L"\x043d"L"\x0435", L"\x043d"L"\x0435"L"\x0433"L"\x043e", L"\x043d"L"\x0435"L"\x0435", L"\x043d"L"\x0435"L"\x0442",
        L"\x043d"L"\x0438", L"\x043d"L"\x0438"L"\x0445", L"\x043d"L"\x043e", L"\x043d"L"\x0443", L"\x043e",
        L"\x043e"L"\x0431", L"\x043e"L"\x0434"L"\x043d"L"\x0430"L"\x043a"L"\x043e", L"\x043e"L"\x043d",
        L"\x043e"L"\x043d"L"\x0430", L"\x043e"L"\x043d"L"\x0438", L"\x043e"L"\x043d"L"\x043e", L"\x043e"L"\x0442",
        L"\x043e"L"\x0447"L"\x0435"L"\x043d"L"\x044c", L"\x043f"L"\x043e", L"\x043f"L"\x043e"L"\x0434", L"\x043f"L"\x0440"L"\x0438",
        L"\x0441", L"\x0441"L"\x043e", L"\x0442"L"\x0430"L"\x043a", L"\x0442"L"\x0430"L"\x043a"L"\x0436"L"\x0435",
        L"\x0442"L"\x0430"L"\x043a"L"\x043e"L"\x0439", L"\x0442"L"\x0430"L"\x043c", L"\x0442"L"\x0435", L"\x0442"L"\x0435"L"\x043c",
        L"\x0442"L"\x043e", L"\x0442"L"\x043e"L"\x0433"L"\x043e", L"\x0442"L"\x043e"L"\x0436"L"\x0435", L"\x0442"L"\x043e"L"\x0439",
        L"\x0442"L"\x043e"L"\x043b"L"\x044c"L"\x043a"L"\x043e", L"\x0442"L"\x043e"L"\x043c", L"\x0442"L"\x044b", L"\x0443",
        L"\x0443"L"\x0436"L"\x0435", L"\x0445"L"\x043e"L"\x0442"L"\x044f", L"\x0447"L"\x0435"L"\x0433"L"\x043e",
        L"\x0447"L"\x0435"L"\x0439", L"\x0447"L"\x0435"L"\x043c", L"\x0447"L"\x0442"L"\x043e",
        L"\x0447"L"\x0442"L"\x043e"L"\x0431"L"\x044b", L"\x0447"L"\x044c"L"\x0435", L"\x0447"L"\x044c"L"\x044f",
        L"\x044d"L"\x0442"L"\x0430", L"\x044d"L"\x0442"L"\x0438", L"\x044d"L"\x0442"L"\x043e", L"\x044f"L""
    };

    /// From svn.tartarus.org/snowball/trunk/website/algorithms/russian/stop.txt
    /// This file is distributed under the BSD License.
    const wchar_t* RussianAnalyzer::_RUSSIAN_STOP_WORDS_DEFAULT[] =
    {
        L"\x0438", L"\x0432", L"\x0432"L"\x043e", L"\x043d"L"\x0435", L"\x0447"L"\x0442"L"\x043e", L"\x043e"L"\x043d",
        L"\x043d"L"\x0430", L"\x044f", L"\x0441", L"\x0441"L"\x043e", L"\x043a"L"\x0430"L"\x043a", L"\x0430",
        L"\x0442"L"\x043e", L"\x0432"L"\x0441"L"\x0435", L"\x043e"L"\x043d"L"\x0430", L"\x0442"L"\x0430"L"\x043a",
        L"\x0435"L"\x0433"L"\x043e", L"\x043d"L"\x043e", L"\x0434"L"\x0430", L"\x0442"L"\x044b", L"\x043a", L"\x0443",
        L"\x0436"L"\x0435", L"\x0432"L"\x044b", L"\x0437"L"\x0430", L"\x0431"L"\x044b", L"\x043f"L"\x043e",
        L"\x0442"L"\x043e"L"\x043b"L"\x044c", L"\x0435"L"\x0435", L"\x043c"L"\x043d"L"\x0435", L"\x0431"L"\x044b"L"\x043b"L"\x043e",
        L"\x0432"L"\x043e"L"\x0442", L"\x043e"L"\x0442", L"\x043c"L"\x0435"L"\x043d"L"\x044f", L"\x0435"L"\x0449"L"\x0435",
        L"\x043d"L"\x0435"L"\x0442", L"\x043e", L"\x0438"L"\x0437", L"\x0435"L"\x043c"L"\x0443",
        L"\x0442"L"\x0435"L"\x043f"L"\x0435"L"\x0440"L"\x044c", L"\x043a"L"\x043e"L"\x0433"L"\x0434"L"\x0430",
        L"\x0434"L"\x0430"L"\x0436"L"\x0435", L"\x043d"L"\x0443", L"\x0432"L"\x0434"L"\x0440"L"\x0443"L"\x0433", L"\x043b"L"\x0438",
        L"\x0435"L"\x0441"L"\x043b"L"\x0438", L"\x0443"L"\x0436"L"\x0435", L"\x0438"L"\x043b"L"\x0438", L"\x043d"L"\x0438",
        L"\x0431"L"\x044b"L"\x0442"L"\x044c", L"\x0431"L"\x044b"L"\x043b", L"\x043d"L"\x0435"L"\x0433"L"\x043e", L"\x0434"L"\x043e",
        L"\x0432"L"\x0430"L"\x0441", L"\x043d"L"\x0438"L"\x0431"L"\x0443"L"\x0434"L"\x044c", L"\x043e"L"\x043f"L"\x044f"L"\x0442"L"\x044c",
        L"\x0443"L"\x0436", L"\x0432"L"\x0430"L"\x043c", L"\x0441"L"\x043a"L"\x0430"L"\x0437"L"\x0430"L"\x043b",
        L"\x0432"L"\x0435"L"\x0434"L"\x044c", L"\x0442"L"\x0430"L"\x043c", L"\x043f"L"\x043e"L"\x0442"L"\x043e"L"\x043c",
        L"\x0441"L"\x0435"L"\x0431"L"\x044f", L"\x043d"L"\x0438"L"\x0447"L"\x0435"L"\x0433"L"\x043e", L"\x0435"L"\x0439",
        L"\x043c"L"\x043e"L"\x0436"L"\x0435"L"\x0442", L"\x043e"L"\x043d"L"\x0438", L"\x0442"L"\x0443"L"\x0442",
        L"\x0433"L"\x0434"L"\x0435", L"\x0435"L"\x0441"L"\x0442"L"\x044c", L"\x043d"L"\x0430"L"\x0434"L"\x043e",
        L"\x043d"L"\x0435"L"\x0439", L"\x0434"L"\x043b"L"\x044f", L"\x043c"L"\x044b", L"\x0442"L"\x0435"L"\x0431"L"\x044f",
        L"\x0438"L"\x0445", L"\x0447"L"\x0435"L"\x043c", L"\x0431"L"\x044b"L"\x043b"L"\x0430", L"\x0441"L"\x0430"L"\x043c",
        L"\x0447"L"\x0442"L"\x043e"L"\x0431", L"\x0431"L"\x0435"L"\x0437", L"\x0431"L"\x0443"L"\x0434"L"\x0442"L"\x043e",
        L"\x0447"L"\x0435"L"\x043b"L"\x043e"L"\x0432"L"\x0435"L"\x043a", L"\x0447"L"\x0435"L"\x0433"L"\x043e", L"\x0440"L"\x0430"L"\x0437",
        L"\x0442"L"\x043e"L"\x0436"L"\x0435", L"\x0441"L"\x0435"L"\x0431"L"\x0435", L"\x043f"L"\x043e"L"\x0434",
        L"\x0436"L"\x0438"L"\x0437"L"\x043d"L"\x044c", L"\x0431"L"\x0443"L"\x0434"L"\x0435"L"\x0442", L"\x0436",
        L"\x0442"L"\x043e"L"\x0433"L"\x0434"L"\x0430", L"\x043a"L"\x0442"L"\x043e", L"\x044d"L"\x0442"L"\x043e"L"\x0442",
        L"\x0433"L"\x043e"L"\x0432"L"\x043e"L"\x0440"L"\x0438"L"\x043b", L"\x0442"L"\x043e"L"\x0433"L"\x043e",
        L"\x043f"L"\x043e"L"\x0442"L"\x043e"L"\x043c"L"\x0443", L"\x044d"L"\x0442"L"\x043e"L"\x0433"L"\x043e",
        L"\x043a"L"\x0430"L"\x043a"L"\x043e"L"\x0439", L"\x0441"L"\x043e"L"\x0432"L"\x0441"L"\x0435"L"\x043c", L"\x043d"L"\x0438"L"\x043c",
        L"\x0437"L"\x0434"L"\x0435"L"\x0441"L"\x044c", L"\x044d"L"\x0442"L"\x043e"L"\x043c", L"\x043e"L"\x0434"L"\x0438"L"\x043d",
        L"\x043f"L"\x043e"L"\x0447"L"\x0442"L"\x0438", L"\x043c"L"\x043e"L"\x0439", L"\x0442"L"\x0435"L"\x043c",
        L"\x0447"L"\x0442"L"\x043e"L"\x0431"L"\x044b", L"\x043d"L"\x0435"L"\x0435", L"\x043a"L"\x0430"L"\x0436"L"\x0435"L"\x0442"L"\x0441"L"\x044f",
        L"\x0441"L"\x0435"L"\x0439"L"\x0447"L"\x0430"L"\x0441", L"\x0431"L"\x044b"L"\x043b"L"\x0438", L"\x043a"L"\x0443"L"\x0434"L"\x0430",
        L"\x0437"L"\x0430"L"\x0447"L"\x0435"L"\x043c", L"\x0441"L"\x043a"L"\x0430"L"\x0437"L"\x0430"L"\x0442"L"\x044c",
        L"\x0432"L"\x0441"L"\x0435"L"\x0445", L"\x043d"L"\x0438"L"\x043a"L"\x043e"L"\x0433"L"\x0434"L"\x0430",
        L"\x0441"L"\x0435"L"\x0433"L"\x043e"L"\x0434"L"\x043d"L"\x044f", L"\x043c"L"\x043e"L"\x0436"L"\x043d"L"\x043e",
        L"\x043f"L"\x0440"L"\x0438", L"\x043d"L"\x0430"L"\x043a"L"\x043e"L"\x043d"L"\x0435"L"\x0446", L"\x0434"L"\x0432"L"\x0430",
        L"\x043e"L"\x0431", L"\x0434"L"\x0440"L"\x0443"L"\x0433"L"\x043e"L"\x0439", L"\x0445"L"\x043e"L"\x0442"L"\x044c",
        L"\x043f"L"\x043e"L"\x0441"L"\x043b"L"\x0435", L"\x043d"L"\x0430"L"\x0434", L"\x0431"L"\x043e"L"\x043b"L"\x044c"L"\x0448"L"\x0435",
        L"\x0442"L"\x043e"L"\x0442", L"\x0447"L"\x0435"L"\x0440"L"\x0435"L"\x0437", L"\x044d"L"\x0442"L"\x0438",
        L"\x043d"L"\x0430"L"\x0441", L"\x043f"L"\x0440"L"\x043e", L"\x0432"L"\x0441"L"\x0435"L"\x0433"L"\x043e",
        L"\x043d"L"\x0438"L"\x0445", L"\x043a"L"\x0430"L"\x043a"L"\x0430"L"\x044f", L"\x043c"L"\x043d"L"\x043e"L"\x0433"L"\x043e",
        L"\x0440"L"\x0430"L"\x0437"L"\x0432"L"\x0435", L"\x0441"L"\x043a"L"\x0430"L"\x0437"L"\x0430"L"\x043b"L"\x0430",
        L"\x0442"L"\x0440"L"\x0438", L"\x044d"L"\x0442"L"\x0443", L"\x043c"L"\x043e"L"\x044f",
        L"\x0432"L"\x043f"L"\x0440"L"\x043e"L"\x0447"L"\x0435"L"\x043c", L"\x0445"L"\x043e"L"\x0440"L"\x043e"L"\x0448"L"\x043e",
        L"\x0441"L"\x0432"L"\x043e"L"\x044e", L"\x044d"L"\x0442"L"\x043e"L"\x0439", L"\x043f"L"\x0435"L"\x0440"L"\x0435"L"\x0434",
        L"\x0438"L"\x043d"L"\x043e"L"\x0433"L"\x0434"L"\x0430", L"\x043b"L"\x0443"L"\x0447"L"\x0448"L"\x0435",
        L"\x0447"L"\x0443"L"\x0442"L"\x044c", L"\x0442"L"\x043e"L"\x043c", L"\x043d"L"\x0435"L"\x043b"L"\x044c"L"\x0437"L"\x044f",
        L"\x0442"L"\x0430"L"\x043a"L"\x043e"L"\x0439", L"\x0438"L"\x043c", L"\x0431"L"\x043e"L"\x043b"L"\x0435"L"\x0435",
        L"\x0432"L"\x0441"L"\x0435"L"\x0433"L"\x0434"L"\x0430", L"\x043a"L"\x043e"L"\x043d"L"\x0435"L"\x0447"L"\x043d"L"\x043e",
        L"\x0432"L"\x0441"L"\x044e", L"\x043c"L"\x0435"L"\x0436"L"\x0434"L"\x0443"L""
    };

    RussianAnalyzer::RussianAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) ? getDefaultStopSet() : getDefaultStopSet30())
    {
    }

    RussianAnalyzer::RussianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }

    RussianAnalyzer::RussianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
        this->exclusionSet = exclusions;
    }

    RussianAnalyzer::~RussianAnalyzer()
    {
    }

    const HashSet<String> RussianAnalyzer::getDefaultStopSet30()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_RUSSIAN_STOP_WORDS_30, _RUSSIAN_STOP_WORDS_30 + SIZEOF_ARRAY(_RUSSIAN_STOP_WORDS_30));
        return stopSet;
    }

    const HashSet<String> RussianAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_RUSSIAN_STOP_WORDS_DEFAULT, _RUSSIAN_STOP_WORDS_DEFAULT + SIZEOF_ARRAY(_RUSSIAN_STOP_WORDS_DEFAULT));
        return stopSet;
    }

    TokenStreamComponentsPtr RussianAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31))
        {
            TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
            TokenStreamPtr result(newLucene<StandardFilter>(matchVersion, source));
            result = newLucene<LowerCaseFilter>(matchVersion, result);
            result = newLucene<StopFilter>(matchVersion, result, stopwords);
            if (!exclusionSet.empty())
                result = newLucene<KeywordMarkerFilter>(result, exclusionSet);
            result = newLucene<SnowballFilter>(result, L"russian");
            return newLucene<TokenStreamComponents>(source, result);
        }
        else
        {
            TokenizerPtr source(newLucene<RussianLetterTokenizer>(matchVersion, reader));
            TokenStreamPtr result(newLucene<LowerCaseFilter>(matchVersion, source));
            result = newLucene<StopFilter>(matchVersion, result, stopwords);
            if (!exclusionSet.empty())
                result = newLucene<KeywordMarkerFilter>(result, exclusionSet);
            return newLucene<TokenStreamComponents>(source, newLucene<RussianStemFilter>(result));
      }
    }
}

