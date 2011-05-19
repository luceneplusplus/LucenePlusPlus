/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "CJKAnalyzer.h"
#include "CJKTokenizer.h"
#include "StopFilter.h"

namespace Lucene
{
    const wchar_t* CJKAnalyzer::_STOP_WORDS[] = 
    {
        L"a", L"and", L"are", L"as", L"at", L"be",
        L"but", L"by", L"for", L"if", L"in", L"into", 
        L"is", L"it", L"no", L"not", L"of", L"on", 
        L"or", L"s", L"such", L"t", L"that", L"the", 
        L"their", L"then", L"there", L"these", 
        L"they", L"this", L"to", L"was", L"will", 
        L"with", L"", L"www"
    };
    
    CJKAnalyzer::CJKAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion, getDefaultStopSet())
    {
    }
    
    CJKAnalyzer::CJKAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords) : StopwordAnalyzerBase(matchVersion, stopwords)
    {
    }
    
    CJKAnalyzer::~CJKAnalyzer()
    {
    }
    
    const HashSet<String> CJKAnalyzer::getDefaultStopSet()
    {
        static HashSet<String> stopSet;
        if (!stopSet)
            stopSet = HashSet<String>::newInstance(_STOP_WORDS, _STOP_WORDS + SIZEOF_ARRAY(_STOP_WORDS));
        return stopSet;
    }
    
    TokenStreamComponentsPtr CJKAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<CJKTokenizer>(reader));
        return newLucene<TokenStreamComponents>(source, newLucene<StopFilter>(matchVersion, source, stopwords));
    }
}
