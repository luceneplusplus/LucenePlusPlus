/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PhraseScorer.h"
#include "PhraseQuery.h"
#include "PhrasePositions.h"
#include "PhraseQueue.h"
#include "Weight.h"
#include "Similarity.h"

namespace Lucene
{
    PhraseScorer::PhraseScorer(WeightPtr weight, Collection<PostingsAndFreqPtr> postings, SimilarityPtr similarity, ByteArray norms) : Scorer(similarity, weight)
    {
        this->firstTime = true;
        this->more = true;
        this->_freq = 0.0;
        
        this->norms = norms;
        this->value = weight->getValue();
        
        // convert tps to a list of phrase positions.  
        // Note: phrase-position differs from term-position in that its position reflects the phrase offset: pp.pos = tp.pos - offset.
        // This allows to easily identify a matching (exact) phrase when all PhrasePositions have exactly the same position.
        for (int32_t i = 0; i < postings.size(); ++i)
        {
            PhrasePositionsPtr pp(newLucene<PhrasePositions>(postings[i]->postings, postings[i]->position));
            if (last) // add next to end of list
                last->_next = pp;
            else
                first = pp;
            last = pp;
        }
        
        pq = newLucene<PhraseQueue>(postings.size()); // construct empty pq
        first->doc = -1;
    }
    
    PhraseScorer::~PhraseScorer()
    {
    }
    
    int32_t PhraseScorer::docID()
    {
        return first->doc;
    }
    
    int32_t PhraseScorer::nextDoc()
    {
        if (firstTime)
        {
            init();
            firstTime = false;
        }
        else if (more)
            more = last->next(); // trigger further scanning
        if (!doNext())
            first->doc = NO_MORE_DOCS;
        return first->doc;
    }
    
    bool PhraseScorer::doNext()
    {
        while (more)
        {
            while (more && first->doc < last->doc) // find doc with all the terms
            {
                more = first->skipTo(last->doc); // skip first upto last and move it to the end
                firstToLast();
            }
            
            if (more)
            {
                // found a doc with all of the terms
                _freq = phraseFreq(); // check for phrase
                if (_freq == 0.0) // no match
                    more = last->next(); // trigger further scanning
                else
                    return true;
            }
        }
        return false; // no more matches
    }
    
    double PhraseScorer::score()
    {
        double raw = getSimilarity()->tf(_freq) * value; // raw score
        return !norms ? raw : raw * getSimilarity()->decodeNormValue(norms[first->doc]); // normalize
    }
    
    int32_t PhraseScorer::advance(int32_t target)
    {
        firstTime = false;
        for (PhrasePositionsPtr pp(first); more && pp; pp = pp->_next)
            more = pp->skipTo(target);
        if (more)
            sort(); // re-sort
        if (!doNext())
            first->doc = NO_MORE_DOCS;
        return first->doc;
    }
    
    double PhraseScorer::freq()
    {
        return _freq;
    }
    
    void PhraseScorer::init()
    {
        for (PhrasePositionsPtr pp(first); more && pp; pp = pp->_next)
            more = pp->next();
        if (more)
            sort();
    }
    
    void PhraseScorer::sort()
    {
        pq->clear();
        for (PhrasePositionsPtr pp(first); more && pp; pp = pp->_next)
            pq->add(pp);
        pqToList();
    }
    
    void PhraseScorer::pqToList()
    {
        last.reset();
        first.reset();
        while (pq->top())
        {
            PhrasePositionsPtr pp(pq->pop());
            if (last) // add next to end of list
                last->_next = pp;
            else
                first = pp;
            last = pp;
            pp->_next.reset();
        }
    }
    
    void PhraseScorer::firstToLast()
    {
        last->_next = first; // move first to end of list
        last = first;
        first = first->_next;
        last->_next.reset();
    }
    
    String PhraseScorer::toString()
    {
        return L"scorer(" + weight->toString() + L")";
    }
}
