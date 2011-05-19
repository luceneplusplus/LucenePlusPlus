/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BRAZILIANANALYZER_H
#define BRAZILIANANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Brazilian Portuguese language. 
    ///
    /// Supports an external list of stopwords (words that will not be indexed at all) and an external list of 
    /// exclusions (words that will not be stemmed, but indexed).
    ///
    /// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
    class LPPCONTRIBAPI BrazilianAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        /// @param matchVersion lucene compatibility version
        BrazilianAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Builds an analyzer with the given stop words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);
        
        /// Builds an analyzer with the given stop words and stemming exclusion words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        /// @param exclusions a stemming exclusion set
        BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);
        
        virtual ~BrazilianAnalyzer();
        
        LUCENE_CLASS(BrazilianAnalyzer);
    
    protected:
        /// Contains words that should be indexed but not stemmed.
        HashSet<String> excltable;
        
        /// List of typical Brazilian Portuguese stopwords.
        static const wchar_t* _DEFAULT_STOP_WORDS[];
    
    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();
        
        /// Builds an exclusion list from a set of Strings.
        /// @deprecated use {@link #BrazilianAnalyzer(Version, Set, Set)} instead
        void setStemExclusionTable(HashSet<String> exclusions);
        
    protected:
        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the 
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link StandardTokenizer} 
        /// filtered with {@link LowerCaseFilter}, {@link StandardFilter}, {@link 
        /// StopFilter}, and {@link BrazilianStemFilter}.
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
