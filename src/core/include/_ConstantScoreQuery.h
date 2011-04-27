/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _CONSTANTSCOREQUERY_H
#define _CONSTANTSCOREQUERY_H

#include "Weight.h"
#include "Collector.h"

namespace Lucene
{
    class ConstantWeight : public Weight
    {
    public:
        ConstantWeight(ConstantScoreQueryPtr constantScorer, SearcherPtr searcher);
        virtual ~ConstantWeight();
    
        LUCENE_CLASS(ConstantWeight);
    
    protected:
        WeightPtr innerWeight;
        ConstantScoreQueryPtr constantScorer;
        SimilarityPtr similarity;
        double queryNorm;
        double queryWeight;
    
    public:
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual bool scoresDocsOutOfOrder();
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
        
        friend class ConstantCollector;
    };
    
    class ConstantScorer : public Scorer
    {
    public:
        ConstantScorer(SimilarityPtr similarity, DocIdSetIteratorPtr docIdSetIterator, WeightPtr w);
        virtual ~ConstantScorer();
    
        LUCENE_CLASS(ConstantScorer);
    
    public:
        DocIdSetIteratorPtr docIdSetIterator;
        double theScore;
    
    public:
        virtual int32_t nextDoc();
        virtual int32_t docID();
        virtual double score();
        virtual int32_t advance(int32_t target);
        
        /// this optimization allows out of order scoring as top scorer
        virtual void score(CollectorPtr collector);

    protected:
        CollectorPtr wrapCollector(CollectorPtr collector);
        
        /// This optimization allows out of order scoring as top scorer
        virtual bool score(CollectorPtr collector, int32_t max, int32_t firstDocID);
        
        friend class ConstantCollector;
    };
    
    class ConstantCollector : public Collector
    {
    public:
        ConstantCollector(ConstantScorerPtr scorer, CollectorPtr collector);
        virtual ~ConstantCollector();
        
        LUCENE_CLASS(ConstantCollector);
    
    protected:
        ConstantScorerWeakPtr _scorer;
        CollectorPtr collector;
    
    public:
        virtual void setScorer(ScorerPtr scorer);
        virtual void collect(int32_t doc);
        virtual void setNextReader(IndexReaderPtr reader, int32_t docBase);
        virtual bool acceptsDocsOutOfOrder();
    };
}

#endif
