/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SimpleAnalyzer.h"
#include "LowerCaseTokenizer.h"

namespace Lucene
{
    SimpleAnalyzer::SimpleAnalyzer(LuceneVersion::Version matchVersion)
    {
        this->matchVersion = matchVersion;
    }
    
    SimpleAnalyzer::SimpleAnalyzer()
    {
        this->matchVersion = LuceneVersion::LUCENE_30;
    }
    
    SimpleAnalyzer::~SimpleAnalyzer()
    {
    }
    
    TokenStreamComponentsPtr SimpleAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<TokenStreamComponents>(newLucene<LowerCaseTokenizer>(matchVersion, reader));
    }
}
