/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef KEYWORDANALYZER_H
#define KEYWORDANALYZER_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// Tokenizes the entire stream as a single token. This is useful for data like zip codes, ids, and some 
    /// product names.
    class LPPAPI KeywordAnalyzer : public ReusableAnalyzerBase
    {
    public:
        virtual ~KeywordAnalyzer();
        LUCENE_CLASS(KeywordAnalyzer);
    
    protected:
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
