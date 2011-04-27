/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StandardAnalyzer.h"
#include "_StandardAnalyzer.h"
#include "StandardTokenizer.h"
#include "StandardFilter.h"
#include "LowerCaseFilter.h"
#include "StopAnalyzer.h"
#include "StopFilter.h"
#include "WordlistLoader.h"
#include "CharArraySet.h"

namespace Lucene
{
    /// Construct an analyzer with the given stop words.
    const int32_t StandardAnalyzer::DEFAULT_MAX_TOKEN_LENGTH = 255;
    
    StandardAnalyzer::StandardAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords) : StopwordAnalyzerBase(matchVersion, stopWords)
    {
        ConstructAnalyser(matchVersion, stopWords);
    }
    
    StandardAnalyzer::StandardAnalyzer(LuceneVersion::Version matchVersion) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, StopAnalyzer::ENGLISH_STOP_WORDS_SET());
    }
    
    StandardAnalyzer::StandardAnalyzer(LuceneVersion::Version matchVersion, const String& stopwords) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
    }
    
    StandardAnalyzer::StandardAnalyzer(LuceneVersion::Version matchVersion, ReaderPtr stopwords) : StopwordAnalyzerBase(matchVersion)
    {
        ConstructAnalyser(matchVersion, WordlistLoader::getWordSet(stopwords));
    }
    
    StandardAnalyzer::~StandardAnalyzer()
    {
    }
    
    void StandardAnalyzer::ConstructAnalyser(LuceneVersion::Version matchVersion, HashSet<String> stopWords)
    {
        this->stopwords = newLucene<CharArraySet>(matchVersion, stopWords, false);
        this->replaceInvalidAcronym = LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_24);
        this->matchVersion = matchVersion;
        this->maxTokenLength = DEFAULT_MAX_TOKEN_LENGTH;
    }
    
    void StandardAnalyzer::setMaxTokenLength(int32_t length)
    {
        maxTokenLength = length;
    }
    
    int32_t StandardAnalyzer::getMaxTokenLength()
    {
        return maxTokenLength;
    }
    
    TokenStreamComponentsPtr StandardAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        StandardTokenizerPtr src(newLucene<StandardTokenizer>(matchVersion, reader));
        src->setMaxTokenLength(maxTokenLength);
        src->setReplaceInvalidAcronym(replaceInvalidAcronym);
        TokenStreamPtr tok(newLucene<StandardFilter>(matchVersion, src));
        tok = newLucene<LowerCaseFilter>(matchVersion, tok);
        tok = newLucene<StopFilter>(matchVersion, tok, stopwords);
        return newLucene<StandardAnalyzerTokenStreamComponents>(shared_from_this(), src, tok);
    }
    
    StandardAnalyzerTokenStreamComponents::StandardAnalyzerTokenStreamComponents(StandardAnalyzerPtr analyzer, TokenizerPtr source, TokenStreamPtr result) : TokenStreamComponents(source, result)
    {
        _analyzer = analyzer;
    }
    
    StandardAnalyzerTokenStreamComponents::StandardAnalyzerTokenStreamComponents(StandardAnalyzerPtr analyzer, TokenizerPtr source) : TokenStreamComponents(source)
    {
        _analyzer = analyzer;
    }
    
    StandardAnalyzerTokenStreamComponents::~StandardAnalyzerTokenStreamComponents()
    {
    }
    
    bool StandardAnalyzerTokenStreamComponents::reset(ReaderPtr reader)
    {
        StandardTokenizerPtr src(boost::static_pointer_cast<StandardTokenizer>(source));
        src->setMaxTokenLength(StandardAnalyzerPtr(_analyzer)->maxTokenLength);
        return TokenStreamComponents::reset(reader);
    }
}
