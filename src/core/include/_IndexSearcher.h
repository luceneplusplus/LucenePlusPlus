/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _INDEXSEARCHER_H
#define _INDEXSEARCHER_H

#include "Scorer.h"

namespace Lucene
{
    /// A thread subclass for searching a single searchable
    class MultiIndexSearcherCallableNoSort : public LuceneObject
    {
    public:
        MultiIndexSearcherCallableNoSort(SynchronizePtr lock, IndexSearcherPtr searchable, WeightPtr weight, FilterPtr filter, 
                                         int32_t nDocs, HitQueuePtr hq, int32_t i, Collection<int32_t> starts);
        virtual ~MultiIndexSearcherCallableNoSort();
    
        LUCENE_CLASS(MultiIndexSearcherCallableNoSort);
    
    protected:
        SynchronizePtr lock;
        IndexSearcherPtr searchable;
        WeightPtr weight;
        FilterPtr filter;
        int32_t nDocs;
        int32_t i;
        HitQueuePtr hq;
        Collection<int32_t> starts;
    
    public:
        TopDocsPtr call();
    };
    
    /// A thread subclass for searching a single searchable
    class MultiIndexSearcherCallableWithSort : public LuceneObject
    {
    public:
        MultiIndexSearcherCallableWithSort(SynchronizePtr lock, IndexSearcherPtr searchable, WeightPtr weight, FilterPtr filter, 
                                           int32_t nDocs, TopFieldCollectorPtr hq, SortPtr sort, int32_t i, 
                                           Collection<int32_t> starts);
        virtual ~MultiIndexSearcherCallableWithSort();
    
        LUCENE_CLASS(MultiIndexSearcherCallableWithSort);
    
    protected:
        SynchronizePtr lock;
        IndexSearcherPtr searchable;
        WeightPtr weight;
        FilterPtr filter;
        int32_t nDocs;
        int32_t i;
        TopFieldCollectorPtr hq;
        Collection<int32_t> starts;
        SortPtr sort;
    
    private:
        FakeScorerPtr fakeScorer;
    
    public:
        TopFieldDocsPtr call();
    };
    
    class FakeScorer : public Scorer
    {
    public:
        FakeScorer();
        virtual ~FakeScorer();
    
        LUCENE_CLASS(FakeScorer);
    
    public:
        double _score;
        int32_t doc;
    
    public:
        virtual int32_t advance(int32_t target);
        virtual int32_t docID();
        virtual double freq();
        virtual int32_t nextDoc();
        virtual double score();
    };
}

#endif
