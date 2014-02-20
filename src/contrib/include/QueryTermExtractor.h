/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef QUERYTERMEXTRACTOR_H
#define QUERYTERMEXTRACTOR_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Utility class used to extract the terms used in a query, plus any weights.  This class will not
/// find terms for MultiTermQuery, TermRangeQuery and PrefixQuery classes so the caller must pass a
/// rewritten query (see Query.rewrite) to obtain a list of expanded terms.
class LPPCONTRIBAPI QueryTermExtractor : public LuceneObject {
public:
    virtual ~QueryTermExtractor();
    LUCENE_CLASS(QueryTermExtractor);

public:
    /// Extracts all terms texts of a given Query into an array of WeightedTerms
    ///
    /// @param query Query to extract term texts from.
    /// @return an array of the terms used in a query, plus their weights.
    static Collection<WeightedTermPtr> getTerms(const QueryPtr& query);

    /// Extracts all terms texts of a given Query into an array of WeightedTerms
    ///
    /// @param query Query to extract term texts from.
    /// @param reader used to compute IDF which can be used to
    /// a) score selected fragments better
    /// b) use graded highlights eg changing intensity of font color
    /// @param fieldName the field on which Inverse Document Frequency (IDF) calculations are based.
    /// @return an array of the terms used in a query, plus their weights.
    static Collection<WeightedTermPtr> getIdfWeightedTerms(const QueryPtr& query, const IndexReaderPtr& reader, const String& fieldName);

    /// Extracts all terms texts of a given Query into an array of WeightedTerms
    ///
    /// @param query Query to extract term texts from.
    /// @param prohibited true to extract "prohibited" terms, too.
    /// @param fieldName The fieldName used to filter query terms.
    /// @return an array of the terms used in a query, plus their weights.
    static Collection<WeightedTermPtr> getTerms(const QueryPtr& query, bool prohibited, const String& fieldName);

    /// Extracts all terms texts of a given Query into an array of WeightedTerms
    ///
    /// @param query Query to extract term texts from.
    /// @param prohibited true to extract "prohibited" terms, too.
    /// @return an array of the terms used in a query, plus their weights.
    static Collection<WeightedTermPtr> getTerms(const QueryPtr& query, bool prohibited);

    static void getTerms(const QueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName);

protected:
    /// extractTerms is currently the only query-independent means of introspecting queries but it only reveals
    /// a list of terms for that query - not the boosts each individual term in that query may or may not have.
    /// "Container" queries such as BooleanQuery should be unwrapped to get at the boost info held in each child
    /// element.
    static void getTermsFromBooleanQuery(const BooleanQueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName);
    static void getTermsFromFilteredQuery(const FilteredQueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName);
};

}

#endif
