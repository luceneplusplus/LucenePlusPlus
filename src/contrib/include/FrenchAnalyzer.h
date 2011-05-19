/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHANALYZER_H
#define FRENCHANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for French language.
    ///
    /// Supports an external list of stopwords (words that will not be indexed at
    /// all) and an external list of exclusions (words that will not be stemmed,
    /// but indexed).  A default set of stopwords is used unless an alternative
    /// list is specified, but the exclusion list is empty by default.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// FrenchAnalyzer:
    /// <ul>
    ///    <li> As of 3.1, Snowball stemming is done with SnowballFilter,
    ///         LowerCaseFilter is used prior to StopFilter, and ElisionFilter
    ///         and Snowball stopwords are used by default.
    ///    <li> As of 2.9, StopFilter preserves position increments
    /// </ul>
    ///
    /// NOTE: This class uses the same {@link LuceneVersion#Version} dependent
    /// settings as {@link StandardAnalyzer}.
    class LPPCONTRIBAPI FrenchAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        /// @param matchVersion lucene compatibility version
        FrenchAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

        /// Builds an analyzer with the given stop words and stemming exclusion words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        /// @param exclusions a stemming exclusion set
        FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

        virtual ~FrenchAnalyzer();

        LUCENE_CLASS(FrenchAnalyzer);

    protected:
        /// Contains words that should be indexed but not stemmed.
        HashSet<String> excltable;

        /// List of typical French stopwords (Lucene 3.0 and below).
        static const wchar_t* _FRENCH_STOP_WORDS_30[];

        /// List of typical French stopwords (Lucene 3.1 and above).
        static const wchar_t* _FRENCH_STOP_WORDS_DEFAULT[];

    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        /// Lucene 3.0 and below.
        static const HashSet<String> getDefaultStopSet30();

        /// Returns an unmodifiable instance of the default stop-words set.
        /// Lucene 3.1 and above.
        static const HashSet<String> getDefaultStopSet();

        /// Builds an exclusionlist from an set of Strings.
        ///  @deprecated use {@link #FrenchAnalyzer(Version, Set, Set)} instead
        void setStemExclusionTable(HashSet<String> exclusions);

        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link StandardTokenizer}
        /// filtered with {@link StandardFilter}, {@link ElisionFilter}, {@link
        /// LowerCaseFilter}, {@link StopFilter}, {@link KeywordMarkerFilter} if a stem
        /// exclusion set is provided, and {@link SnowballFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif

