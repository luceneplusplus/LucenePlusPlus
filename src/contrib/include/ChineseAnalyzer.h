/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHINESEANALYZER_H
#define CHINESEANALYZER_H

#include "LuceneContrib.h"
#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// An {@link Analyzer} that tokenizes text with {@link ChineseTokenizer} and 
    /// filters with {@link ChineseFilter}
    ///
    /// @deprecated Use {@link StandardAnalyzer} instead, which has the same 
    /// functionality.
    class LPPCONTRIBAPI ChineseAnalyzer : public ReusableAnalyzerBase
    {
    public:
        virtual ~ChineseAnalyzer();
        
        LUCENE_CLASS(ChineseAnalyzer);
    
    protected:
        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the 
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link ChineseTokenizer} 
        /// filtered with {@link ChineseFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
