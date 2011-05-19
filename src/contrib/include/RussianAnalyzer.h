/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANANALYZER_H
#define RUSSIANANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Russian language.
    ///
    /// Supports an external list of stopwords (words that will not be indexed at all).
    /// A default set of stopwords is used unless an alternative list is specified.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// RussianAnalyzer:
    /// <ul>
    ///    <li> As of 3.1, StandardTokenizer is used, Snowball stemming is done with
    ///         SnowballFilter, and Snowball stopwords are used by default.
    /// </ul>
    class LPPCONTRIBAPI RussianAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        /// @param matchVersion lucene compatibility version
        RussianAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        RussianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

        /// Builds an analyzer with the given stop words and stemming exclusion words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        /// @param exclusions a stemming exclusion set
        RussianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

        virtual ~RussianAnalyzer();

        LUCENE_CLASS(RussianAnalyzer);

    protected:
        /// Contains the stopwords used with the {@link StopFilter}.
        HashSet<String> stopSet;

        /// Contains words that should be indexed but not stemmed.
        HashSet<String> exclusionSet;

        LuceneVersion::Version matchVersion;

        /// List of typical Russian stopwords (Lucene 3.0 and below).
        static const wchar_t* _RUSSIAN_STOP_WORDS_30[];

        /// List of typical Russian stopwords (Lucene 3.1).
        static const wchar_t* _RUSSIAN_STOP_WORDS_DEFAULT[];

    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        /// Lucene 3.0 and below.
        static const HashSet<String> getDefaultStopSet30();

        /// Returns an unmodifiable instance of the default stop-words set.
        /// Lucene 3.1 and above.
        static const HashSet<String> getDefaultStopSet();

        /// Creates {@link TokenStreamComponents} used to tokenize all the text in
        /// the provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link
        /// StandardTokenizer} filtered with {@link StandardFilter}, {@link
        /// LowerCaseFilter}, {@link StopFilter}, {@link KeywordMarkerFilter} if
        /// a stem exclusion set is provided, and {@link SnowballFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif

