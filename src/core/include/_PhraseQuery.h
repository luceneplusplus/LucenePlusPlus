/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _PHRASEQUERY_H
#define _PHRASEQUERY_H

#include "Weight.h"

namespace Lucene
{
    class PhraseWeight : public Weight
    {
    public:
        PhraseWeight(PhraseQueryPtr query, SearcherPtr searcher);
        virtual ~PhraseWeight();
    
        LUCENE_CLASS(PhraseWeight);
    
    protected:
        PhraseQueryPtr query;
        SimilarityPtr similarity;
        double value;
        double idf;
        double queryNorm;
        double queryWeight;
        IDFExplanationPtr idfExp;
    
    public:
        virtual String toString();
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
    };
}

#endif
