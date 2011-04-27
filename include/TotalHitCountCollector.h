/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOTALHITCOUNTCOLLECTOR_H
#define TOTALHITCOUNTCOLLECTOR_H

#include "Collector.h"

namespace Lucene
{
    /// Just counts the total number of hits.
    class LPPAPI TotalHitCountCollector : public Collector
    {
    public:
        TotalHitCountCollector();
        virtual ~TotalHitCountCollector();
        
        LUCENE_CLASS(TotalHitCountCollector);
    
    private:
        int32_t totalHits;
    
    public:
        int32_t getTotalHits();
        virtual void setScorer(ScorerPtr scorer);
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual bool acceptsDocsOutOfOrder();
    };
}

#endif
