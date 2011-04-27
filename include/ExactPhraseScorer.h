/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef EXACTPHRASESCORER_H
#define EXACTPHRASESCORER_H

#include "Scorer.h"

namespace Lucene
{
    class ExactPhraseScorer : public Scorer
    {
    public:
        ExactPhraseScorer(WeightPtr weight, Collection<PostingsAndFreqPtr> postings, SimilarityPtr similarity, ByteArray norms);
        virtual ~ExactPhraseScorer();
    
        LUCENE_CLASS(ExactPhraseScorer);
    
    private:
        ByteArray norms;
        double value;

        static const int32_t SCORE_CACHE_SIZE;
        DoubleArray scoreCache;

        int32_t endMinus1;

        static const int32_t CHUNK;

        int32_t gen;
        IntArray counts;
        IntArray gens;
        
        Collection<ChunkStatePtr> chunkStates;

        int32_t _docID;
        int32_t _freq;

    public:
        bool noDocs;

    public:
        virtual int32_t nextDoc();
        virtual int32_t advance(int32_t target);
        virtual String toString();
        virtual double freq();
        virtual int32_t docID();
        virtual double score();

    protected:
        virtual double phraseFreq();
    };
}

#endif
