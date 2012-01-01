/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DUTCHANALYZER_H
#define DUTCHANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene
{
    /// {@link Analyzer} for Dutch language.
    ///
    /// Supports an external list of stopwords (words that will not be indexed at all) and an external list of
    /// exclusions (words that will not be stemmed, but indexed).  A default set of stopwords is used unless an
    /// alternative list is specified, but the exclusion list is empty by default.
    ///
    /// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
    class LPPCONTRIBAPI DutchAnalyzer : public Analyzer
    {
    public:
        /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
        DutchAnalyzer(LuceneVersion::Version matchVersion);

        /// Builds an analyzer with the given stop words.
        DutchAnalyzer(LuceneVersion::Version matchVersion, SetString stopwords);

        /// Builds an analyzer with the given stop words and stemming exclusion words.
        DutchAnalyzer(LuceneVersion::Version matchVersion, SetString stopwords, SetString exclusions);

        virtual ~DutchAnalyzer();

        LUCENE_CLASS(DutchAnalyzer);

    protected:
        /// Contains the stopwords used with the {@link StopFilter}.
        SetString stoptable;

        /// Contains words that should be indexed but not stemmed.
        SetString excltable;

        MapStringString stemdict;

        LuceneVersion::Version matchVersion;

        /// List of typical Dutch stopwords.
        static const wchar_t* _DUTCH_STOP_WORDS[];

    public:
        virtual void initialize();

        /// Returns an unmodifiable instance of the default stop-words set.
        static const SetString getDefaultStopSet();

        void setStemExclusionTable(SetString exclusions);

        /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
        ///
        /// @return A {@link TokenStream} built from a {@link StandardTokenizer} filtered with
        /// {@link StandardFilter}, {@link StopFilter} and {@link DutchStemFilter}.
        virtual TokenStreamPtr tokenStream(const String& fieldName, ReaderPtr reader);

        /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
        /// provided {@link Reader}.
        ///
        /// @return A {@link TokenStream} built from a {@link StandardTokenizer} filtered with
        /// {@link StandardFilter}, {@link StopFilter} and {@link DutchStemFilter}.
        virtual TokenStreamPtr reusableTokenStream(const String& fieldName, ReaderPtr reader);
    };

    class LPPCONTRIBAPI DutchAnalyzerSavedStreams : public LuceneObject
    {
    public:
        virtual ~DutchAnalyzerSavedStreams();

        LUCENE_CLASS(DutchAnalyzerSavedStreams);

    public:
        TokenizerPtr source;
        TokenStreamPtr result;
    };
}

#endif
