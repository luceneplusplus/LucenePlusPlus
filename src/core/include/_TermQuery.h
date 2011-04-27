/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TERMQUERY_H
#define _TERMQUERY_H

#include "Weight.h"
#include "ReaderUtil.h"

namespace Lucene
{
    class TermWeight : public Weight
    {
    public:
        TermWeight(TermQueryPtr query, SearcherPtr searcher);
        virtual ~TermWeight();
    
        LUCENE_CLASS(TermWeight);
    
    protected:
        TermQueryPtr query;
        SimilarityPtr similarity;
        double value;
        double idf;
        double queryNorm;
        double queryWeight;
        IDFExplanationPtr idfExp;
        HashSet<int32_t> hash;
    
    public:
        virtual String toString();
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
    };
    
    class TermQueryReaderGather : public ReaderGather
    {
    public:
        TermQueryReaderGather(TermQueryPtr query, IntArray dfSum, HashSet<int32_t> hash, IndexReaderPtr r);
        virtual ~TermQueryReaderGather();
        
        LUCENE_CLASS(TermQueryReaderGather);
    
    protected:
        TermQueryWeakPtr _query;
        IntArray dfSum;
        HashSet<int32_t> hash;
    
    protected:
        virtual void add(int32_t base, IndexReaderPtr r);
    };
}

#endif
