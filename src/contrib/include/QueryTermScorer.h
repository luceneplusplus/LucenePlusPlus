/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef QUERYTERMSCORER_H
#define QUERYTERMSCORER_H

#include "LuceneContrib.h"
#include "HighlighterScorer.h"

namespace Lucene {

/// {@link HighlighterScorer} implementation which scores text fragments by the number of unique query terms found.
/// This class uses the {@link QueryTermExtractor} class to process determine the query terms and their
/// boosts to be used.
class LPPCONTRIBAPI QueryTermScorer : public HighlighterScorer, public LuceneObject {
public:
    /// @param query a Lucene query (ideally rewritten using query.rewrite before being passed to this class
    /// and the searcher)
    QueryTermScorer(const QueryPtr& query);

    /// @param query a Lucene query (ideally rewritten using query.rewrite before being passed to this class
    /// and the searcher)
    /// @param fieldName the Field name which is used to match Query terms
    QueryTermScorer(const QueryPtr& query, const String& fieldName);

    /// @param query a Lucene query (ideally rewritten using query.rewrite before being passed to this class
    /// and the searcher)
    /// @param reader used to compute IDF which can be used to
    /// a) score selected fragments better
    /// b) use graded highlights eg set font color intensity
    /// @param fieldName the field on which Inverse Document Frequency (IDF) calculations are based
    QueryTermScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& fieldName);

    /// @param weightedTerms an array of pre-created {@link WeightedTerm}s
    QueryTermScorer(Collection<WeightedTermPtr> weightedTerms);

    virtual ~QueryTermScorer();

    LUCENE_CLASS(QueryTermScorer);

public:
    TextFragmentPtr currentTextFragment;
    HashSet<String> uniqueTermsInFragment;

    double totalScore;
    double maxTermWeight;

protected:
    MapStringWeightedTerm termsToFind;
    TermAttributePtr termAtt;

protected:
    void ConstructQueryTermScorer(Collection<WeightedTermPtr> weightedTerms);

public:
    virtual TokenStreamPtr init(const TokenStreamPtr& tokenStream);
    virtual void startFragment(const TextFragmentPtr& newFragment);
    virtual double getTokenScore();
    virtual double getFragmentScore();
    virtual void allFragmentsProcessed();

    /// @return The highest weighted term (useful for passing to GradientFormatter to set top end of coloring scale.
    virtual double getMaxTermWeight();
};

}

#endif
