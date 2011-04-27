/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SIMPLEANALYZER_H
#define SIMPLEANALYZER_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// An {@link Analyzer} that filters {@link LetterTokenizer} with {@link LowerCaseFilter} 
    ///
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// CharTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link LowerCaseTokenizer} uses an int based API to normalize and
    ///    detect token codepoints. See {@link CharTokenizer#isTokenChar(int)} and {@link 
    ///    CharTokenizer#normalize(wchar_t)} for details.
    /// </ul>
    class LPPAPI SimpleAnalyzer : public ReusableAnalyzerBase
    {
    public:
        /// Creates a new {@link SimpleAnalyzer}
        /// @param matchVersion Lucene version to match.
        SimpleAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Creates a new {@link SimpleAnalyzer}
        /// @deprecated use {@link #SimpleAnalyzer(Version)} instead 
        SimpleAnalyzer();
        
        virtual ~SimpleAnalyzer();
        
        LUCENE_CLASS(SimpleAnalyzer);
    
    protected:
        LuceneVersion::Version matchVersion;
    
    protected:
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
