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
        L"\x0430", L"\x0431\x0435\x0437", L"\x0431\x043e\x043b\x0435\x0435", L"\x0431\x044b",
        L"\x0431\x044b\x043b", L"\x0431\x044b\x043b\x0430", L"\x0431\x044b\x043b\x0438",
        L"\x0431\x044b\x043b\x043e", L"\x0431\x044b\x0442\x044c", L"\x0432",
        L"\x0432\x0430\x043c", L"\x0432\x0430\x0441", L"\x0432\x0435\x0441\x044c",
        L"\x0432\x043e", L"\x0432\x043e\x0442", L"\x0432\x0441\x0435", L"\x0432\x0441\x0435\x0433\x043e",
        L"\x0432\x0441\x0435\x0445", L"\x0432\x044b", L"\x0433\x0434\x0435", L"\x0434\x0430",
        L"\x0434\x0430\x0436\x0435", L"\x0434\x043b\x044f", L"\x0434\x043e", L"\x0435\x0433\x043e",
        L"\x0435\x0435", L"\x0435\x0439", L"\x0435\x044e", L"\x0435\x0441\x043b\x0438",
        L"\x0435\x0441\x0442\x044c", L"\x0435\x0449\x0435", L"\x0436\x0435", L"\x0437\x0430",
        L"\x0437\x0434\x0435\x0441\x044c", L"\x0438", L"\x0438\x0437", L"\x0438\x043b\x0438",
        L"\x0438\x043c", L"\x0438\x0445", L"\x043a", L"\x043a\x0430\x043a", L"\x043a\x043e",
        L"\x043a\x043e\x0433\x0434\x0430", L"\x043a\x0442\x043e", L"\x043b\x0438",
        L"\x043b\x0438\x0431\x043e", L"\x043c\x043d\x0435", L"\x043c\x043e\x0436\x0435\x0442",
        L"\x043c\x044b", L"\x043d\x0430", L"\x043d\x0430\x0434\x043e", L"\x043d\x0430\x0448",
        L"\x043d\x0435", L"\x043d\x0435\x0433\x043e", L"\x043d\x0435\x0435", L"\x043d\x0435\x0442",
        L"\x043d\x0438", L"\x043d\x0438\x0445", L"\x043d\x043e", L"\x043d\x0443", L"\x043e",
        L"\x043e\x0431", L"\x043e\x0434\x043d\x0430\x043a\x043e", L"\x043e\x043d",
        L"\x043e\x043d\x0430", L"\x043e\x043d\x0438", L"\x043e\x043d\x043e", L"\x043e\x0442",
        L"\x043e\x0447\x0435\x043d\x044c", L"\x043f\x043e", L"\x043f\x043e\x0434", L"\x043f\x0440\x0438",
        L"\x0441", L"\x0441\x043e", L"\x0442\x0430\x043a", L"\x0442\x0430\x043a\x0436\x0435",
        L"\x0442\x0430\x043a\x043e\x0439", L"\x0442\x0430\x043c", L"\x0442\x0435", L"\x0442\x0435\x043c",
        L"\x0442\x043e", L"\x0442\x043e\x0433\x043e", L"\x0442\x043e\x0436\x0435", L"\x0442\x043e\x0439",
        L"\x0442\x043e\x043b\x044c\x043a\x043e", L"\x0442\x043e\x043c", L"\x0442\x044b", L"\x0443",
        L"\x0443\x0436\x0435", L"\x0445\x043e\x0442\x044f", L"\x0447\x0435\x0433\x043e",
        L"\x0447\x0435\x0439", L"\x0447\x0435\x043c", L"\x0447\x0442\x043e",
        L"\x0447\x0442\x043e\x0431\x044b", L"\x0447\x044c\x0435", L"\x0447\x044c\x044f",
        L"\x044d\x0442\x0430", L"\x044d\x0442\x0438", L"\x044d\x0442\x043e", L"\x044f"
    };

    /// From svn.tartarus.org/snowball/trunk/website/algorithms/russian/stop.txt
    /// This file is distributed under the BSD License.
    const wchar_t* RussianAnalyzer::_RUSSIAN_STOP_WORDS_DEFAULT[] =
    {
        L"\x0438", L"\x0432", L"\x0432\x043e", L"\x043d\x0435", L"\x0447\x0442\x043e", L"\x043e\x043d",
        L"\x043d\x0430", L"\x044f", L"\x0441", L"\x0441\x043e", L"\x043a\x0430\x043a", L"\x0430",
        L"\x0442\x043e", L"\x0432\x0441\x0435", L"\x043e\x043d\x0430", L"\x0442\x0430\x043a",
        L"\x0435\x0433\x043e", L"\x043d\x043e", L"\x0434\x0430", L"\x0442\x044b", L"\x043a", L"\x0443",
        L"\x0436\x0435", L"\x0432\x044b", L"\x0437\x0430", L"\x0431\x044b", L"\x043f\x043e",
        L"\x0442\x043e\x043b\x044c", L"\x0435\x0435", L"\x043c\x043d\x0435", L"\x0431\x044b\x043b\x043e",
        L"\x0432\x043e\x0442", L"\x043e\x0442", L"\x043c\x0435\x043d\x044f", L"\x0435\x0449\x0435",
        L"\x043d\x0435\x0442", L"\x043e", L"\x0438\x0437", L"\x0435\x043c\x0443",
        L"\x0442\x0435\x043f\x0435\x0440\x044c", L"\x043a\x043e\x0433\x0434\x0430",
        L"\x0434\x0430\x0436\x0435", L"\x043d\x0443", L"\x0432\x0434\x0440\x0443\x0433", L"\x043b\x0438",
        L"\x0435\x0441\x043b\x0438", L"\x0443\x0436\x0435", L"\x0438\x043b\x0438", L"\x043d\x0438",
        L"\x0431\x044b\x0442\x044c", L"\x0431\x044b\x043b", L"\x043d\x0435\x0433\x043e", L"\x0434\x043e",
        L"\x0432\x0430\x0441", L"\x043d\x0438\x0431\x0443\x0434\x044c", L"\x043e\x043f\x044f\x0442\x044c",
        L"\x0443\x0436", L"\x0432\x0430\x043c", L"\x0441\x043a\x0430\x0437\x0430\x043b",
        L"\x0432\x0435\x0434\x044c", L"\x0442\x0430\x043c", L"\x043f\x043e\x0442\x043e\x043c",
        L"\x0441\x0435\x0431\x044f", L"\x043d\x0438\x0447\x0435\x0433\x043e", L"\x0435\x0439",
        L"\x043c\x043e\x0436\x0435\x0442", L"\x043e\x043d\x0438", L"\x0442\x0443\x0442",
        L"\x0433\x0434\x0435", L"\x0435\x0441\x0442\x044c", L"\x043d\x0430\x0434\x043e",
        L"\x043d\x0435\x0439", L"\x0434\x043b\x044f", L"\x043c\x044b", L"\x0442\x0435\x0431\x044f",
        L"\x0438\x0445", L"\x0447\x0435\x043c", L"\x0431\x044b\x043b\x0430", L"\x0441\x0430\x043c",
        L"\x0447\x0442\x043e\x0431", L"\x0431\x0435\x0437", L"\x0431\x0443\x0434\x0442\x043e",
        L"\x0447\x0435\x043b\x043e\x0432\x0435\x043a", L"\x0447\x0435\x0433\x043e", L"\x0440\x0430\x0437",
        L"\x0442\x043e\x0436\x0435", L"\x0441\x0435\x0431\x0435", L"\x043f\x043e\x0434",
        L"\x0436\x0438\x0437\x043d\x044c", L"\x0431\x0443\x0434\x0435\x0442", L"\x0436",
        L"\x0442\x043e\x0433\x0434\x0430", L"\x043a\x0442\x043e", L"\x044d\x0442\x043e\x0442",
        L"\x0433\x043e\x0432\x043e\x0440\x0438\x043b", L"\x0442\x043e\x0433\x043e",
        L"\x043f\x043e\x0442\x043e\x043c\x0443", L"\x044d\x0442\x043e\x0433\x043e",
        L"\x043a\x0430\x043a\x043e\x0439", L"\x0441\x043e\x0432\x0441\x0435\x043c", L"\x043d\x0438\x043c",
        L"\x0437\x0434\x0435\x0441\x044c", L"\x044d\x0442\x043e\x043c", L"\x043e\x0434\x0438\x043d",
        L"\x043f\x043e\x0447\x0442\x0438", L"\x043c\x043e\x0439", L"\x0442\x0435\x043c",
        L"\x0447\x0442\x043e\x0431\x044b", L"\x043d\x0435\x0435", L"\x043a\x0430\x0436\x0435\x0442\x0441\x044f",
        L"\x0441\x0435\x0439\x0447\x0430\x0441", L"\x0431\x044b\x043b\x0438", L"\x043a\x0443\x0434\x0430",
        L"\x0437\x0430\x0447\x0435\x043c", L"\x0441\x043a\x0430\x0437\x0430\x0442\x044c",
        L"\x0432\x0441\x0435\x0445", L"\x043d\x0438\x043a\x043e\x0433\x0434\x0430",
        L"\x0441\x0435\x0433\x043e\x0434\x043d\x044f", L"\x043c\x043e\x0436\x043d\x043e",
        L"\x043f\x0440\x0438", L"\x043d\x0430\x043a\x043e\x043d\x0435\x0446", L"\x0434\x0432\x0430",
        L"\x043e\x0431", L"\x0434\x0440\x0443\x0433\x043e\x0439", L"\x0445\x043e\x0442\x044c",
        L"\x043f\x043e\x0441\x043b\x0435", L"\x043d\x0430\x0434", L"\x0431\x043e\x043b\x044c\x0448\x0435",
        L"\x0442\x043e\x0442", L"\x0447\x0435\x0440\x0435\x0437", L"\x044d\x0442\x0438",
        L"\x043d\x0430\x0441", L"\x043f\x0440\x043e", L"\x0432\x0441\x0435\x0433\x043e",
        L"\x043d\x0438\x0445", L"\x043a\x0430\x043a\x0430\x044f", L"\x043c\x043d\x043e\x0433\x043e",
        L"\x0440\x0430\x0437\x0432\x0435", L"\x0441\x043a\x0430\x0437\x0430\x043b\x0430",
        L"\x0442\x0440\x0438", L"\x044d\x0442\x0443", L"\x043c\x043e\x044f",
        L"\x0432\x043f\x0440\x043e\x0447\x0435\x043c", L"\x0445\x043e\x0440\x043e\x0448\x043e",
        L"\x0441\x0432\x043e\x044e", L"\x044d\x0442\x043e\x0439", L"\x043f\x0435\x0440\x0435\x0434",
        L"\x0438\x043d\x043e\x0433\x0434\x0430", L"\x043b\x0443\x0447\x0448\x0435",
        L"\x0447\x0443\x0442\x044c", L"\x0442\x043e\x043c", L"\x043d\x0435\x043b\x044c\x0437\x044f",
        L"\x0442\x0430\x043a\x043e\x0439", L"\x0438\x043c", L"\x0431\x043e\x043b\x0435\x0435",
        L"\x0432\x0441\x0435\x0433\x0434\x0430", L"\x043a\x043e\x043d\x0435\x0447\x043d\x043e",
        L"\x0432\x0441\x044e", L"\x043c\x0435\x0436\x0434\x0443"
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

