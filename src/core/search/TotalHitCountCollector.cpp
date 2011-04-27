/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TotalHitCountCollector.h"

namespace Lucene
{
    TotalHitCountCollector::TotalHitCountCollector()
    {
        totalHits = 0;
    }
    
    TotalHitCountCollector::~TotalHitCountCollector()
    {
    }
    
    int32_t TotalHitCountCollector::getTotalHits()
    {
        return totalHits;
    }
    
    void TotalHitCountCollector::setScorer(ScorerPtr scorer)
    {
    }
    
    void TotalHitCountCollector::collect(int32_t doc)
    {
        ++totalHits;
    }
    
    void TotalHitCountCollector::setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
    }
    
    bool TotalHitCountCollector::acceptsDocsOutOfOrder()
    {
        return true;
    }
}
