/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TURKISHANALYZER_H
#define TURKISHANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Turkish language.
    class LPPCONTRIBAPI TurkishAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        /// @param matchVersion lucene compatibility version
        TurkishAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        TurkishAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

        /// Builds an analyzer with the given stop words and stemming exclusion words.
        /// @param matchVersion lucene compatibility version
        /// @param stopwords a stopword set
        /// @param exclusions a stemming exclusion set
        TurkishAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

        virtual ~TurkishAnalyzer();

        LUCENE_CLASS(TurkishAnalyzer);

    protected:
        /// Contains the stopwords used with the {@link StopFilter}.
        HashSet<String> stopSet;

        /// Contains words that should be indexed but not stemmed.
        HashSet<String> exclusionSet;

        LuceneVersion::Version matchVersion;

        /// List of typical Turkish stopwords.
        static const wchar_t* _TURKISH_STOP_WORDS_DEFAULT[];

    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();

        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the
        /// provided {@link Reader}.
        /// @return A {@link TokenStreamComponents} built from an {@link StandardTokenizer}
        /// filtered with {@link StandardFilter}, {@link TurkishLowerCaseFilter}, {@link
        /// StopFilter}, {@link KeywordMarkerFilter} if a stem exclusion set is provided
        /// and {@link SnowballFilter}.
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif

