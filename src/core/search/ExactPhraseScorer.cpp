/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ExactPhraseScorer.h"
#include "_ExactPhraseScorer.h"
#include "PhrasePositions.h"
#include "PhraseQuery.h"
#include "Weight.h"
#include "TermPositions.h"
#include "Similarity.h"
#include "MiscUtils.h"

namespace Lucene
{
    const int32_t ExactPhraseScorer::SCORE_CACHE_SIZE = 32;
    const int32_t ExactPhraseScorer::CHUNK = 4096;
    
    ExactPhraseScorer::ExactPhraseScorer(WeightPtr weight, Collection<PostingsAndFreqPtr> postings, SimilarityPtr similarity, ByteArray norms) : Scorer(similarity, weight)
    {
        this->scoreCache = DoubleArray::newInstance(SCORE_CACHE_SIZE);
        this->gen = 0;
        this->counts = IntArray::newInstance(CHUNK);
        this->gens = IntArray::newInstance(CHUNK);
        this->noDocs = false;
        this->_docID = -1;
        this->_freq = 0;
        
        this->norms = norms;
        this->value = weight->getValue();

        chunkStates = Collection<ChunkStatePtr>::newInstance(postings.size());

        endMinus1 = postings.size() - 1;

        for (int32_t i = 0; i < postings.size(); ++i)
        {
            // Coarse optimization: advance(target) is fairly costly, so, if the relative freq of the 2nd
            // rarest term is not that much (> 1/5th) rarer than the first term, then we just use .nextDoc() 
            // when ANDing.  This buys ~15% gain for phrases where freq of rarest 2 terms is close
            bool useAdvance = postings[i]->docFreq > (5 * postings[0]->docFreq);
            chunkStates[i] = newLucene<ChunkState>(postings[i]->postings, -postings[i]->position, useAdvance);
            if (i > 0 && !postings[i]->postings->next())
            {
                noDocs = true;
                return;
            }
        }

        for (int32_t i = 0; i < SCORE_CACHE_SIZE; ++i)
            scoreCache[i] = getSimilarity()->tf((double)i) * value;
    }
    
    ExactPhraseScorer::~ExactPhraseScorer()
    {
    }
    
    int32_t ExactPhraseScorer::nextDoc()
    {
        while (true)
        {
            // first (rarest) term
            if (!chunkStates[0]->posEnum->next())
            {
                _docID = DocIdSetIterator::NO_MORE_DOCS;
                return _docID;
            }

            int32_t doc = chunkStates[0]->posEnum->doc();

            // not-first terms
            int32_t i = 1;
            while (i < chunkStates.size())
            {
                ChunkStatePtr cs(chunkStates[i]);
                int32_t doc2 = cs->posEnum->doc();
                if (cs->useAdvance)
                {
                    if (doc2 < doc)
                    {
                        if (!cs->posEnum->skipTo(doc))
                        {
                            _docID = DocIdSetIterator::NO_MORE_DOCS;
                            return _docID;
                        }
                        else
                            doc2 = cs->posEnum->doc();
                    }
                }
                else
                {
                    int32_t iter = 0;
                    while (doc2 < doc)
                    {
                        // safety net -- fallback to .skipTo if we've done too many .nextDocs
                        if (++iter == 50)
                        {
                            if (!cs->posEnum->skipTo(doc))
                            {
                                _docID = DocIdSetIterator::NO_MORE_DOCS;
                                return _docID;
                            }
                            else
                                doc2 = cs->posEnum->doc();
                            break;
                        }
                        else
                        {
                            if (cs->posEnum->next())
                                doc2 = cs->posEnum->doc();
                            else
                            {
                                _docID = DocIdSetIterator::NO_MORE_DOCS;
                                return _docID;
                            }
                        }
                    }
                }
                if (doc2 > doc)
                    break;
                ++i;
            }

            if (i == chunkStates.size())
            {
                // this doc has all the terms -- now test whether phrase occurs
                _docID = doc;

                _freq = phraseFreq();
                if (_freq != 0)
                    return _docID;
            }
        }
    }
    
    int32_t ExactPhraseScorer::advance(int32_t target)
    {
        // first term
        if (!chunkStates[0]->posEnum->skipTo(target))
        {
            _docID = DocIdSetIterator::NO_MORE_DOCS;
            return _docID;
        }
        int32_t doc = chunkStates[0]->posEnum->doc();

        while (true)
        {
            // not-first terms
            int32_t i = 1;
            while (i < chunkStates.size())
            {
                int32_t doc2 = chunkStates[i]->posEnum->doc();
                if (doc2 < doc)
                {
                    if (!chunkStates[i]->posEnum->skipTo(doc))
                    {
                        _docID = DocIdSetIterator::NO_MORE_DOCS;
                        return _docID;
                    } 
                    else
                        doc2 = chunkStates[i]->posEnum->doc();
                }
                if (doc2 > doc)
                    break;
                ++i;
            }

            if (i == chunkStates.size())
            {
                // this doc has all the terms -- now test whether phrase occurs
                _docID = doc;
                _freq = phraseFreq();
                if (_freq != 0)
                    return _docID;
            }

            if (!chunkStates[0]->posEnum->next())
            {
                _docID = DocIdSetIterator::NO_MORE_DOCS;
                return _docID;
            }
            else
                doc = chunkStates[0]->posEnum->doc();
        }
    }
    
    String ExactPhraseScorer::toString()
    {
        return L"ExactPhraseScorer(" + weight->toString() + L")";
    }
    
    double ExactPhraseScorer::freq()
    {
        return _freq;
    }
    
    int32_t ExactPhraseScorer::docID()
    {
        return _docID;
    }
    
    double ExactPhraseScorer::score()
    {
        double raw = 0; // raw score
        if (_freq < SCORE_CACHE_SIZE)
            raw = scoreCache[_freq];
        else
            raw = getSimilarity()->tf((double)_freq) * value;
        return !norms ? raw : raw * getSimilarity()->decodeNormValue(norms[_docID]); // normalize
    }
    
    double ExactPhraseScorer::phraseFreq()
    {
        _freq = 0;

        // init chunks
        for (int32_t i = 0; i < chunkStates.size(); ++i)
        {
            ChunkStatePtr cs(chunkStates[i]);
            cs->posLimit = cs->posEnum->freq();
            cs->pos = cs->offset + cs->posEnum->nextPosition();
            cs->posUpto = 1;
            cs->lastPos = -1;
        }

        int32_t chunkStart = 0;
        int32_t chunkEnd = CHUNK;

        // process chunk by chunk
        bool end = false;

        while (!end)
        {
            ++gen;

            if (gen == 0)
            {
                // wraparound
                MiscUtils::arrayFill(gens.get(), 0, gens.size(), 0);
                ++gen;
            }

            // first term
            {
                ChunkStatePtr cs(chunkStates[0]);
                while (cs->pos < chunkEnd)
                {
                    if (cs->pos > cs->lastPos)
                    {
                        cs->lastPos = cs->pos;
                        int32_t posIndex = cs->pos - chunkStart;
                        counts[posIndex] = 1;
                        BOOST_ASSERT(gens[posIndex] != gen);
                        gens[posIndex] = gen;
                    }

                    if (cs->posUpto == cs->posLimit)
                    {
                        end = true;
                        break;
                    }
                    ++cs->posUpto;
                    cs->pos = cs->offset + cs->posEnum->nextPosition();
                }
            }

            // middle terms
            bool any = true;
            for (int32_t t = 1; t < endMinus1; ++t)
            {
                ChunkStatePtr cs(chunkStates[t]);
                any = false;
                while (cs->pos < chunkEnd)
                {
                    if (cs->pos > cs->lastPos)
                    {
                        cs->lastPos = cs->pos;
                        int32_t posIndex = cs->pos - chunkStart;
                        if (posIndex >= 0 && gens[posIndex] == gen && counts[posIndex] == t)
                        {
                            // viable
                            ++counts[posIndex];
                            any = true;
                        }
                    }

                    if (cs->posUpto == cs->posLimit)
                    {
                        end = true;
                        break;
                    }
                    ++cs->posUpto;
                    cs->pos = cs->offset + cs->posEnum->nextPosition();
                }

                if (!any)
                    break;
            }

            if (!any)
            {
                // petered out for this chunk
                chunkStart += CHUNK;
                chunkEnd += CHUNK;
                continue;
            }

            // last term
            {
                ChunkStatePtr cs(chunkStates[endMinus1]);
                while (cs->pos < chunkEnd)
                {
                    if (cs->pos > cs->lastPos)
                    {
                        cs->lastPos = cs->pos;
                        int32_t posIndex = cs->pos - chunkStart;
                        if (posIndex >= 0 && gens[posIndex] == gen && counts[posIndex] == endMinus1)
                            ++_freq;
                    }

                    if (cs->posUpto == cs->posLimit)
                    {
                        end = true;
                        break;
                    }
                    ++cs->posUpto;
                    cs->pos = cs->offset + cs->posEnum->nextPosition();
                }
            }

            chunkStart += CHUNK;
            chunkEnd += CHUNK;
        }

        return _freq;
    }
    
    ChunkState::ChunkState(TermPositionsPtr posEnum, int32_t offset, bool useAdvance)
    {
        this->posUpto = 0;
        this->posLimit = 0;
        this->pos = 0;
        this->lastPos = 0;
        
        this->posEnum = posEnum;
        this->offset = offset;
        this->useAdvance = useAdvance;
    }
    
    ChunkState::~ChunkState()
    {
    }
}
