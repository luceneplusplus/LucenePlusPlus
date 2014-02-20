/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GERMANANALYZER_H
#define GERMANANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for German language.
///
/// Supports an external list of stopwords (words that will not be indexed at all) and an external list of
/// exclusions (words that will not be stemmed, but indexed).  A default set of stopwords is used unless an
/// alternative list is specified, but the exclusion list is empty by default.
///
/// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
class LPPCONTRIBAPI GermanAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    GermanAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    /// Builds an analyzer with the given stop words and stemming exclusion words.
    GermanAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords, HashSet<String> exclusions);

    virtual ~GermanAnalyzer();

    LUCENE_CLASS(GermanAnalyzer);

protected:
    /// Contains the stopwords used with the {@link StopFilter}.
    HashSet<String> stopSet;

    /// Contains words that should be indexed but not stemmed.
    HashSet<String> exclusionSet;

    LuceneVersion::Version matchVersion;

    /// List of typical German stopwords.
    static const wchar_t* _GERMAN_STOP_WORDS[];

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    void setStemExclusionTable(HashSet<String> exclusions);

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from a {@link StandardTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StandardFilter}, {@link StopFilter}, and {@link GermanStemFilter}.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link GermanLetterTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StopFilter}, {@link GermanNormalizationFilter} and
    /// {@link GermanStemFilter}.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI GermanAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~GermanAnalyzerSavedStreams();

    LUCENE_CLASS(GermanAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
