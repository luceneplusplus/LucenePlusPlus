/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STOPANALYZER_H
#define STOPANALYZER_H

#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// Filters {@link LetterTokenizer} with {@link LowerCaseFilter} and {@link StopFilter}.
    ///
    /// You must specify the required {@link Version} compatibility when creating StopAnalyzer:
    /// <ul>
    ///    <li> As of 3.1, StopFilter correctly handles Unicode 4.0 supplementary characters 
    ///    in stopwords
    ///    <li> As of 2.9, position increments are preserved
    /// </ul>
    class LPPAPI StopAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer which removes words in {@link #ENGLISH_STOP_WORDS_SET}.
        StopAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Builds an analyzer with the stop words from the given set.
        StopAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopWords);
        
        /// Builds an analyzer with the stop words from the given file.
        StopAnalyzer(LuceneVersion::Version matchVersion, const String& stopwordsFile);
        
        /// Builds an analyzer with the stop words from the given reader.
        StopAnalyzer(LuceneVersion::Version matchVersion, ReaderPtr stopwords);
        
        virtual ~StopAnalyzer();
        
        LUCENE_CLASS(StopAnalyzer);
    
    public:
        /// An unmodifiable set containing some common English words that are usually not 
        /// useful for searching.
        static const HashSet<String> ENGLISH_STOP_WORDS_SET();
    
    protected:
        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the 
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link LowerCaseTokenizer} 
        /// filtered with {@link StopFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
