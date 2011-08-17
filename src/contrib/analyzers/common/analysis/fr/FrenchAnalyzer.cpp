/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "FrenchStemFilter.h"
#include "ElisionFilter.h"
#include "KeywordMarkerFilter.h"
#include "SnowballFilter.h"

namespace Lucene
{
    const wchar_t* FrenchAnalyzer::_FRENCH_STOP_WORDS_30[] =
    {
        L"a", L"afin", L"ai", L"ainsi", L"apr\x00e8"L"s", L"attendu", L"au", L"aujourd", L"auquel", L"aussi",
        L"autre", L"autres", L"aux", L"auxquelles", L"auxquels", L"avait", L"avant", L"avec", L"avoir",
        L"c", L"car", L"ce", L"ceci", L"cela", L"celle", L"celles", L"celui", L"cependant", L"certain",
        L"certaine", L"certaines", L"certains", L"ces", L"cet", L"cette", L"ceux", L"chez", L"ci",
        L"combien", L"comme", L"comment", L"concernant", L"contre", L"d", L"dans", L"de", L"debout",
        L"dedans", L"dehors", L"del\x00e0", L"depuis", L"derri\x00e8"L"re", L"des", L"d\x00e9"L"sormais",
        L"desquelles", L"desquels", L"dessous", L"dessus", L"devant", L"devers", L"devra", L"divers",
        L"diverse", L"diverses", L"doit", L"donc", L"dont", L"du", L"duquel", L"durant", L"d\x00e8"L"s",
        L"elle", L"elles", L"en", L"entre", L"environ", L"est", L"et", L"etc", L"etre", L"eu", L"eux",
        L"except\x00e9", L"hormis", L"hors", L"h\x00e9"L"las", L"hui", L"il", L"ils", L"j", L"je", L"jusqu",
        L"jusque", L"l", L"la", L"laquelle", L"le", L"lequel", L"les", L"lesquelles", L"lesquels", L"leur",
        L"leurs", L"lorsque", L"lui", L"l\x00e0", L"ma", L"mais", L"malgr\x00e9", L"me", L"merci", L"mes",
        L"mien", L"mienne", L"miennes", L"miens", L"moi", L"moins", L"mon", L"moyennant", L"m\x00ea"L"me",
        L"m\x00ea"L"mes", L"n", L"ne", L"ni", L"non", L"nos", L"notre", L"nous", L"n\x00e9"L"anmoins",
        L"n\x00f4"L"tre", L"n\x00f4"L"tres", L"on", L"ont", L"ou", L"outre", L"o\x00f9", L"par", L"parmi",
        L"partant", L"pas", L"pass\x00e9", L"pendant", L"plein", L"plus", L"plusieurs", L"pour", L"pourquoi",
        L"proche", L"pr\x00e8"L"s", L"puisque", L"qu", L"quand", L"que", L"quel", L"quelle", L"quelles",
        L"quels", L"qui", L"quoi", L"quoique", L"revoici", L"revoil\x00e0", L"s", L"sa", L"sans", L"sauf",
        L"se", L"selon", L"seront", L"ses", L"si", L"sien", L"sienne", L"siennes", L"siens", L"sinon",
        L"soi", L"soit", L"son", L"sont", L"sous", L"suivant", L"sur", L"ta", L"te", L"tes", L"tien",
        L"tienne", L"tiennes", L"tiens", L"toi", L"ton", L"tous", L"tout", L"toute", L"toutes", L"tu", L"un",
        L"une", L"va", L"vers", L"voici", L"voil\x00e0", L"vos", L"votre", L"vous", L"vu", L"v\x00f4"L"tre",
        L"v\x00f4"L"tres", L"y", L"\x00e0", L"\x00e7"L"a", L"\x00e8"L"s", L"\x00e9"L"t\x00e9", L"\x00ea"L"tre", L"\x00f4"L""
    };

    /// From svn.tartarus.org/snowball/trunk/website/algorithms/french/stop.txt
    /// This file is distributed under the BSD License.
    const wchar_t* FrenchAnalyzer::_FRENCH_STOP_WORDS_DEFAULT[] =
    {
        L"au", L"aux", L"avec", L"ce", L"ces", L"dans", L"de", L"des", L"du", L"elle", L"en", L"et",
        L"eux", L"il", L"je", L"la", L"le", L"leur", L"lui", L"ma", L"mais", L"me", L"m\x00ea"L"me",
        L"mes", L"moi", L"mon", L"e", L"os", L"otre", L"ous", L"on", L"ou", L"par", L"pas", L"pour",
        L"qu", L"que", L"qui", L"sa", L"se", L"ses", L"son", L"sur", L"ta", L"te", L"tes", L"toi",
        L"ton", L"tu", L"un", L"une", L"vos", L"votre", L"vous", L"c", L"d", L"j", L"l", L"\x00e0",
        L"m", L"n", L"s", L"t", L"y", L"\x00e9"L"t\x00e9", L"\x00e9"L"t\x00e9"L"e", L"\x00e9"L"t\x00e9"L"es",
        L"\x00e9"L"t\x00e9"L"s", L"\x00e9"L"tant", L"suis", L"es", L"est", L"sommes", L"\x00ea"L"tes", L"sont",
        L"serai", L"seras", L"sera", L"serons", L"serez", L"seront", L"serais", L"serait", L"serions",
        L"seriez", L"seraient", L"\x00e9"L"tais", L"\x00e9"L"tait", L"\x00e9"L"tions", L"\x00e9"L"tiez",
        L"\x00e9"L"taient", L"fus", L"fut", L"f\x00fb"L"mes", L"f\x00fb"L"tes", L"furent", L"sois", L"soit",
        L"soyons", L"soyez", L"soient", L"fusse", L"fusses", L"f\x00fb"L"t", L"fussions", L"fussiez",
        L"fussent", L"ayant", L"eu", L"eue", L"eues", L"eus", L"ai", L"as", L"avons", L"avez",
        L"ont", L"aurai", L"auras", L"aura", L"aurons", L"aurons", L"aurez", L"auront", L"aurais",
        L"aurait", L"aurions", L"auriez", L"auraient", L"avais", L"avait", L"avions", L"aviez",
        L"avaient", L"eut", L"e\x00fb"L"mes", L"e\x00fb"L"tes", L"eurent", L"aie", L"eirs", L"ait",
        L"ayons", L"ayez", L"aient", L"eusse", L"eusse", L"eusses", L"e\x00fb"L"t", L"eussions",
        L"eussiez", L"eussent", L"ceci", L"cel\x00e0"L"\x00a0", L"cet", L"cette", L"ici", L"ils",
        L"les", L"leurs", L"quel", L"quels", L"quelle", L"quelles", L"sans", L"soi"
    };

    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31) ? getDefaultStopSet() : getDefaultStopSet30())
    {
    }

    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }

    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
        this->excltable = exclusions;
    }

    FrenchAnalyzer::~FrenchAnalyzer()
    {
    }

    const HashSet<String> FrenchAnalyzer::getDefaultStopSet30()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_FRENCH_STOP_WORDS_30, _FRENCH_STOP_WORDS_30 + SIZEOF_ARRAY(_FRENCH_STOP_WORDS_30));
        return stopSet;
    }

    const HashSet<String> FrenchAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_FRENCH_STOP_WORDS_DEFAULT, _FRENCH_STOP_WORDS_DEFAULT + SIZEOF_ARRAY(_FRENCH_STOP_WORDS_DEFAULT));
        return stopSet;
    }

    void FrenchAnalyzer::setStemExclusionTable(HashSet<String> exclusions)
    {
        excltable = exclusions;
        setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
    }

    TokenStreamComponentsPtr FrenchAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        if (LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_31))
        {
            TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
            TokenStreamPtr result(newLucene<StandardFilter>(matchVersion, source));
            result = newLucene<ElisionFilter>(matchVersion, result);
            result = newLucene<LowerCaseFilter>(matchVersion, result);
            result = newLucene<StopFilter>(matchVersion, result, stopwords);
            if (!excltable.empty())
                result = newLucene<KeywordMarkerFilter>(result, excltable);
            result = newLucene<SnowballFilter>(result, L"french");
            return newLucene<TokenStreamComponents>(source, result);
        }
        else
        {
            TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
            TokenStreamPtr result(newLucene<StandardFilter>(matchVersion, source));
            result = newLucene<StopFilter>(matchVersion, result, stopwords);
            if (!excltable.empty())
                result = newLucene<KeywordMarkerFilter>(result, excltable);
            result = newLucene<FrenchStemFilter>(result);
            // Convert to lowercase after stemming!
            return newLucene<TokenStreamComponents>(source, newLucene<LowerCaseFilter>(matchVersion, result));
        }
    }
}

