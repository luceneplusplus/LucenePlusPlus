/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MULTICOLLECTOR_H
#define MULTICOLLECTOR_H

#include "Collector.h"

namespace Lucene
{
    /// A {@link Collector} which allows running a search with several {@link Collector}s. It offers 
    /// a static {@link #wrap} method which accepts a list of collectors and wraps them with {@link 
    /// MultiCollector}, while filtering out the null null ones.
    class LPPAPI MultiCollector : public Collector
    {
    public:
        MultiCollector(Collection<CollectorPtr> collectors);
        virtual ~MultiCollector();
        
        LUCENE_CLASS(MultiCollector);
    
    private:
        Collection<CollectorPtr> collectors;
    
    public:
        /// Wraps a list of {@link Collector}s with a {@link MultiCollector}. This method works as 
        /// follows:
        /// <ul>
        ///    <li>Filters out the null collectors, so they are not used during search time.
        ///    <li>If the input contains 1 real collector (i.e. non-null ), it is returned.
        ///    <li>Otherwise the method returns a {@link MultiCollector} which wraps the non-null ones.
        /// </ul>
        static CollectorPtr wrap(Collection<CollectorPtr> collectors);
        
        virtual bool acceptsDocsOutOfOrder();
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual void setScorer(ScorerPtr scorer);
    };
}

#endif
