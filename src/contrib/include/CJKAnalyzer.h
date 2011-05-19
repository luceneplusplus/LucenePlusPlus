/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CJKANALYZER_H
#define CJKANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// An {@link Analyzer} that tokenizes text with {@link CJKTokenizer} and filters with {@link StopFilter}
    class LPPCONTRIBAPI CJKAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer which removes words in {@link #getDefaultStopSet()}.
        /// @param matchVersion lucene compatibility version
        CJKAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Builds an analyzer with the given stop words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        CJKAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);
        
        virtual ~CJKAnalyzer();
        
        LUCENE_CLASS(CJKAnalyzer);
    
    protected:
        /// List of typical English stopwords.
        static const wchar_t* _STOP_WORDS[];
    
    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();

    protected:
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
