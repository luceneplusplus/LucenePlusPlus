/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSIANANALYZER_H
#define PERSIANANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for Persian.
///
/// This Analyzer uses {@link ArabicLetterTokenizer} which implies tokenizing around
/// zero-width non-joiner in addition to whitespace.  Some persian-specific variant
/// forms (such as farsi yeh and keheh) are standardized. "Stemming" is accomplished
/// via stopwords.
class LPPCONTRIBAPI PersianAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    PersianAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    PersianAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    virtual ~PersianAnalyzer();

    LUCENE_CLASS(PersianAnalyzer);

public:
    /// Default Persian stopwords in UTF-8 format.
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
    /// {@link LowerCaseFilter}, {@link ArabicNormalizationFilter}, {@link PersianNormalizationFilter}
    /// and Persian Stop words.
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from an {@link ArabicLetterTokenizer} filtered with
    /// {@link LowerCaseFilter}, {@link ArabicNormalizationFilter}, {@link PersianNormalizationFilter}
    /// and Persian Stop words.
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI PersianAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~PersianAnalyzerSavedStreams();

    LUCENE_CLASS(PersianAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
