/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "QueryScorer.h"
#include "WeightedSpanTerm.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "TokenStream.h"
#include "MapWeightedSpanTerm.h"
#include "WeightedSpanTermExtractor.h"

namespace Lucene {

QueryScorer::QueryScorer(const QueryPtr& query) {
    init(query, L"", IndexReaderPtr(), true);
}

QueryScorer::QueryScorer(const QueryPtr& query, const String& field) {
    init(query, field, IndexReaderPtr(), true);
}

QueryScorer::QueryScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& field) {
    init(query, field, reader, true);
}

QueryScorer::QueryScorer(const QueryPtr& query, const IndexReaderPtr& reader, const String& field, const String& defaultField) {
    this->defaultField = defaultField;
    init(query, field, reader, true);
}

QueryScorer::QueryScorer(const QueryPtr& query, const String& field, const String& defaultField) {
    this->defaultField = defaultField;
    init(query, field, IndexReaderPtr(), true);
}

QueryScorer::QueryScorer(Collection<WeightedSpanTermPtr> weightedTerms) {
    init(QueryPtr(), L"", IndexReaderPtr(), true);

    this->fieldWeightedSpanTerms = newLucene<MapWeightedSpanTerm>();
    for (int32_t i = 0; i < weightedTerms.size(); ++i) {
        WeightedSpanTermPtr existingTerm(fieldWeightedSpanTerms->get(weightedTerms[i]->term));
        if (!existingTerm || existingTerm->weight < weightedTerms[i]->weight) {
            // if a term is defined more than once, always use the highest scoring weight
            fieldWeightedSpanTerms->put(weightedTerms[i]->term, weightedTerms[i]);
            maxTermWeight = std::max(maxTermWeight, weightedTerms[i]->getWeight());
        }
    }
    skipInitExtractor = true;
}

QueryScorer::~QueryScorer() {
}

void QueryScorer::init(const QueryPtr& query, const String& field, const IndexReaderPtr& reader, bool expandMultiTermQuery) {
    this->totalScore = 0;
    this->maxTermWeight = 0;
    this->position = -1;
    this->skipInitExtractor = false;
    this->wrapToCaching = true;

    this->reader = reader;
    this->expandMultiTermQuery = expandMultiTermQuery;
    this->query = query;
    this->field = field;
}

double QueryScorer::getFragmentScore() {
    return totalScore;
}

double QueryScorer::getMaxTermWeight() {
    return maxTermWeight;
}

double QueryScorer::getTokenScore() {
    position += posIncAtt->getPositionIncrement();
    String termText(termAtt->term());

    WeightedSpanTermPtr weightedSpanTerm(fieldWeightedSpanTerms->get(termText));

    if (!weightedSpanTerm) {
        return 0.0;
    }

    if (weightedSpanTerm->positionSensitive && !weightedSpanTerm->checkPosition(position)) {
        return 0.0;
    }

    double score = weightedSpanTerm->getWeight();

    // found a query term - is it unique in this doc?
    if (!foundTerms.contains(termText)) {
        totalScore += score;
        foundTerms.add(termText);
    }

    return score;
}

TokenStreamPtr QueryScorer::init(const TokenStreamPtr& tokenStream) {
    position = -1;
    termAtt = tokenStream->addAttribute<TermAttribute>();
    posIncAtt = tokenStream->addAttribute<PositionIncrementAttribute>();
    if (!skipInitExtractor) {
        if (fieldWeightedSpanTerms) {
            fieldWeightedSpanTerms->clear();
        }
        return initExtractor(tokenStream);
    }
    return TokenStreamPtr();
}

WeightedSpanTermPtr QueryScorer::getWeightedSpanTerm(const String& token) {
    return fieldWeightedSpanTerms->get(token);
}

TokenStreamPtr QueryScorer::initExtractor(const TokenStreamPtr& tokenStream) {
    WeightedSpanTermExtractorPtr qse(newLucene<WeightedSpanTermExtractor>(defaultField));

    qse->setExpandMultiTermQuery(expandMultiTermQuery);
    qse->setWrapIfNotCachingTokenFilter(wrapToCaching);
    if (!reader) {
        this->fieldWeightedSpanTerms = qse->getWeightedSpanTerms(query, tokenStream, field);
    } else {
        this->fieldWeightedSpanTerms = qse->getWeightedSpanTermsWithScores(query, tokenStream, field, reader);
    }
    if (qse->isCachedTokenStream()) {
        return qse->getTokenStream();
    }
    return TokenStreamPtr();
}

void QueryScorer::startFragment(const TextFragmentPtr& newFragment) {
    foundTerms = HashSet<String>::newInstance();
    totalScore = 0;
}

bool QueryScorer::isExpandMultiTermQuery() {
    return expandMultiTermQuery;
}

void QueryScorer::setExpandMultiTermQuery(bool expandMultiTermQuery) {
    this->expandMultiTermQuery = expandMultiTermQuery;
}

void QueryScorer::setWrapIfNotCachingTokenFilter(bool wrap) {
    this->wrapToCaching = wrap;
}

}
