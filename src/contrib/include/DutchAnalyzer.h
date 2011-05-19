/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DUTCHANALYZER_H
#define DUTCHANALYZER_H

#include "LuceneContrib.h"
#include "ReusableAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Dutch language.
    ///
    /// Supports an external list of stopwords (words that will not be indexed at
    /// all) and an external list of exclusions (words that will not be stemmed,
    /// but indexed).  A default set of stopwords is used unless an alternative
    /// list is specified, but the exclusion list is empty by default.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// DutchAnalyzer:
    /// <ul>
    ///    <li> As of 3.1, Snowball stemming is done with SnowballFilter,
    ///         LowerCaseFilter is used prior to StopFilter, and Snowball
    ///         stopwords are used by default.
    ///    <li> As of 2.9, StopFilter preserves position increments
    /// </ul>
    ///
    /// NOTE: This class uses the same {@link LuceneVersion#Version} dependent
    /// settings as {@link StandardAnalyzer}.
    class LPPCONTRIBAPI DutchAnalyzer : public ReusableAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        DutchAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        DutchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

        /// Builds an analyzer with the given stop words and stemming exclusion words.
        DutchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

        virtual ~DutchAnalyzer();

        LUCENE_CLASS(DutchAnalyzer);

    protected:
        /// Contains the stopwords used with the {@link StopFilter}.
        HashSet<String> stoptable;

        /// Contains words that should be indexed but not stemmed.
        HashSet<String> excltable;

        MapStringString stemdict;

        LuceneVersion::Version matchVersion;

        /// List of typical Dutch stopwords.
        static const wchar_t* _DUTCH_STOP_WORDS[];

    public:
        virtual void initialize();

        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();

        void setStemExclusionTable(HashSet<String> exclusions);

        /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
        /// provided {@link Reader}.
        ///
        /// @return A {@link TokenStream} built from a {@link StandardTokenizer}
        /// filtered with {@link StandardFilter}, {@link LowerCaseFilter}, {@link StopFilter},
        /// {@link KeywordMarkerFilter} if a stem exclusion set is provided, {@link
        /// StemmerOverrideFilter}, and {@link SnowballFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif

