/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Scorer.h"
#include "Collector.h"

namespace Lucene {
    
    Scorer::Scorer(const SimilarityPtr& similarity) {
        this->similarity = similarity;
    }
    
    Scorer::Scorer(const WeightPtr& weight) {
        this->weight = weight;
    }
    
    Scorer::~Scorer() {
    }
    
    SimilarityPtr Scorer::getSimilarity() {
        return similarity;
    }
    
    void Scorer::score(const CollectorPtr& collector) {
        collector->setScorer(shared_from_this());
        int32_t doc;
        while ((doc = nextDoc()) != NO_MORE_DOCS) {
            collector->collect(doc);
        }
    }
    
    bool Scorer::score(const CollectorPtr& collector, int32_t max, int32_t firstDocID) {
        collector->setScorer(shared_from_this());
        int32_t doc = firstDocID;
        while (doc < max) {
            collector->collect(doc);
            doc = nextDoc();
        }
        return (doc != NO_MORE_DOCS);
    }
    
    void Scorer::visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship,
                                 ScorerVisitor *visitor){
        QueryPtr q = weight->getQuery();
        switch (relationship) {
            case BooleanClause::MUST:
            visitor->visitRequired(parent, q, shared_from_this());
            break;
            case BooleanClause::MUST_NOT:
            visitor->visitProhibited(parent, q, shared_from_this());
            break;
            case BooleanClause::SHOULD:
            visitor->visitOptional(parent, q, shared_from_this());
            break;
        }
    }
    
    void Scorer::visitScorers(ScorerVisitor *visitor) {
        boost::shared_ptr<Query> s_obj;
        
        visitSubScorers(s_obj, BooleanClause::MUST/*must id default*/, visitor);
    }
    
    
    
}
