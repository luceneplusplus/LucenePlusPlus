/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENECONTRIB_H
#define LUCENECONTRIB_H

#include "Lucene.h"

namespace Lucene
{
    // analyzers
    DECLARE_LUCENE_PTR(ArabicAnalyzer)
    DECLARE_LUCENE_PTR(ArabicAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(ArabicLetterTokenizer)
    DECLARE_LUCENE_PTR(ArabicNormalizationFilter)
    DECLARE_LUCENE_PTR(ArabicNormalizer)
    DECLARE_LUCENE_PTR(ArabicStemFilter)
    DECLARE_LUCENE_PTR(ArabicStemmer)
    DECLARE_LUCENE_PTR(BrazilianAnalyzer)
    DECLARE_LUCENE_PTR(BrazilianAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(BrazilianStemFilter)
    DECLARE_LUCENE_PTR(BrazilianStemmer)
    DECLARE_LUCENE_PTR(CJKAnalyzer)
    DECLARE_LUCENE_PTR(CJKAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(CJKTokenizer)
    DECLARE_LUCENE_PTR(ChineseAnalyzer)
    DECLARE_LUCENE_PTR(ChineseAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(ChineseFilter)
    DECLARE_LUCENE_PTR(ChineseTokenizer)
    DECLARE_LUCENE_PTR(CzechAnalyzer)
    DECLARE_LUCENE_PTR(CzechAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(DutchAnalyzer)
    DECLARE_LUCENE_PTR(DutchAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(DutchStemFilter)
    DECLARE_LUCENE_PTR(DutchStemmer)
    DECLARE_LUCENE_PTR(ElisionFilter)
    DECLARE_LUCENE_PTR(FrenchAnalyzer)
    DECLARE_LUCENE_PTR(FrenchAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(FrenchStemFilter)
    DECLARE_LUCENE_PTR(FrenchStemmer)
    DECLARE_LUCENE_PTR(GermanAnalyzer)
    DECLARE_LUCENE_PTR(GermanAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(GermanStemFilter)
    DECLARE_LUCENE_PTR(GermanStemmer)
    DECLARE_LUCENE_PTR(GreekLowerCaseFilter)
    DECLARE_LUCENE_PTR(GreekAnalyzer)
    DECLARE_LUCENE_PTR(GreekAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(PersianAnalyzer)
    DECLARE_LUCENE_PTR(PersianAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(PersianNormalizationFilter)
    DECLARE_LUCENE_PTR(PersianNormalizer)
    DECLARE_LUCENE_PTR(ReverseStringFilter)
    DECLARE_LUCENE_PTR(RussianAnalyzer)
    DECLARE_LUCENE_PTR(RussianAnalyzerSavedStreams)
    DECLARE_LUCENE_PTR(RussianLetterTokenizer)
    DECLARE_LUCENE_PTR(RussianLowerCaseFilter)
    DECLARE_LUCENE_PTR(RussianStemFilter)
    DECLARE_LUCENE_PTR(RussianStemmer)
    DECLARE_LUCENE_PTR(SnowballFilter)
    DECLARE_LUCENE_PTR(SnowballAnalyzer)
    DECLARE_LUCENE_PTR(SnowballAnalyzerSavedStreams)

    // highlighter
    DECLARE_LUCENE_PTR(DefaultEncoder)
    DECLARE_LUCENE_PTR(Encoder)
    DECLARE_LUCENE_PTR(FakeReader)
    DECLARE_LUCENE_PTR(Formatter)
    DECLARE_LUCENE_PTR(Fragmenter)
    DECLARE_LUCENE_PTR(FragmentQueue)
    DECLARE_LUCENE_PTR(GradientFormatter)
    DECLARE_LUCENE_PTR(Highlighter)
    DECLARE_LUCENE_PTR(HighlighterScorer)
    DECLARE_LUCENE_PTR(MapWeightedSpanTerm)
    DECLARE_LUCENE_PTR(NullFragmenter)
    DECLARE_LUCENE_PTR(PositionCheckingMap)
    DECLARE_LUCENE_PTR(PositionSpan)
    DECLARE_LUCENE_PTR(QueryScorer)
    DECLARE_LUCENE_PTR(QueryTermExtractor)
    DECLARE_LUCENE_PTR(QueryTermScorer)
    DECLARE_LUCENE_PTR(SimpleFragmenter)
    DECLARE_LUCENE_PTR(SimpleHTMLEncoder)
    DECLARE_LUCENE_PTR(SimpleHTMLFormatter)
    DECLARE_LUCENE_PTR(SimpleSpanFragmenter)
    DECLARE_LUCENE_PTR(SpanGradientFormatter)
    DECLARE_LUCENE_PTR(StringBuffer)
    DECLARE_LUCENE_PTR(TextFragment)
    DECLARE_LUCENE_PTR(TokenGroup)
    DECLARE_LUCENE_PTR(TokenSources)
    DECLARE_LUCENE_PTR(WeightedSpanTerm)
    DECLARE_LUCENE_PTR(WeightedSpanTermExtractor)
    DECLARE_LUCENE_PTR(WeightedTerm)

    // memory
    DECLARE_LUCENE_PTR(MemoryIndex)
    DECLARE_LUCENE_PTR(MemoryIndexInfo)
    DECLARE_LUCENE_PTR(MemoryIndexReader)

    typedef HashMap<String, WeightedSpanTermPtr > MapStringWeightedSpanTerm;
    typedef HashMap<String, WeightedTermPtr > MapStringWeightedTerm;
    typedef HashMap<String, SpanQueryPtr > MapStringSpanQuery;
    typedef HashMap<String, Collection<int32_t> > MapStringIntCollection;
    typedef HashMap<String, MemoryIndexInfoPtr > MapStringMemoryIndexInfo;

    typedef std::pair< String, Collection<int32_t> > PairStringIntCollection;
    typedef Collection< PairStringIntCollection > Collection<String>IntCollection;

    typedef std::pair< String, MemoryIndexInfoPtr > PairStringMemoryIndexInfo;
    typedef Collection< PairStringMemoryIndexInfo > Collection<String>MemoryIndexInfo;

    typedef HashSet< WeightedTermPtr, luceneHash<WeightedTermPtr>, luceneEquals<WeightedTermPtr> > SetWeightedTerm;
}

#endif
