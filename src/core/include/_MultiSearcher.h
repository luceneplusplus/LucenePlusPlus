/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _MULTISEARCHER_H
#define _MULTISEARCHER_H

#include "Searcher.h"
#include "Collector.h"

namespace Lucene
{
    /// Document Frequency cache acting as a Dummy-Searcher.  This class is not a full-fledged Searcher, but 
    /// only supports the methods necessary to initialize Weights.
    class CachedDfSource : public Searcher
    {
    public:
        CachedDfSource(MapTermInt dfMap, int32_t maxDoc, SimilarityPtr similarity);
        virtual ~CachedDfSource();
    
        LUCENE_CLASS(CachedDfSource);
    
    protected:
        MapTermInt dfMap; // Map from Terms to corresponding doc freqs
        int32_t _maxDoc; // document count
    
    public:
        virtual int32_t docFreq(TermPtr term);
        virtual Collection<int32_t> docFreqs(Collection<TermPtr> terms);
        virtual int32_t maxDoc();
        virtual QueryPtr rewrite(QueryPtr query);
        virtual void close();
        virtual DocumentPtr doc(int32_t n);
        virtual DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector);
        virtual ExplanationPtr explain(WeightPtr weight, int32_t doc);
        virtual void search(WeightPtr weight, FilterPtr filter, CollectorPtr results);
        virtual TopDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n);
        virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort);
    };

    /// A subclass for searching a single searchable
    class MultiSearcherCallableNoSort : public LuceneObject
    {
    public:
        MultiSearcherCallableNoSort(SynchronizePtr lock, SearchablePtr searchable, WeightPtr weight, FilterPtr filter, int32_t nDocs, 
                                    HitQueuePtr hq, int32_t i, Collection<int32_t> starts);
        virtual ~MultiSearcherCallableNoSort();
    
        LUCENE_CLASS(MultiSearcherCallableNoSort);
    
    protected:
        SynchronizePtr lock;
        SearchablePtr searchable;
        WeightPtr weight;
        FilterPtr filter;
        int32_t nDocs;
        int32_t i;
        HitQueuePtr hq;
        Collection<int32_t> starts;
    
    public:
        TopDocsPtr call();
    };
    
    /// A subclass for searching a single searchable
    class MultiSearcherCallableWithSort : public LuceneObject
    {
    public:
        MultiSearcherCallableWithSort(SynchronizePtr lock, SearchablePtr searchable, WeightPtr weight, FilterPtr filter, 
                                      int32_t nDocs, FieldDocSortedHitQueuePtr hq, SortPtr sort, int32_t i, Collection<int32_t> starts);
        virtual ~MultiSearcherCallableWithSort();
    
        LUCENE_CLASS(MultiSearcherCallableWithSort);
    
    protected:
        SynchronizePtr lock;
        SearchablePtr searchable;
        WeightPtr weight;
        FilterPtr filter;
        int32_t nDocs;
        int32_t i;
        FieldDocSortedHitQueuePtr hq;
        Collection<int32_t> starts;
        SortPtr sort;
    
    public:
        TopFieldDocsPtr call();
    };
    
    class MultiSearcherCollector : public Collector
    {
    public:
        MultiSearcherCollector(CollectorPtr collector, int32_t start);
        virtual ~MultiSearcherCollector();
        
        LUCENE_CLASS(MultiSearcherCollector);
    
    protected:
        CollectorPtr collector;
        int32_t start;
    
    public:
        virtual void setScorer(ScorerPtr scorer);
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual bool acceptsDocsOutOfOrder();
    };
}

#endif
