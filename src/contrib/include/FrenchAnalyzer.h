/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHANALYZER_H
#define FRENCHANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for French language.
///
/// Supports an external list of stopwords (words that will not be indexed at all) and an external list of
/// exclusions (words that will not be stemmed, but indexed).  A default set of stopwords is used unless an
/// alternative list is specified, but the exclusion list is empty by default.
///
/// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
class LPPCONTRIBAPI FrenchAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    FrenchAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    /// Builds an analyzer with the given stop words and stemming exclusion words.
    FrenchAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

    virtual ~FrenchAnalyzer();

    LUCENE_CLASS(FrenchAnalyzer);

protected:
    /// Contains the stopwords used with the {@link StopFilter}.
    HashSet<String> stoptable;

    /// Contains words that should be indexed but not stemmed.
    HashSet<String> excltable;

    LuceneVersion::Version matchVersion;

    /// List of typical French stopwords.
    static const wchar_t* _FRENCH_STOP_WORDS[];

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    void setStemExclusionTable(HashSet<String> exclusions);

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from a {@link StandardTokenizer} filtered with
    /// {@link StandardFilter}, {@link StopFilter}, {@link FrenchStemFilter}, and {@link LowerCaseFilter}.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link StandardTokenizer} filtered with
    /// {@link StandardFilter}, {@link StopFilter}, {@link FrenchStemFilter} and {@link LowerCaseFilter}.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI FrenchAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~FrenchAnalyzerSavedStreams();

    LUCENE_CLASS(FrenchAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
