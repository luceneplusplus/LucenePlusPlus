/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StopAnalyzer.h"
#include "StopFilter.h"
#include "WordlistLoader.h"
#include "Reader.h"
#include "LowerCaseTokenizer.h"
#include "CharArraySet.h"

namespace Lucene
{
    StopAnalyzer::StopAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion)
    {
        this->stopwords = newLucene<CharArraySet>(matchVersion, ENGLISH_STOP_WORDS_SET(), false);
    }
    
    StopAnalyzer::StopAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords) : StopwordAnalyzerBase(matchVersion, stopWords)
    {
    }
    
    StopAnalyzer::StopAnalyzer(LuceneVersion::Version matchVersion, const String& stopwordsFile) : StopwordAnalyzerBase(matchVersion)
    {
        this->stopwords = newLucene<CharArraySet>(matchVersion, WordlistLoader::getWordSet(stopwordsFile, false));
    }
    
    StopAnalyzer::StopAnalyzer(LuceneVersion::Version matchVersion, ReaderPtr stopwords) : StopwordAnalyzerBase(matchVersion)
    {
        this->stopwords = newLucene<CharArraySet>(matchVersion, WordlistLoader::getWordSet(stopwords), false);
    }
    
    StopAnalyzer::~StopAnalyzer()
    {
    }
    
    const HashSet<String> StopAnalyzer::ENGLISH_STOP_WORDS_SET()
    {
        const wchar_t* _ENGLISH_STOP_WORDS_SET[] = 
        {
            L"a", L"an", L"and", L"are", L"as", L"at", L"be", L"but", L"by",
            L"for", L"if", L"in", L"into", L"is", L"it", L"no", L"not", L"of", 
            L"on", L"or", L"such", L"that", L"the", L"their", L"then", L"there", 
            L"these", L"they", L"this", L"to", L"was", L"will", L"with"
        };
        static HashSet<String> __ENGLISH_STOP_WORDS_SET;
        if (!__ENGLISH_STOP_WORDS_SET)
            __ENGLISH_STOP_WORDS_SET = HashSet<String>::newInstance(_ENGLISH_STOP_WORDS_SET, _ENGLISH_STOP_WORDS_SET + SIZEOF_ARRAY(_ENGLISH_STOP_WORDS_SET));
        return __ENGLISH_STOP_WORDS_SET;
    }
    
    TokenStreamComponentsPtr StopAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<LowerCaseTokenizer>(matchVersion, reader));
        return newLucene<TokenStreamComponents>(source, newLucene<StopFilter>(matchVersion, source, stopwords));
    }
}
