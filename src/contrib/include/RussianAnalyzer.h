/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANANALYZER_H
#define RUSSIANANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for Russian language.
///
/// Supports an external list of stopwords (words that will not be indexed at all).
/// A default set of stopwords is used unless an alternative list is specified.
class LPPCONTRIBAPI RussianAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    RussianAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    RussianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    virtual ~RussianAnalyzer();

    LUCENE_CLASS(RussianAnalyzer);

protected:
    /// Contains the stopwords used with the {@link StopFilter}.
    HashSet<String> stopSet;

    LuceneVersion::Version matchVersion;

    /// List of typical Russian stopwords.
    static const uint8_t DEFAULT_STOPWORD_FILE[];

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from a {@link RussianLetterTokenizer} filtered with
    /// {@link RussianLowerCaseFilter}, {@link StopFilter} and {@link RussianStemFilter}.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from a {@link RussianLetterTokenizer} filtered with
    /// {@link RussianLowerCaseFilter}, {@link StopFilter} and {@link RussianStemFilter}.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI RussianAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~RussianAnalyzerSavedStreams();

    LUCENE_CLASS(RussianAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
