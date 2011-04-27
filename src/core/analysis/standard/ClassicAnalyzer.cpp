/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ClassicAnalyzer.h"
#include "_ClassicAnalyzer.h"
#include "ClassicTokenizer.h"
#include "ClassicFilter.h"
#include "LowerCaseFilter.h"
#include "StopAnalyzer.h"
#include "StopFilter.h"
#include "WordlistLoader.h"
#include "CharArraySet.h"

namespace Lucene
{
    /// Construct an analyzer with the given stop words.
    const int32_t ClassicAnalyzer::DEFAULT_MAX_TOKEN_LENGTH = 255;
    
    ClassicAnalyzer::ClassicAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, StopAnalyzer::ENGLISH_STOP_WORDS_SET());
    }
    
    ClassicAnalyzer::ClassicAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords) : StopwordAnalyzerBase(matchVersion, stopWords)
    {
        ConstructAnalyser(matchVersion, stopWords);
    }
    
    ClassicAnalyzer::ClassicAnalyzer(LuceneVersion::Version matchVersion, const String& stopwords) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
    }
    
    ClassicAnalyzer::ClassicAnalyzer(LuceneVersion::Version matchVersion, ReaderPtr stopwords) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
    }
    
    ClassicAnalyzer::~ClassicAnalyzer()
    {
    }
    
    void ClassicAnalyzer::ConstructAnalyser(LuceneVersion::Version matchVersion, HashSet<String> stopWords)
    {
        this->stopwords = newLucene<CharArraySet>(matchVersion, stopWords, false);
        this->replaceInvalidAcronym = LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_24);
        this->matchVersion = matchVersion;
        this->maxTokenLength = DEFAULT_MAX_TOKEN_LENGTH;
    }
    
    void ClassicAnalyzer::setMaxTokenLength(int32_t length)
    {
        maxTokenLength = length;
    }
    
    int32_t ClassicAnalyzer::getMaxTokenLength()
    {
        return maxTokenLength;
    }
    
    TokenStreamComponentsPtr ClassicAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        ClassicTokenizerPtr src(newLucene<ClassicTokenizer>(matchVersion, reader));
        src->setMaxTokenLength(maxTokenLength);
        src->setReplaceInvalidAcronym(replaceInvalidAcronym);
        TokenStreamPtr tok(newLucene<ClassicFilter>(src));
        tok = newLucene<LowerCaseFilter>(matchVersion, tok);
        tok = newLucene<StopFilter>(matchVersion, tok, stopwords);
        return newLucene<ClassicAnalyzerTokenStreamComponents>(shared_from_this(), src, tok);
    }
    
    ClassicAnalyzerTokenStreamComponents::ClassicAnalyzerTokenStreamComponents(ClassicAnalyzerPtr analyzer, TokenizerPtr source, TokenStreamPtr result) : TokenStreamComponents(source, result)
    {
        _analyzer = analyzer;
    }
    
    ClassicAnalyzerTokenStreamComponents::ClassicAnalyzerTokenStreamComponents(ClassicAnalyzerPtr analyzer, TokenizerPtr source) : TokenStreamComponents(source)
    {
        _analyzer = analyzer;
    }
    
    ClassicAnalyzerTokenStreamComponents::~ClassicAnalyzerTokenStreamComponents()
    {
    }
    
    bool ClassicAnalyzerTokenStreamComponents::reset(ReaderPtr reader)
    {
        ClassicTokenizerPtr src(boost::static_pointer_cast<ClassicTokenizer>(source));
        src->setMaxTokenLength(ClassicAnalyzerPtr(_analyzer)->maxTokenLength);
        return TokenStreamComponents::reset(reader);
    }
}
