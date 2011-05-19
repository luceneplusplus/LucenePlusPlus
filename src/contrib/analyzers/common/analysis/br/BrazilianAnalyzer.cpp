/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "BrazilianAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopFilter.h"
#include "BrazilianStemFilter.h"
#include "KeywordMarkerFilter.h"

namespace Lucene
{
    const wchar_t* BrazilianAnalyzer::_DEFAULT_STOP_WORDS[] = 
    {
        L"a", L"ainda", L"alem", L"ambas", L"ambos", L"antes",
        L"ao", L"aonde", L"aos", L"apos", L"aquele", L"aqueles",
        L"as", L"assim", L"com", L"como", L"contra", L"contudo",
        L"cuja", L"cujas", L"cujo", L"cujos", L"da", L"das", L"de",
        L"dela", L"dele", L"deles", L"demais", L"depois", L"desde",
        L"desta", L"deste", L"dispoe", L"dispoem", L"diversa",
        L"diversas", L"diversos", L"do", L"dos", L"durante", L"e",
        L"ela", L"elas", L"ele", L"eles", L"em", L"entao", L"entre",
        L"essa", L"essas", L"esse", L"esses", L"esta", L"estas",
        L"este", L"estes", L"ha", L"isso", L"isto", L"logo", L"mais",
        L"mas", L"mediante", L"menos", L"mesma", L"mesmas", L"mesmo",
        L"mesmos", L"na", L"nas", L"nao", L"nas", L"nem", L"nesse", 
        L"neste", L"nos", L"o", L"os", L"ou", L"outra", L"outras", 
        L"outro", L"outros", L"pelas", L"pelas", L"pelo", L"pelos", 
        L"perante", L"pois", L"por", L"porque", L"portanto", 
        L"proprio", L"propios", L"quais", L"qual", L"qualquer", 
        L"quando", L"quanto", L"que", L"quem", L"quer", L"se", L"seja", 
        L"sem", L"sendo", L"seu", L"seus", L"sob", L"sobre", L"sua",
        L"suas", L"tal", L"tambem", L"teu", L"teus", L"toda", L"todas", 
        L"todo", L"todos", L"tua", L"tuas", L"tudo", L"um", L"uma", 
        L"umas", L"uns"
    };
    
    BrazilianAnalyzer::BrazilianAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, getDefaultStopSet())
    {
    }
    
    BrazilianAnalyzer::BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }
    
    BrazilianAnalyzer::BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
        this->excltable = exclusions;
    }
    
    BrazilianAnalyzer::~BrazilianAnalyzer()
    {
    }
    
    const HashSet<String> BrazilianAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_DEFAULT_STOP_WORDS, _DEFAULT_STOP_WORDS + SIZEOF_ARRAY(_DEFAULT_STOP_WORDS));
        return stopSet;
    }
    
    void BrazilianAnalyzer::setStemExclusionTable(HashSet<String> exclusions)
    {
        excltable = exclusions;
        setPreviousTokenStream(LuceneObjectPtr()); // force a new stemmer to be created
    }
    
    TokenStreamComponentsPtr BrazilianAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<StandardTokenizer>(matchVersion, reader));
        TokenStreamPtr result(newLucene<LowerCaseFilter>(matchVersion, source));
        result = newLucene<StandardFilter>(matchVersion, result);
        result = newLucene<StopFilter>(matchVersion, result, stopwords);
        if (excltable && !excltable.empty())
            result = newLucene<KeywordMarkerFilter>(result, excltable);
        return newLucene<TokenStreamComponents>(source, newLucene<BrazilianStemFilter>(result));
    }
}
