/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef QUERYUTILS_H
#define QUERYUTILS_H

#include "test_lucene.h"

namespace Lucene {

class QueryUtils {
public:
    virtual ~QueryUtils();

public:
    /// Check the types of things query objects should be able to do.
    static void check(const QueryPtr& q);

    /// Check very basic hashCode and equals
    static void checkHashEquals(const QueryPtr& q);

    static void checkEqual(const QueryPtr& q1, const QueryPtr& q2);
    static void checkUnequal(const QueryPtr& q1, const QueryPtr& q2);

    /// Deep check that explanations of a query 'score' correctly
    static void checkExplanations(const QueryPtr& q, const SearcherPtr& s);

    /// Various query sanity checks on a searcher, some checks are only done
    /// for types of IndexSearcher.
    static void check(const QueryPtr& q1, const SearcherPtr& s);
    static void check(const QueryPtr& q1, const SearcherPtr& s, bool wrap);

    /// Given an IndexSearcher, returns a new IndexSearcher whose IndexReader is a MultiReader
    /// containing the Reader of the original IndexSearcher, as well as several "empty"
    /// IndexReaders - some of which will have deleted documents in them.  This new IndexSearcher
    /// should behave exactly the same as the original IndexSearcher.
    /// @param s the searcher to wrap.
    /// @param edge if negative, s will be the first sub; if 0, s will be in the middle, if
    /// positive s will be the last sub
    static IndexSearcherPtr wrapUnderlyingReader(const IndexSearcherPtr& s, int32_t edge);

    /// Given a Searcher, returns a new MultiSearcher wrapping the the original Searcher, as well
    /// as several "empty" IndexSearchers - some of which will have deleted documents in them.
    /// This new MultiSearcher should behave exactly the same as the original Searcher.
    /// @param s the Searcher to wrap
    /// @param edge if negative, s will be the first sub; if 0, s will be in hte middle, if positive
    /// s will be the last sub
    static MultiSearcherPtr wrapSearcher(const SearcherPtr& s, int32_t edge);

    static RAMDirectoryPtr makeEmptyIndex(int32_t numDeletedDocs);

    /// Alternate scorer skipTo(), skipTo(), next(), next(), skipTo(), skipTo(), etc and ensure
    /// a hitcollector receives same docs and scores
    static void checkSkipTo(const QueryPtr& q, const IndexSearcherPtr& s);

    /// Check that first skip on just created scorers always goes to the right doc
    static void checkFirstSkipTo(const QueryPtr& q, const IndexSearcherPtr& s);
};

}

#endif
