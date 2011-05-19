/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ChineseAnalyzer.h"
#include "ChineseTokenizer.h"
#include "ChineseFilter.h"

namespace Lucene
{
    ChineseAnalyzer::~ChineseAnalyzer()
    {
    }
    
    TokenStreamComponentsPtr ChineseAnalyzer::createComponents(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr source(newLucene<ChineseTokenizer>(reader));
        return newLucene<TokenStreamComponents>(source, newLucene<ChineseFilter>(source));
    }
}
