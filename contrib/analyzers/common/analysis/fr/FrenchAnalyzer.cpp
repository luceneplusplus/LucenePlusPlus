/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrenchAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "FrenchStemFilter.h"

namespace Lucene
{
    const wchar_t* FrenchAnalyzer::_FRENCH_STOP_WORDS[] = 
    {
        L"a", L"afin", L"ai", L"ainsi", L"apr\u00e8s", L"attendu", L"au", L"aujourd", L"auquel", L"aussi",
        L"autre", L"autres", L"aux", L"auxquelles", L"auxquels", L"avait", L"avant", L"avec", L"avoir",
        L"c", L"car", L"ce", L"ceci", L"cela", L"celle", L"celles", L"celui", L"cependant", L"certain",
        L"certaine", L"certaines", L"certains", L"ces", L"cet", L"cette", L"ceux", L"chez", L"ci",
        L"combien", L"comme", L"comment", L"concernant", L"contre", L"d", L"dans", L"de", L"debout",
        L"dedans", L"dehors", L"del\u00e0", L"depuis", L"derri\u00e8re", L"des", L"d\u00e9sormais", 
        L"desquelles", L"desquels", L"dessous", L"dessus", L"devant", L"devers", L"devra", L"divers", 
        L"diverse", L"diverses", L"doit", L"donc", L"dont", L"du", L"duquel", L"durant", L"d\u00e8s", 
        L"elle", L"elles", L"en", L"entre", L"environ", L"est", L"et", L"etc", L"etre", L"eu", L"eux", 
        L"except\u00e9", L"hormis", L"hors", L"h\u00e9las", L"hui", L"il", L"ils", L"j", L"je", L"jusqu", 
        L"jusque", L"l", L"la", L"laquelle", L"le", L"lequel", L"les", L"lesquelles", L"lesquels", L"leur", 
        L"leurs", L"lorsque", L"lui", L"l\u00e0", L"ma", L"mais", L"malgr\u00e9", L"me", L"merci", L"mes", 
        L"mien", L"mienne", L"miennes", L"miens", L"moi", L"moins", L"mon", L"moyennant", L"m\u00eame", 
        L"m\u00eames", L"n", L"ne", L"ni", L"non", L"nos", L"notre", L"nous", L"n\u00e9anmoins", 
        L"n\u00f4tre", L"n\u00f4tres", L"on", L"ont", L"ou", L"outre", L"o\u00f9", L"par", L"parmi", 
        L"partant", L"pas", L"pass\u00e9", L"pendant", L"plein", L"plus", L"plusieurs", L"pour", L"pourquoi",
        L"proche", L"pr\u00e8s", L"puisque", L"qu", L"quand", L"que", L"quel", L"quelle", L"quelles", 
        L"quels", L"qui", L"quoi", L"quoique", L"revoici", L"revoil\u00e0", L"s", L"sa", L"sans", L"sauf", 
        L"se", L"selon", L"seront", L"ses", L"si", L"sien", L"sienne", L"siennes", L"siens", L"sinon", 
        L"soi", L"soit", L"son", L"sont", L"sous", L"suivant", L"sur", L"ta", L"te", L"tes", L"tien", 
        L"tienne", L"tiennes", L"tiens", L"toi", L"ton", L"tous", L"tout", L"toute", L"toutes", L"tu", L"un", 
        L"une", L"va", L"vers", L"voici", L"voil\u00e0", L"vos", L"votre", L"vous", L"vu", L"v\u00f4tre", 
        L"v\u00f4tres", L"y", L"\u00e0", L"\u00e7a", L"\u00e8s", L"\u00e9t\u00e9", L"\u00eatre", L"\u00f4"
    };
    
    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion)
    {
        this->stoptable = getDefaultStopSet();
        this->matchVersion = matchVersion;
    }
    
    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords)
    {
        this->stoptable = stopwords;
        this->matchVersion = matchVersion;
    }
    
    FrenchAnalyzer::FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions)
    {
        this->stoptable = stopwords;
        this->excltable = exclusions;
        this->matchVersion = matchVersion;
    }
    
    FrenchAnalyzer::~FrenchAnalyzer()
    {
    }
    
    HashSet<String> FrenchAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stoptable;
        if (!stoptable)
            stoptable = HashSet<String>::newInstance(_FRENCH_STOP_WORDS, _FRENCH_STOP_WORDS + SIZEOF_ARRAY(_FRENCH_STOP_WORDS));
        return stoptable;
    }
    
    void FrenchAnalyzer::setStemExclusionTable(HashSet<String> exclusions)
    {
        excltable = exclusions;
        setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
    }
    
    TokenStreamPtr FrenchAnalyzer::tokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenStreamPtr result = newLucene<StandardTokenizer>(matchVersion, reader);
        result = newLucene<StandardFilter>(result);
        result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), result, stoptable);
        result = newLucene<FrenchStemFilter>(result, excltable);
        // Convert to lowercase after stemming
        result = newLucene<LowerCaseFilter>(result);        
        return result;
    }
    
    TokenStreamPtr FrenchAnalyzer::reusableTokenStream(const String& fieldName, ReaderPtr reader)
    {
        FrenchAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<FrenchAnalyzerSavedStreams>(getPreviousTokenStream()));
        if (!streams)
        {
            streams = newLucene<FrenchAnalyzerSavedStreams>();
            streams->source = newLucene<StandardTokenizer>(matchVersion, reader);
            streams->result = newLucene<StandardFilter>(streams->source);
            streams->result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->result, stoptable);
            streams->result = newLucene<FrenchStemFilter>(streams->result, excltable);
            // Convert to lowercase after stemming
            streams->result = newLucene<LowerCaseFilter>(streams->result);            
            setPreviousTokenStream(streams);
        }
        else
            streams->source->reset(reader);
        return streams->result;
    }
    
    FrenchAnalyzerSavedStreams::~FrenchAnalyzerSavedStreams()
    {
    }
}
