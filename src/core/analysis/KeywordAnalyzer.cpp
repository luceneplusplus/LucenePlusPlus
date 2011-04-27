/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "KeywordAnalyzer.h"
#include "KeywordTokenizer.h"

namespace Lucene
{
    KeywordAnalyzer::~KeywordAnalyzer()
    {
    }
    
    TokenStreamComponentsPtr KeywordAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<TokenStreamComponents>(newLucene<KeywordTokenizer>(reader));
    }
}
