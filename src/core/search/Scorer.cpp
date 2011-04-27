/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "Scorer.h"
#include "Weight.h"
#include "Collector.h"

namespace Lucene
{
    Scorer::Scorer(WeightPtr weight)
    {
        this->weight = weight;
    }
    
    Scorer::Scorer(SimilarityPtr similarity)
    {
        this->similarity = similarity;
    }
    
    Scorer::Scorer(SimilarityPtr similarity, WeightPtr weight)
    {
        this->similarity = similarity;
        this->weight = weight;
    }
    
    Scorer::~Scorer()
    {
    }
    
    SimilarityPtr Scorer::getSimilarity()
    {
        return similarity;
    }
    
    void Scorer::score(CollectorPtr collector)
    {
        collector->setScorer(shared_from_this());
        int32_t doc;
        while ((doc = nextDoc()) != NO_MORE_DOCS)
            collector->collect(doc);
    }
    
    bool Scorer::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        collector->setScorer(shared_from_this());
        int32_t doc = firstDocID;
        while (doc < max)
        {
            collector->collect(doc);
            doc = nextDoc();
        }
        return (doc != NO_MORE_DOCS);
    }
    
    double Scorer::freq()
    {
        boost::throw_exception(UnsupportedOperationException(getClassName() + L" does not implement freq()"));
        return 0;
    }
    
    void Scorer::visitScorers(ScorerVisitorPtr visitor)
    {
        visitSubScorers(QueryPtr(), BooleanClause::MUST, visitor); // must id default
    }
    
    void Scorer::visitSubScorers(QueryPtr parent, BooleanClause::Occur relationship, ScorerVisitorPtr visitor)
    {
        if (!weight)
            boost::throw_exception(UnsupportedOperationException());

        QueryPtr q(weight->getQuery());
        switch (relationship)
        {
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
    
    ScorerVisitor::~ScorerVisitor()
    {
    }
    
    void ScorerVisitor::visitOptional(QueryPtr parent, QueryPtr child, ScorerPtr scorer)
    {
    }
    
    void ScorerVisitor::visitRequired(QueryPtr parent, QueryPtr child, ScorerPtr scorer)
    {
    }
    
    void ScorerVisitor::visitProhibited(QueryPtr parent, QueryPtr child, ScorerPtr scorer)
    {
    }
}
