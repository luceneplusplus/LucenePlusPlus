/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GREEKANALYZER_H
#define GREEKANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Greek language.
    ///
    /// Supports an external list of stopwords (words that will not be indexed at all).
    /// A default set of stopwords is used unless an alternative list is specified.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// GreekAnalyzer:
    /// <ul>
    ///    <li> As of 3.1, StandardFilter and GreekStemmer are used by default.
    ///    <li> As of 2.9, StopFilter preserves position increments
    /// </ul>
    ///
    /// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings
    /// as {@link StandardAnalyzer}.
    class LPPCONTRIBAPI GreekAnalyzer : public StopwordAnalyzerBase
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        GreekAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        GreekAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

        virtual ~GreekAnalyzer();

        LUCENE_CLASS(GreekAnalyzer);

    protected:
        /// Contains the stopwords used with the {@link StopFilter}.
        HashSet<String> stopSet;

        LuceneVersion::Version matchVersion;

        /// Default Greek stopwords in UTF-8 format.
        static const uint8_t _GREEK_STOP_WORDS[];

    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();

        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link StandardTokenizer}
        /// filtered with {@link GreekLowerCaseFilter}, {@link StandardFilter},
        /// {@link StopFilter}, and {@link GreekStemFilter}
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
    };
}

#endif

