/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StopFilter.h"
#include "CharArraySet.h"
#include "CharTermAttribute.h"
#include "PositionIncrementAttribute.h"

namespace Lucene
{
    StopFilter::StopFilter(bool enablePositionIncrements, TokenStreamPtr input, HashSet<String> stopWords, bool ignoreCase) : FilteringTokenFilter(enablePositionIncrements, input)
    {
        ConstructStopFilter(LuceneVersion::LUCENE_30, enablePositionIncrements, input, newLucene<CharArraySet>(LuceneVersion::LUCENE_30, stopWords, ignoreCase));
    }
    
    StopFilter::StopFilter(bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords) : FilteringTokenFilter(enablePositionIncrements, input)
    {
        ConstructStopFilter(LuceneVersion::LUCENE_30, enablePositionIncrements, input, stopWords);
    }
    
    StopFilter::StopFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, HashSet<String> stopWords, bool ignoreCase) : FilteringTokenFilter(LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_29), input)
    {
        ConstructStopFilter(matchVersion, LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_29), input, newLucene<CharArraySet>(matchVersion, stopWords, ignoreCase));
    }
    
    StopFilter::StopFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, CharArraySetPtr stopWords) : FilteringTokenFilter(LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_29), input)
    {
        ConstructStopFilter(matchVersion, LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_29), input, stopWords);
    }
    
    StopFilter::StopFilter(LuceneVersion::Version matchVersion, bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords) : FilteringTokenFilter(enablePositionIncrements, input)
    {
        ConstructStopFilter(matchVersion, enablePositionIncrements, input, stopWords);
    }
    
    StopFilter::~StopFilter()
    {
    }
    
    void StopFilter::ConstructStopFilter(LuceneVersion::Version matchVersion, bool enablePositionIncrements, TokenStreamPtr input, CharArraySetPtr stopWords)
    {
        this->stopWords = stopWords;
        termAtt = addAttribute<CharTermAttribute>();
    }
    
    CharArraySetPtr StopFilter::makeStopSet(LuceneVersion::Version matchVersion, Collection<String> stopWords, bool ignoreCase)
    {
        return newLucene<CharArraySet>(matchVersion, stopWords, ignoreCase);
    }
    
    bool StopFilter::accept()
    {
        return !stopWords->contains(termAtt->bufferArray(), 0, termAtt->length());
    }
    
    bool StopFilter::getEnablePositionIncrementsVersionDefault(LuceneVersion::Version matchVersion)
    {
        return LuceneVersion::onOrAfter(matchVersion, LuceneVersion::LUCENE_29);
    }
}
