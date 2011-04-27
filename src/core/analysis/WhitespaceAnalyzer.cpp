/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "WhitespaceAnalyzer.h"
#include "WhitespaceTokenizer.h"

namespace Lucene
{
    WhitespaceAnalyzer::WhitespaceAnalyzer(LuceneVersion::Version matchVersion)
    {
        this->matchVersion = matchVersion;
    }
    
    WhitespaceAnalyzer::WhitespaceAnalyzer()
    {
        this->matchVersion = LuceneVersion::LUCENE_30;
    }
    
    WhitespaceAnalyzer::~WhitespaceAnalyzer()
    {
    }
    
    TokenStreamComponentsPtr WhitespaceAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<TokenStreamComponents>(newLucene<WhitespaceTokenizer>(matchVersion, reader));
    }
}
