/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHECKHITS_H
#define CHECKHITS_H

#include "test_lucene.h"

namespace Lucene {

class CheckHits {
public:
    virtual ~CheckHits();

public:
    /// Some explains methods calculate their values though a slightly different order of operations
    /// from the actual scoring method - this allows for a small amount of variation
    static const double EXPLAIN_SCORE_TOLERANCE_DELTA;

    /// Tests that all documents up to maxDoc which are *not* in the expected result set, have an
    /// explanation which indicates no match (ie: Explanation value of 0.0)
    static void checkNoMatchExplanations(const QueryPtr& q, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results);

    /// Tests that a query matches the an expected set of documents using a HitCollector.
    ///
    /// Note that when using the HitCollector API, documents will be collected if they "match"
    /// regardless of what their score is.
    static void checkHitCollector(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results);

    /// Tests that a query matches the an expected set of documents using Hits.
    ///
    /// Note that when using the Hits API, documents will only be returned if they have a
    /// positive normalized score.
    static void checkHits(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, Collection<int32_t> results);

    /// Tests that a Hits has an expected order of documents
    static void checkDocIds(Collection<int32_t> results, Collection<ScoreDocPtr> hits);

    /// Tests that two queries have an expected order of documents, and that the two queries have
    /// the same score values.
    static void checkHitsQuery(const QueryPtr& query, Collection<ScoreDocPtr> hits1, Collection<ScoreDocPtr> hits2, Collection<int32_t> results);
    static void checkEqual(const QueryPtr& query, Collection<ScoreDocPtr> hits1, Collection<ScoreDocPtr> hits2);

    /// Asserts that the explanation value for every document matching a query corresponds with the true score.
    /// Optionally does "deep" testing of the explanation details.
    static void checkExplanations(const QueryPtr& query, const String& defaultFieldName, const SearcherPtr& searcher, bool deep = false);

    /// Assert that an explanation has the expected score, and optionally that its sub-details max/sum/factor
    /// match to that score.
    static void verifyExplanation(const String& q, int32_t doc, double score, bool deep, const ExplanationPtr& expl);
};

}

#endif
