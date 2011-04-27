/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "StopwordAnalyzerBase.h"
#include "WordlistLoader.h"
#include "CharArraySet.h"

namespace Lucene
{
    StopwordAnalyzerBase::StopwordAnalyzerBase(LuceneVersion::Version version, HashSet<String> stopwords)
    {
        matchVersion = version;
        this->stopwords = newLucene<CharArraySet>(version, stopwords, false);
    }
    
    StopwordAnalyzerBase::StopwordAnalyzerBase(LuceneVersion::Version version)
    {
        matchVersion = version;
        this->stopwords = newLucene<CharArraySet>(version, HashSet<String>(), false);
    }
    
    StopwordAnalyzerBase::~StopwordAnalyzerBase()
    {
    }
    
    CharArraySetPtr StopwordAnalyzerBase::getStopwordSet()
    {
        return stopwords;
    }
    
    CharArraySetPtr StopwordAnalyzerBase::loadStopwordSet(bool ignoreCase, const String& resource, const String& comment)
    {
        return newLucene<CharArraySet>(LuceneVersion::LUCENE_31, WordlistLoader::getWordSet(resource, comment), ignoreCase);
    }
}
