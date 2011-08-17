/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "TurkishAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "TurkishLowerCaseFilter.h"
#include "StopFilter.h"
#include "KeywordMarkerFilter.h"
#include "SnowballFilter.h"

namespace Lucene
{
    /// From http://www.users.muohio.edu/canf/papers/JASIST2008offPrint.pdf
    const wchar_t* TurkishAnalyzer::_TURKISH_STOP_WORDS_DEFAULT[] =
    {
        L"acaba", L"altm\x0131"L"\x015f", L"alt\x0131", L"ama", L"ancak", L"arada",
        L"asl\x0131"L"nda", L"ayr\x0131"L"ca", L"bana", L"baz\x0131", L"belki", L"be",
        L"bende", L"beni", L"benim", L"beri", L"be\x015f", L"bile", L"bi", L"bir",
        L"bir\x00e7"L"ok", L"biri", L"birka\x00e7", L"birkez", L"bir\x015f"L"ey",
        L"bir\x015f"L"eyi", L"biz", L"bize", L"bizde", L"bizi", L"bizim", L"b\x00f6"L"yle",
        L"b\x00f6"L"ylece", L"bu", L"buna", L"bunda", L"bunda", L"bunlar",
        L"bunlar\x0131", L"bunlar\x0131", L"bunu", L"bunu", L"burada", L"\x00e7"L"ok",
        L"\x00e7"L"\x00fc"L"nk\x00fc", L"da", L"daha", L"dahi", L"de", L"defa",
        L"de\x011f"L"il", L"di\x011f"L"er", L"diye", L"doksa", L"dokuz", L"dolay\x0131",
        L"dolay\x0131"L"s\x0131"L"yla", L"d\x00f6"L"rt", L"edecek", L"ede", L"ederek",
        L"edilecek", L"ediliyor", L"edilmesi", L"ediyor", L"e\x011f"L"er", L"elli",
        L"e", L"etmesi", L"etti", L"etti\x011f"L"i", L"etti\x011f"L"ini", L"gibi",
        L"g\x00f6"L"re", L"hale", L"hangi", L"hatta", L"hem", L"hen\x00fc"L"z", L"hep",
        L"hepsi", L"her", L"herhangi", L"herkesi", L"hi\x00e7", L"hi\x00e7"L"bir",
        L"i\x00e7"L"i", L"iki", L"ile", L"ilgili", L"ise", L"i\x015f"L"te", L"itibare",
        L"itibariyle", L"kadar", L"kar\x015f"L"\x0131", L"katrilyo", L"kendi",
        L"kendilerine", L"kendini", L"kendisi", L"kendisine", L"kendisini", L"kez",
        L"ki", L"kim", L"kimde", L"kime", L"kimi", L"kimse", L"k\x0131"L"rk", L"milyar",
        L"milyo", L"mu", L"m\x00fc", L"m\x0131", L"as\x0131"L"l", L"e", L"ede",
        L"edenle", L"erde", L"erede", L"ereye", L"iye", L"i\x00e7"L"i", L"o", L"ola",
        L"olarak", L"oldu", L"oldu\x011f"L"u", L"oldu\x011f"L"unu", L"olduklar\x0131"L"n\x0131",
        L"olmad\x0131", L"olmad\x0131"L"\x011f"L"\x0131", L"olmak", L"olmas\x0131",
        L"olmaya", L"olmaz", L"olsa", L"olsu", L"olup", L"olur", L"olursa",
        L"oluyor", L"o", L"ona", L"onda", L"onlar", L"onlarda", L"onlar\x0131",
        L"onlar\x0131", L"onu", L"onu", L"otuz", L"oysa", L"\x00f6"L"yle", L"pek",
        L"ra\x011f"L"me", L"sadece", L"sanki", L"sekiz", L"sekse", L"se", L"sende",
        L"seni", L"seni", L"siz", L"sizde", L"sizi", L"sizi", L"\x015f"L"ey",
        L"\x015f"L"eyde", L"\x015f"L"eyi", L"\x015f"L"eyler", L"\x015f"L"\x00f6"L"yle", L"\x015f"L"u",
        L"\x015f"L"una", L"\x015f"L"unda", L"\x015f"L"unda", L"\x015f"L"unlar\x0131", L"\x015f"L"unu",
        L"taraf\x0131"L"nda", L"trilyo", L"t\x00fc"L"m", L"\x00fc"L"\x00e7", L"\x00fc"L"zere",
        L"var", L"vard\x0131", L"ve", L"veya", L"ya", L"yani", L"yapacak",
        L"yap\x0131"L"la", L"yap\x0131"L"lmas\x0131", L"yap\x0131"L"yor", L"yapmak",
        L"yapt\x0131", L"yapt\x0131"L"\x011f"L"\x0131", L"yapt\x0131"L"\x011f"L"\x0131"L"n\x0131",
        L"yapt\x0131"L"klar\x0131", L"yedi", L"yerine", L"yetmi\x015f", L"yine",
        L"yirmi", L"yoksa", L"y\x00fc"L"z", L"zate"
    };

    TurkishAnalyzer::TurkishAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, getDefaultStopSet())
    {
    }

    TurkishAnalyzer::TurkishAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }

    TurkishAnalyzer::TurkishAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
        this->exclusionSet = exclusions;
    }

    TurkishAnalyzer::~TurkishAnalyzer()
    {
    }

    const HashSet<String> TurkishAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_TURKISH_STOP_WORDS_DEFAULT, _TURKISH_STOP_WORDS_DEFAULT + SIZEOF_ARRAY(_TURKISH_STOP_WORDS_DEFAULT));
        return stopSet;
    }

    TokenStreamComponentsPtr TurkishAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
        TokenStreamPtr result(newLucene<StandardFilter>(matchVersion, source));
        result = newLucene<TurkishLowerCaseFilter>(result);
        result = newLucene<StopFilter>(matchVersion, result, stopwords);
        if (!exclusionSet.empty())
            result = newLucene<KeywordMarkerFilter>(result, exclusionSet);
        result = newLucene<SnowballFilter>(result, L"turkish");
        return newLucene<TokenStreamComponents>(source, result);
    }
}

