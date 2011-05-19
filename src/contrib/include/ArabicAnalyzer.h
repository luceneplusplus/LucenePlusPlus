/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICANALYZER_H
#define ARABICANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Arabic. 
    ///
    /// This analyzer implements light-stemming as specified by:
    /// Light Stemming for Arabic Information Retrieval
    ///
    /// http://www.mtholyoke.edu/~lballest/Pubs/arab_stem05.pdf
    ///
    /// The analysis package contains three primary components:
    /// <ul>
    /// <li> {@link ArabicNormalizationFilter}: Arabic orthographic normalization.
    /// <li> {@link ArabicStemFilter}: Arabic light stemming.
    /// <li> Arabic stop words file: a set of default Arabic stop words.
    /// </ul>
    class LPPCONTRIBAPI ArabicAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        ArabicAnalyzer(LuceneVersion::Version matchVersion);
        
        /// Builds an analyzer with the given stop words.
        ArabicAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);
        
        /// Builds an analyzer with the given stop words. If a none-empty stem exclusion set is
        /// provided this analyzer will add a {@link KeywordMarkerFilter} before {@link 
        /// ArabicStemFilter}.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        /// @param stemExclusionSet a set of terms not to be stemmed
        ArabicAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> stemExclusionSet);
        
        virtual ~ArabicAnalyzer();
        
        LUCENE_CLASS(ArabicAnalyzer);
    
    public:
        /// Default Arabic stopwords in UTF-8 format.
        ///
        /// Generated from http://members.unine.ch/jacques.savoy/clef/index.html
        /// The stopword list is BSD-Licensed.
        static const uint8_t DEFAULT_STOPWORD_FILE[];
    
    private:
        HashSet<String> stemExclusionSet;
    
    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();
    
    protected:
        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from an {@link StandardTokenizer} filtered with {@link 
        /// LowerCaseFilter}, {@link StopFilter}, {@link ArabicNormalizationFilter}, {@link KeywordMarkerFilter}
        /// if a stem exclusion set is provided and {@link ArabicStemFilter}.
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif
