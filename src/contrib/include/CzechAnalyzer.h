/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CZECHANALYZER_H
#define CZECHANALYZER_H

#include "LuceneContrib.h"
#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// Supports an external list of stopwords (words that will not be indexed at
    /// all). A default set of stopwords is used unless an alternative list is
    /// specified.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// CzechAnalyzer:
    /// <ul>
    ///    <li>As of 3.1, words are stemmed with {@link CzechStemFilter}
    ///    <li>As of 2.9, StopFilter preserves position increments
    ///    <li>As of 2.4, Tokens incorrectly identified as acronyms are corrected 
    /// </ul>
    class LPPCONTRIBAPI CzechAnalyzer : public ReusableAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        /// @param matchVersion Lucene version to match
        CzechAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Builds an analyzer with the given stop words.
        /// @param matchVersion Lucene version to match
        /// @param stopwords a stopword set
        CzechAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);
        
        /// Builds an analyzer with the given stop words.
        /// @param matchVersion Lucene version to match
        /// @param stopwords a stopword set
        /// @param stemExclusionTable a stemming exclusion set
        CzechAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> stemExclusionTable);
        
        virtual ~CzechAnalyzer();
        
        LUCENE_CLASS(CzechAnalyzer);
    
    private:
        /// Contains the stopwords used with the {@link StopFilter}.
        HashSet<String> stoptable;
        
        HashSet<String> stemExclusionTable;
        
        LuceneVersion::Version matchVersion;
        
        /// Default Czech stopwords in UTF-8 format.
        static const uint8_t _CZECH_STOP_WORDS[];
        
    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();
    
    protected:
        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the 
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link StandardTokenizer} 
        /// filtered with {@link StandardFilter}, {@link LowerCaseFilter}, {@link StopFilter}
        /// and {@link CzechStemFilter} (only if version is >= LUCENE_31). If a version 
        /// is >= LUCENE_31 and a stem exclusion set is provided via {@link 
        /// #CzechAnalyzer(Version, Set, Set)} a {@link KeywordMarkerFilter} is added before
        /// {@link CzechStemFilter}.
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
