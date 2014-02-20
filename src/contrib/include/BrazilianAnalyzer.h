/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BRAZILIANANALYZER_H
#define BRAZILIANANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for Brazilian Portuguese language.
///
/// Supports an external list of stopwords (words that will not be indexed at all) and an external list of
/// exclusions (words that will not be stemmed, but indexed).
///
/// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
class LPPCONTRIBAPI BrazilianAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    BrazilianAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    /// Builds an analyzer with the given stop words and stemming exclusion words.
    BrazilianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

    virtual ~BrazilianAnalyzer();

    LUCENE_CLASS(BrazilianAnalyzer);

protected:
    /// Contains the stopwords used with the {@link StopFilter}.
    HashSet<String> stoptable;

    /// Contains words that should be indexed but not stemmed.
    HashSet<String> excltable;

    LuceneVersion::Version matchVersion;

    /// List of typical Brazilian Portuguese stopwords.
    static const wchar_t* _BRAZILIAN_STOP_WORDS[];

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    void setStemExclusionTable(HashSet<String> exclusions);

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from a {@link StandardTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StandardFilter}, {@link StopFilter}, and {@link BrazilianStemFilter}.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link BrazilianLetterTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StopFilter}, {@link BrazilianNormalizationFilter} and
    /// {@link BrazilianStemFilter}.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI BrazilianAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~BrazilianAnalyzerSavedStreams();

    LUCENE_CLASS(BrazilianAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
