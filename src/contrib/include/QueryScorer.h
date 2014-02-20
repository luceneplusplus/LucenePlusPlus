/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef QUERYSCORER_H
#define QUERYSCORER_H

#include "LuceneContrib.h"
#include "HighlighterScorer.h"

namespace Lucene {

/// {@link HighlighterScorer} implementation which scores text fragments by the number of unique query terms found.
/// This class converts appropriate {@link Query}s to {@link SpanQuery}s and attempts to score only
/// those terms that participated in generating the 'hit' on the document.
class LPPCONTRIBAPI QueryScorer : public HighlighterScorer, public LuceneObject {
public:
    /// @param query Query to use for highlighting
    QueryScorer(const QueryPtr& query);

    /// @param query Query to use for highlighting
    /// @param field Field to highlight - pass empty string to ignore fields
    QueryScorer(const QueryPtr& query, const String& field);

    /// @param query Query to use for highlighting
    /// @param reader {@link IndexReader} to use for quasi tf/idf scoring
    /// @param field Field to highlight - pass empty string to ignore fields
    QueryScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& field);

    /// @param query Query to use for highlighting
    /// @param reader {@link IndexReader} to use for quasi tf/idf scoring
    /// @param field Field to highlight - pass empty string to ignore fields
    /// @param defaultField
    QueryScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& field, const String& defaultField);

    /// @param query Query to use for highlighting
    /// @param field Field to highlight - pass empty string to ignore fields
    /// @param defaultField
    QueryScorer(const QueryPtr& query, const String& field, const String& defaultField);

    /// @param weightedTerms an array of pre-created {@link WeightedSpanTerm}s
    QueryScorer(Collection<WeightedSpanTermPtr> weightedTerms);

    virtual ~QueryScorer();

    LUCENE_CLASS(QueryScorer);

protected:
    double totalScore;
    HashSet<String> foundTerms;
    MapWeightedSpanTermPtr fieldWeightedSpanTerms;
    double maxTermWeight;
    int32_t position;
    String defaultField;
    TermAttributePtr termAtt;
    PositionIncrementAttributePtr posIncAtt;
    bool expandMultiTermQuery;
    QueryPtr query;
    String field;
    IndexReaderPtr reader;
    bool skipInitExtractor;
    bool wrapToCaching;

protected:
    void init(const QueryPtr& query, const String& field, const IndexReaderPtr& reader, bool expandMultiTermQuery);
    TokenStreamPtr initExtractor(const TokenStreamPtr& tokenStream);

public:
    virtual double getFragmentScore();

    /// @return The highest weighted term (useful for passing to GradientFormatter to set top end of coloring scale).
    virtual double getMaxTermWeight();

    virtual double getTokenScore();
    virtual TokenStreamPtr init(const TokenStreamPtr& tokenStream);
    virtual WeightedSpanTermPtr getWeightedSpanTerm(const String& token);
    virtual void startFragment(const TextFragmentPtr& newFragment);

    /// @return true if multi-term queries should be expanded
    virtual bool isExpandMultiTermQuery();

    /// Controls whether or not multi-term queries are expanded against a {@link MemoryIndex} {@link IndexReader}.
    /// @param expandMultiTermQuery true if multi-term queries should be expanded
    virtual void setExpandMultiTermQuery(bool expandMultiTermQuery);

    /// By default, {@link TokenStream}s that are not of the type {@link CachingTokenFilter} are wrapped in a {@link
    /// CachingTokenFilter} to ensure an efficient reset - if you are already using a different caching {@link
    /// TokenStream} impl and you don't want it to be wrapped, set this to false.
    virtual void setWrapIfNotCachingTokenFilter(bool wrap);
};

}

#endif
