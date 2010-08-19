/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Scorer.h"
#include "Collector.h"

namespace Lucene
{
    Scorer::Scorer(SimilarityPtr similarity)
    {
        this->similarity = similarity;
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
}
