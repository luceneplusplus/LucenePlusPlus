/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "QueryTermExtractor.h"
#include "Term.h"
#include "BooleanQuery.h"
#include "BooleanClause.h"
#include "FilteredQuery.h"
#include "WeightedTerm.h"
#include "IndexReader.h"
#include "MiscUtils.h"

namespace Lucene {

QueryTermExtractor::~QueryTermExtractor() {
}

Collection<WeightedTermPtr> QueryTermExtractor::getTerms(const QueryPtr& query) {
    return getTerms(query, false);
}

Collection<WeightedTermPtr> QueryTermExtractor::getIdfWeightedTerms(const QueryPtr& query, const IndexReaderPtr& reader, const String& fieldName) {
    Collection<WeightedTermPtr> terms(getTerms(query, false, fieldName));
    int32_t totalNumDocs = reader->numDocs();
    for (int32_t i = 0; i < terms.size(); ++i) {
        try {
            int32_t docFreq = reader->docFreq(newLucene<Term>(fieldName, terms[i]->term));
            // docFreq counts deletes
            if (totalNumDocs < docFreq) {
                docFreq = totalNumDocs;
            }
            // IDF algorithm taken from DefaultSimilarity class
            double idf = (double)(std::log((double)totalNumDocs / (double)(docFreq + 1)) + 1.0);
            terms[i]->weight *= idf;
        } catch (...) {
            // ignore
        }
    }
    return terms;
}

Collection<WeightedTermPtr> QueryTermExtractor::getTerms(const QueryPtr& query, bool prohibited, const String& fieldName) {
    SetWeightedTerm terms(SetWeightedTerm::newInstance());
    getTerms(query, terms, prohibited, fieldName);
    return Collection<WeightedTermPtr>::newInstance(terms.begin(), terms.end());
}

Collection<WeightedTermPtr> QueryTermExtractor::getTerms(const QueryPtr& query, bool prohibited) {
    SetWeightedTerm terms(SetWeightedTerm::newInstance());
    getTerms(query, terms, prohibited, L"");
    return Collection<WeightedTermPtr>::newInstance(terms.begin(), terms.end());
}

void QueryTermExtractor::getTerms(const QueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName) {
    try {
        if (MiscUtils::typeOf<BooleanQuery>(query)) {
            getTermsFromBooleanQuery(boost::dynamic_pointer_cast<BooleanQuery>(query), terms, prohibited, fieldName);
        } else if (MiscUtils::typeOf<FilteredQuery>(query)) {
            getTermsFromFilteredQuery(boost::dynamic_pointer_cast<FilteredQuery>(query), terms, prohibited, fieldName);
        } else {
            SetTerm nonWeightedTerms(SetTerm::newInstance());
            query->extractTerms(nonWeightedTerms);
            for (SetTerm::iterator term = nonWeightedTerms.begin(); term != nonWeightedTerms.end(); ++term) {
                if (fieldName.empty() || (*term)->field() == fieldName) {
                    terms.add(newLucene<WeightedTerm>(query->getBoost(), (*term)->text()));
                }
            }
        }
    } catch (UnsupportedOperationException&) {
        // this is non-fatal for our purposes
    }
}

void QueryTermExtractor::getTermsFromBooleanQuery(const BooleanQueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName) {
    Collection<BooleanClausePtr> queryClauses(query->getClauses());
    for (int32_t i = 0; i < queryClauses.size(); ++i) {
        if (prohibited || queryClauses[i]->getOccur() != BooleanClause::MUST_NOT) {
            getTerms(queryClauses[i]->getQuery(), terms, prohibited, fieldName);
        }
    }
}

void QueryTermExtractor::getTermsFromFilteredQuery(const FilteredQueryPtr& query, SetWeightedTerm terms, bool prohibited, const String& fieldName) {
    getTerms(query->getQuery(), terms, prohibited, fieldName);
}

}
