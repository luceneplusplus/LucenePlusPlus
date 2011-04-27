/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WHITESPACEANALYZER_H
#define WHITESPACEANALYZER_H

#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// An Analyzer that uses {@link WhitespaceTokenizer}.
    ///
    /// You must specify the required {@link Version} compatibility when creating 
    /// {@link CharTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link WhitespaceTokenizer} uses an int based API to normalize and
    ///    detect token codepoints. See {@link CharTokenizer#isTokenChar(int)} and {@link 
    ///    CharTokenizer#normalize(int)} for details.
    /// </ul>
    class LPPAPI WhitespaceAnalyzer : public ReusableAnalyzerBase
    {
    public:
        /// Creates a new {@link WhitespaceAnalyzer}
        /// @param matchVersion Lucene version to match
        WhitespaceAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Creates a new {@link WhitespaceAnalyzer}
        /// @deprecated use {@link #WhitespaceAnalyzer(Version)} instead 
        WhitespaceAnalyzer();
        
        virtual ~WhitespaceAnalyzer();
        
        LUCENE_CLASS(WhitespaceAnalyzer);
    
    private:
        LuceneVersion::Version matchVersion;
    
    public:
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
