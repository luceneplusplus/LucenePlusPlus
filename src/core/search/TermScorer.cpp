/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermScorer.h"
#include "TermDocs.h"
#include "Similarity.h"
#include "Weight.h"
#include "Collector.h"

namespace Lucene
{
    const int32_t TermScorer::SCORE_CACHE_SIZE = 32;
    
    TermScorer::TermScorer(WeightPtr weight, TermDocsPtr td, SimilarityPtr similarity, ByteArray norms) : Scorer(similarity, weight)
    {
        this->termDocs = td;
        this->norms = norms;
        this->weightValue = weight->getValue();
        this->doc = -1;
        this->_freq = 0;
        this->docs = Collection<int32_t>::newInstance(32);
        this->freqs = Collection<int32_t>::newInstance(32);
        this->pointer = 0;
        this->pointerMax = 0;
        this->scoreCache = Collection<double>::newInstance(SCORE_CACHE_SIZE);
        
        for (int32_t i = 0; i < SCORE_CACHE_SIZE; ++i)
            scoreCache[i] = getSimilarity()->tf(i) * weightValue;
    }
    
    TermScorer::~TermScorer()
    {
    }
    
    void TermScorer::score(CollectorPtr collector)
    {
        score(collector, INT_MAX, nextDoc());
    }
    
    bool TermScorer::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        // firstDocID is ignored since nextDoc() sets 'doc'
        collector->setScorer(shared_from_this());
        while (doc < max) // for docs in window
        {
            collector->collect(doc);
            
            if (++pointer >= pointerMax)
            {
                pointerMax = termDocs->read(docs, freqs); // refill buffers
                if (pointerMax != 0)
                    pointer = 0;
                else
                {
                    termDocs->close(); // close stream
                    doc = INT_MAX; // set to sentinel value
                    return false;
                }
            }
            doc = docs[pointer];
            _freq = freqs[pointer];
        }
        return true;
    }
    
    int32_t TermScorer::docID()
    {
        return doc;
    }
    
    double TermScorer::freq()
    {
        return _freq;
    }
    
    int32_t TermScorer::nextDoc()
    {
        ++pointer;
        if (pointer >= pointerMax)
        {
            pointerMax = termDocs->read(docs, freqs); // refill buffer
            if (pointerMax != 0)
                pointer = 0;
            else
            {
                termDocs->close(); // close stream
                doc = NO_MORE_DOCS;
                return doc;
            }
        }
        doc = docs[pointer];
        _freq = freqs[pointer];
        return doc;
    }
    
    double TermScorer::score()
    {
        BOOST_ASSERT(doc != -1);
        double raw = _freq < SCORE_CACHE_SIZE ? scoreCache[_freq] : getSimilarity()->tf(_freq) * weightValue; // compute tf(f) * weight
        return norms ? raw * getSimilarity()->decodeNormValue(norms[doc]) : raw; // normalize for field
    }
    
    int32_t TermScorer::advance(int32_t target)
    {
        // first scan in cache
        for (++pointer; pointer < pointerMax; ++pointer)
        {
            if (docs[pointer] >= target)
            {
                _freq = freqs[pointer];
                doc = docs[pointer];
                return doc;
            }
        }
        
        // not found in cache, seek underlying stream
        bool result = termDocs->skipTo(target);
        if (result)
        {
            pointerMax = 1;
            pointer = 0;
            doc = termDocs->doc();
            docs[pointer] = doc;
            _freq = termDocs->freq();
            freqs[pointer] = _freq;
        }
        else
            doc = NO_MORE_DOCS;
        return doc;
    }
    
    String TermScorer::toString()
    {
        return L"scorer(" + weight->toString() + L")";
    }
}
