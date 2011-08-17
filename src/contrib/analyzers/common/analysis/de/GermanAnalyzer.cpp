/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "KeywordMarkerFilter.h"
#include "GermanStemFilter.h"
#include "SnowballFilter.h"

namespace Lucene
{
    const wchar_t* GermanAnalyzer::_GERMAN_STOP_WORDS_30[] =
    {
        L"einer", L"eine", L"eines", L"einem", L"einen", L"der", L"die",
        L"das", L"dass", L"da\x00df", L"du", L"er", L"sie", L"es", L"was",
        L"wer", L"wie", L"wir", L"und", L"oder", L"ohne", L"mit", L"am",
        L"im", L"in", L"aus", L"auf", L"ist", L"sein", L"war", L"wird",
        L"ihr", L"ihre", L"ihres", L"als", L"f\x00fc"L"r", L"von", L"mit",
        L"dich", L"dir", L"mich", L"mir", L"mein", L"sein", L"kein",
        L"durch", L"wegen", L"wird"
    };

    /// From svn.tartarus.org/snowball/trunk/website/algorithms/german/stop.txt
    /// This file is distributed under the BSD License.
    const wchar_t* GermanAnalyzer::_GERMAN_STOP_WORDS_DEFAULT[] =
    {
        L"aber", L"alle", L"allem", L"allen", L"aller", L"alles", L"als",
        L"also", L"am", L"an", L"ander", L"andere", L"anderem", L"anderen",
        L"anderer", L"anderes", L"anderm", L"andern", L"anderr", L"anders",
        L"auch", L"auf", L"aus", L"bei", L"bin", L"bis", L"bist", L"da",
        L"damit", L"dann", L"der", L"den", L"des", L"dem", L"die", L"das",
        L"da\x00df", L"derselbe", L"derselben", L"denselben", L"desselben",
        L"demselben", L"dieselbe", L"dieselben", L"dasselbe", L"dazu", L"dein",
        L"deine", L"deinem", L"deinen", L"deiner", L"deines", L"denn", L"derer",
        L"dessen", L"dich", L"dir", L"du", L"dies", L"diese", L"diesem",
        L"diesen", L"dieser", L"dieses", L"doch", L"dort", L"durch", L"ein",
        L"eine", L"einem", L"einen", L"einer", L"eines", L"einig", L"einige",
        L"einigem", L"einigen", L"einiger", L"einiges", L"einmal", L"er",
        L"ihn", L"ihm", L"es", L"etwas", L"euer", L"eure", L"eurem", L"euren",
        L"eurer", L"eures", L"f\x00fc"L"r", L"gegen", L"gewesen", L"hab", L"habe",
        L"haben", L"hat", L"hatte", L"hatten", L"hier", L"hin", L"hinter",
        L"ich", L"mich", L"mir", L"ihr", L"ihre", L"ihrem", L"ihren", L"ihrer",
        L"ihres", L"euch", L"im", L"in", L"indem", L"ins", L"ist", L"jede",
        L"jedem", L"jeden", L"jeder", L"jedes", L"jene", L"jenem", L"jenen",
        L"jener", L"jenes", L"jetzt", L"kann", L"kein", L"keine", L"keinem",
        L"keinen", L"keiner", L"keines", L"k\x00f6"L"nnen", L"k\x00f6"L"nnte",
        L"machen", L"man", L"manche", L"manchem", L"manchen", L"mancher",
        L"manches", L"mein", L"meine", L"meinem", L"meinen", L"meiner",
        L"meines", L"mit", L"muss", L"musste", L"nach", L"nicht", L"nichts",
        L"noch", L"nun", L"nur", L"ob", L"oder", L"ohne", L"sehr", L"sein",
        L"seine", L"seinem", L"seinen", L"seiner", L"seines", L"selbst", L"sich",
        L"sie", L"ihnen", L"sind", L"so", L"solche", L"solchem", L"solchen",
        L"solcher", L"solches", L"soll", L"sollte", L"sondern", L"sonst",
        L"\x00fc"L"ber", L"um", L"und", L"uns", L"unse", L"unsem", L"unsen",
        L"unser", L"unses", L"unter", L"viel", L"vom", L"von", L"vor",
        L"w\x00e4"L"hrend", L"war", L"waren", L"warst", L"was", L"weg", L"weil",
        L"weiter", L"welche", L"welchem", L"welchen", L"welcher", L"welches",
        L"wenn", L"werde", L"werden", L"wie", L"wieder", L"will", L"wir",
        L"wird", L"wirst", L"wo", L"wollen", L"wollte", L"w\x00fc"L"rde",
        L"w\x00fc"L"rden", L"zu", L"zum", L"zur", L"zwar", L"zwischen"
    };

    GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) ? getDefaultStopSet() : getDefaultStopSet30())
    {
    }

    GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }

    GermanAnalyzer::GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
        this->exclusionSet = exclusions;
    }

    GermanAnalyzer::~GermanAnalyzer()
    {
    }

    const HashSet<String> GermanAnalyzer::getDefaultStopSet30()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_GERMAN_STOP_WORDS_30, _GERMAN_STOP_WORDS_30 + SIZEOF_ARRAY(_GERMAN_STOP_WORDS_30));
        return stopSet;
    }

    const HashSet<String> GermanAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_GERMAN_STOP_WORDS_DEFAULT, _GERMAN_STOP_WORDS_DEFAULT + SIZEOF_ARRAY(_GERMAN_STOP_WORDS_DEFAULT));
        return stopSet;
    }

    void GermanAnalyzer::setStemExclusionTable(HashSet<String> exclusions)
    {
        exclusionSet = exclusions;
        setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
    }

    TokenStreamComponentsPtr GermanAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
        TokenStreamPtr result(newLucene<StandardFilter>(matchVersion, source));
        result = newLucene<LowerCaseFilter>(matchVersion, result);
        result = newLucene<StopFilter>(matchVersion, result, stopwords);
        result = newLucene<KeywordMarkerFilter>(result, exclusionSet);
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31))
            result = newLucene<SnowballFilter>(result, L"german");
        else
            result = newLucene<GermanStemFilter>(result);
        return newLucene<TokenStreamComponents>(source, result);
    }
}

