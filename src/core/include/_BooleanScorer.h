/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _BOOLEANSCORER_H
#define _BOOLEANSCORER_H

#include "Scorer.h"
#include "Collector.h"

namespace Lucene
{
    class BooleanScorerCollector : public Collector
    {
    public:
        BooleanScorerCollector(int32_t mask, BucketTablePtr bucketTable);
        virtual ~BooleanScorerCollector();
    
        LUCENE_CLASS(BooleanScorerCollector);
    
    protected:
        BucketTableWeakPtr _bucketTable;
        int32_t mask;
        ScorerWeakPtr _scorer;
    
    public:
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);        
        virtual void setScorer(ScorerPtr scorer);
        virtual bool acceptsDocsOutOfOrder();
    };
    
    // An internal class which is used in score(Collector, int32_t) for setting the current score. This is required 
    // since Collector exposes a setScorer method and implementations that need the score will call scorer->score().
    // Therefore the only methods that are implemented are score() and doc().
    class BucketScorer : public Scorer
    {
    public:
        BucketScorer(WeightPtr weight);
        virtual ~BucketScorer();
    
        LUCENE_CLASS(BucketScorer);
    
    public:
        double _score;
        int32_t doc;
        int32_t _freq;
    
    public:
        virtual int32_t advance(int32_t target);
        virtual int32_t docID();
        virtual double freq();
        virtual int32_t nextDoc();
        virtual double score();
    };
    
    class Bucket : public LuceneObject
    {
    public:
        Bucket();
        virtual ~Bucket();
        
        LUCENE_CLASS(Bucket);
    
    public:
        int32_t doc; // tells if bucket is valid
        double score; // incremental score
        int32_t bits; // used for bool constraints
        int32_t coord; // count of terms in score
        BucketWeakPtr _next; // next valid bucket
    };
    
    /// A simple hash table of document scores within a range.
    class BucketTable : public LuceneObject
    {
    public:
        BucketTable();
        virtual ~BucketTable();
        
        LUCENE_CLASS(BucketTable);
    
    public:
        static const int32_t SIZE;
        static const int32_t MASK;
        
        Collection<BucketPtr> buckets;
        BucketPtr first; // head of valid list
    
    public:
        CollectorPtr newCollector(int32_t mask);
        int32_t size();
    };
    
    class SubScorer : public LuceneObject
    {
    public:
        SubScorer(ScorerPtr scorer, bool required, bool prohibited, CollectorPtr collector, SubScorerPtr next);
        virtual ~SubScorer();
        
        LUCENE_CLASS(SubScorer);
    
    public:
        ScorerPtr scorer;
        bool prohibited;
        CollectorPtr collector;
        SubScorerPtr next;
    };
}

#endif
