/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENECONTRIB_H
#define LUCENECONTRIB_H

#include "Lucene.h"

namespace Lucene {

// analyzers
DECLARE_SHARED_PTR(ArabicAnalyzer)
DECLARE_SHARED_PTR(ArabicAnalyzerSavedStreams)
DECLARE_SHARED_PTR(ArabicLetterTokenizer)
DECLARE_SHARED_PTR(ArabicNormalizationFilter)
DECLARE_SHARED_PTR(ArabicNormalizer)
DECLARE_SHARED_PTR(ArabicStemFilter)
DECLARE_SHARED_PTR(ArabicStemmer)
DECLARE_SHARED_PTR(BrazilianAnalyzer)
DECLARE_SHARED_PTR(BrazilianAnalyzerSavedStreams)
DECLARE_SHARED_PTR(BrazilianStemFilter)
DECLARE_SHARED_PTR(BrazilianStemmer)
DECLARE_SHARED_PTR(CJKAnalyzer)
DECLARE_SHARED_PTR(CJKAnalyzerSavedStreams)
DECLARE_SHARED_PTR(CJKTokenizer)
DECLARE_SHARED_PTR(ChineseAnalyzer)
DECLARE_SHARED_PTR(ChineseAnalyzerSavedStreams)
DECLARE_SHARED_PTR(ChineseFilter)
DECLARE_SHARED_PTR(ChineseTokenizer)
DECLARE_SHARED_PTR(CzechAnalyzer)
DECLARE_SHARED_PTR(CzechAnalyzerSavedStreams)
DECLARE_SHARED_PTR(DutchAnalyzer)
DECLARE_SHARED_PTR(DutchAnalyzerSavedStreams)
DECLARE_SHARED_PTR(DutchStemFilter)
DECLARE_SHARED_PTR(DutchStemmer)
DECLARE_SHARED_PTR(ElisionFilter)
DECLARE_SHARED_PTR(FrenchAnalyzer)
DECLARE_SHARED_PTR(FrenchAnalyzerSavedStreams)
DECLARE_SHARED_PTR(FrenchStemFilter)
DECLARE_SHARED_PTR(FrenchStemmer)
DECLARE_SHARED_PTR(GermanAnalyzer)
DECLARE_SHARED_PTR(GermanAnalyzerSavedStreams)
DECLARE_SHARED_PTR(GermanStemFilter)
DECLARE_SHARED_PTR(GermanStemmer)
DECLARE_SHARED_PTR(GreekLowerCaseFilter)
DECLARE_SHARED_PTR(GreekAnalyzer)
DECLARE_SHARED_PTR(GreekAnalyzerSavedStreams)
DECLARE_SHARED_PTR(PersianAnalyzer)
DECLARE_SHARED_PTR(PersianAnalyzerSavedStreams)
DECLARE_SHARED_PTR(PersianNormalizationFilter)
DECLARE_SHARED_PTR(PersianNormalizer)
DECLARE_SHARED_PTR(ReverseStringFilter)
DECLARE_SHARED_PTR(RussianAnalyzer)
DECLARE_SHARED_PTR(RussianAnalyzerSavedStreams)
DECLARE_SHARED_PTR(RussianLetterTokenizer)
DECLARE_SHARED_PTR(RussianLowerCaseFilter)
DECLARE_SHARED_PTR(RussianStemFilter)
DECLARE_SHARED_PTR(RussianStemmer)
DECLARE_SHARED_PTR(SnowballFilter)
DECLARE_SHARED_PTR(SnowballAnalyzer)
DECLARE_SHARED_PTR(SnowballAnalyzerSavedStreams)

// highlighter
DECLARE_SHARED_PTR(DefaultEncoder)
DECLARE_SHARED_PTR(Encoder)
DECLARE_SHARED_PTR(FakeReader)
DECLARE_SHARED_PTR(Formatter)
DECLARE_SHARED_PTR(Fragmenter)
DECLARE_SHARED_PTR(FragmentQueue)
DECLARE_SHARED_PTR(GradientFormatter)
DECLARE_SHARED_PTR(Highlighter)
DECLARE_SHARED_PTR(HighlighterScorer)
DECLARE_SHARED_PTR(MapWeightedSpanTerm)
DECLARE_SHARED_PTR(NullFragmenter)
DECLARE_SHARED_PTR(PositionCheckingMap)
DECLARE_SHARED_PTR(PositionSpan)
DECLARE_SHARED_PTR(QueryScorer)
DECLARE_SHARED_PTR(QueryTermExtractor)
DECLARE_SHARED_PTR(QueryTermScorer)
DECLARE_SHARED_PTR(SimpleFragmenter)
DECLARE_SHARED_PTR(SimpleHTMLEncoder)
DECLARE_SHARED_PTR(SimpleHTMLFormatter)
DECLARE_SHARED_PTR(SimpleSpanFragmenter)
DECLARE_SHARED_PTR(SpanGradientFormatter)
DECLARE_SHARED_PTR(StringBuffer)
DECLARE_SHARED_PTR(TextFragment)
DECLARE_SHARED_PTR(TokenGroup)
DECLARE_SHARED_PTR(TokenSources)
DECLARE_SHARED_PTR(WeightedSpanTerm)
DECLARE_SHARED_PTR(WeightedSpanTermExtractor)
DECLARE_SHARED_PTR(WeightedTerm)

// memory
DECLARE_SHARED_PTR(MemoryIndex)
DECLARE_SHARED_PTR(MemoryIndexInfo)
DECLARE_SHARED_PTR(MemoryIndexReader)

typedef HashMap< String, WeightedSpanTermPtr > MapStringWeightedSpanTerm;
typedef HashMap< String, WeightedTermPtr > MapStringWeightedTerm;
typedef HashMap< String, SpanQueryPtr > MapStringSpanQuery;
typedef HashMap< String, Collection<int32_t> > MapStringIntCollection;
typedef HashMap< String, MemoryIndexInfoPtr > MapStringMemoryIndexInfo;

typedef std::pair< String, Collection<int32_t> > PairStringIntCollection;
typedef Collection< PairStringIntCollection > CollectionStringIntCollection;

typedef std::pair< String, MemoryIndexInfoPtr > PairStringMemoryIndexInfo;
typedef Collection< PairStringMemoryIndexInfo > CollectionStringMemoryIndexInfo;

typedef HashSet< WeightedTermPtr, luceneHash<WeightedTermPtr>, luceneEquals<WeightedTermPtr> > SetWeightedTerm;
}

#endif
