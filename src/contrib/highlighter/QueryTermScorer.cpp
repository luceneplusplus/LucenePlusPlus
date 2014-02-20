/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "QueryTermScorer.h"
#include "QueryTermExtractor.h"
#include "TermAttribute.h"
#include "WeightedTerm.h"
#include "TokenStream.h"

namespace Lucene {

QueryTermScorer::QueryTermScorer(const QueryPtr& query) {
    ConstructQueryTermScorer(QueryTermExtractor::getTerms(query));
}

QueryTermScorer::QueryTermScorer(const QueryPtr& query, const String& fieldName) {
    ConstructQueryTermScorer(QueryTermExtractor::getTerms(query, false, fieldName));
}

QueryTermScorer::QueryTermScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& fieldName) {
    ConstructQueryTermScorer(QueryTermExtractor::getIdfWeightedTerms(query, reader, fieldName));
}

QueryTermScorer::QueryTermScorer(Collection<WeightedTermPtr> weightedTerms) {
    ConstructQueryTermScorer(weightedTerms);
}

QueryTermScorer::~QueryTermScorer() {
}

void QueryTermScorer::ConstructQueryTermScorer(Collection<WeightedTermPtr> weightedTerms) {
    totalScore = 0;
    maxTermWeight = 0;

    termsToFind = MapStringWeightedTerm::newInstance();
    for (int32_t i = 0; i < weightedTerms.size(); ++i) {
        WeightedTermPtr existingTerm(termsToFind.get(weightedTerms[i]->term));
        if (!existingTerm || existingTerm->weight < weightedTerms[i]->weight) {
            // if a term is defined more than once, always use the highest scoring weight
            termsToFind.put(weightedTerms[i]->term, weightedTerms[i]);
            maxTermWeight = std::max(maxTermWeight, weightedTerms[i]->getWeight());
        }
    }
}

TokenStreamPtr QueryTermScorer::init(const TokenStreamPtr& tokenStream) {
    termAtt = tokenStream->addAttribute<TermAttribute>();
    return TokenStreamPtr();
}

void QueryTermScorer::startFragment(const TextFragmentPtr& newFragment) {
    uniqueTermsInFragment = HashSet<String>::newInstance();
    currentTextFragment = newFragment;
    totalScore = 0;
}

double QueryTermScorer::getTokenScore() {
    String termText(termAtt->term());

    WeightedTermPtr queryTerm(termsToFind.get(termText));
    if (!queryTerm) {
        return 0.0;    // not a query term - return
    }

    // found a query term - is it unique in this doc?
    if (!uniqueTermsInFragment.contains(termText)) {
        totalScore += queryTerm->getWeight();;
        uniqueTermsInFragment.add(termText);
    }

    return queryTerm->getWeight();
}

double QueryTermScorer::getFragmentScore() {
    return totalScore;
}

void QueryTermScorer::allFragmentsProcessed() {
    // this class has no special operations to perform at end of processing
}

double QueryTermScorer::getMaxTermWeight() {
    return maxTermWeight;
}

}
