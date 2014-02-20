/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICANALYZER_H
#define ARABICANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for Arabic.
///
/// This analyzer implements light-stemming as specified by:
/// Light Stemming for Arabic Information Retrieval
///
/// http://www.mtholyoke.edu/~lballest/Pubs/arab_stem05.pdf
///
/// The analysis package contains three primary components:
/// <ul>
/// <li> {@link ArabicNormalizationFilter}: Arabic orthographic normalization.
/// <li> {@link ArabicStemFilter}: Arabic light stemming.
/// <li> Arabic stop words file: a set of default Arabic stop words.
/// </ul>
class LPPCONTRIBAPI ArabicAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    ArabicAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    ArabicAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    virtual ~ArabicAnalyzer();

    LUCENE_CLASS(ArabicAnalyzer);

public:
    /// Default Arabic stopwords in UTF-8 format.
    ///
    /// Generated from http://members.unine.ch/jacques.savoy/clef/index.html
    /// The stopword list is BSD-Licensed.
    static const uint8_t DEFAULT_STOPWORD_FILE[];

protected:
    /// Contains the stopwords used with the StopFilter.
    HashSet<String> stoptable;

    LuceneVersion::Version matchVersion;

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link ArabicLetterTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StopFilter}, {@link ArabicNormalizationFilter} and
    /// {@link ArabicStemFilter}.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link ArabicLetterTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link StopFilter}, {@link ArabicNormalizationFilter} and
    /// {@link ArabicStemFilter}.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI ArabicAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~ArabicAnalyzerSavedStreams();

    LUCENE_CLASS(ArabicAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
