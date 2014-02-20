/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CZECHANALYZER_H
#define CZECHANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// {@link Analyzer} for Czech language.
///
/// Supports an external list of stopwords (words that will not be indexed at all).
/// A default set of stopwords is used unless an alternative list is specified.
///
/// NOTE: This class uses the same {@link LuceneVersion#Version} dependent settings as {@link StandardAnalyzer}.
class LPPCONTRIBAPI CzechAnalyzer : public Analyzer {
public:
    /// Builds an analyzer with the default stop words: {@link #getDefaultStopSet}.
    CzechAnalyzer(LuceneVersion::Version matchVersion);

    /// Builds an analyzer with the given stop words.
    CzechAnalyzer(LuceneVersion::Version matchVersion, HashSet<String> stopwords);

    virtual ~CzechAnalyzer();

    LUCENE_CLASS(CzechAnalyzer);

protected:
    /// Contains the stopwords used with the {@link StopFilter}.
    HashSet<String> stoptable;

    LuceneVersion::Version matchVersion;

    /// Default Czech stopwords in UTF-8 format.
    static const uint8_t _CZECH_STOP_WORDS[];

public:
    /// Returns an unmodifiable instance of the default stop-words set.
    static const HashSet<String> getDefaultStopSet();

    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from {@link StandardTokenizer}, filtered with {@link StandardFilter},
    /// {@link LowerCaseFilter}, and {@link StopFilter}
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from {@link StandardTokenizer}, filtered with {@link StandardFilter},
    /// {@link LowerCaseFilter}, and {@link StopFilter}
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI CzechAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~CzechAnalyzerSavedStreams();

    LUCENE_CLASS(CzechAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
