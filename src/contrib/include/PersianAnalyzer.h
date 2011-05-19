/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSIANANALYZER_H
#define PERSIANANALYZER_H

#include "LuceneContrib.h"
#include "StopwordAnalyzerBase.h"

namespace Lucene
{
    /// {@link Analyzer} for Persian.
    ///
    /// This Analyzer uses {@link ArabicLetterTokenizer} which implies tokenizing around
    /// zero-width non-joiner in addition to whitespace.  Some persian-specific variant
    /// forms (such as farsi yeh and keheh) are standardized. "Stemming" is accomplished
    /// via stopwords.
    class LPPCONTRIBAPI PersianAnalyzer : public StopwordAnalyzerBase
    {
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

    public:
        /// Returns an unmodifiable instance of the default stop-words set.
        static const HashSet<String> getDefaultStopSet();

        /// Creates {@link TokenStreamComponents} used to tokenize all the text in the
        /// provided {@link Reader}.
        /// @return {@link TokenStreamComponents} built from a {@link StandardTokenizer}
        /// filtered with {@link LowerCaseFilter}, {@link ArabicNormalizationFilter},
        /// {@link PersianNormalizationFilter} and Persian Stop words
        virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);

    protected:
        /// Wraps the Reader with {@link PersianCharFilter}
        virtual ReaderPtr initReader(ReaderPtr reader);
    };
}

#endif

