/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermScorer.h"
#include "TermDocs.h"
#include "Similarity.h"
#include "Weight.h"
#include "Collector.h"

namespace Lucene {

const int32_t TermScorer::SCORE_CACHE_SIZE = 32;

TermScorer::TermScorer(const WeightPtr& weight, const TermDocsPtr& td, const SimilarityPtr& similarity, ByteArray norms) : Scorer(similarity) {
    this->weight = weight;
    this->termDocs = td;
    this->__termDocs = this->termDocs.get();
    this->norms = norms;
    this->weightValue = weight->getValue();
    this->doc = -1;
    this->docs = Collection<int32_t>::newInstance(123);
    this->__docs = this->docs.get();
    this->freqs = Collection<int32_t>::newInstance(128);
    this->__freqs = this->freqs.get();
    this->pointer = 0;
    this->pointerMax = 0;
    this->scoreCache = Collection<double>::newInstance(SCORE_CACHE_SIZE);

    for (int32_t i = 0; i < SCORE_CACHE_SIZE; ++i) {
        scoreCache[i] = similarity->tf(i) * weightValue;
    }
}

TermScorer::~TermScorer() {
}

inline const Collection<double>& TermScorer::SIM_NORM_DECODER() {
    return Similarity::NORM_TABLE;
}

void TermScorer::score(const CollectorPtr& collector) {
    score(collector, INT_MAX, nextDoc());
}

bool TermScorer::score(const CollectorPtr& collector, int32_t max, int32_t firstDocID) {
    // firstDocID is ignored since nextDoc() sets 'doc'
    auto* __collector = collector.get();
    __collector->setScorer(shared_from_this());
    while (doc < max) { // for docs in window
        __collector->collect(doc);

        if (++pointer >= pointerMax) {
            pointerMax = __termDocs->read(docs, freqs); // refill buffers
            if (pointerMax != 0) {
                pointer = 0;
            } else {
                __termDocs->close(); // close stream
                doc = INT_MAX; // set to sentinel value
                return false;
            }
        }
        doc = __docs->operator[](pointer);
        freq = __freqs->operator[](pointer);
    }
    return true;
}

int32_t TermScorer::docID() {
    return doc;
}

int32_t TermScorer::nextDoc() {
    ++pointer;
    if (pointer >= pointerMax) {
        pointerMax = __termDocs->read(docs, freqs); // refill buffer
        if (pointerMax != 0) {
            pointer = 0;
        } else {
            __termDocs->close(); // close stream
            doc = NO_MORE_DOCS;
            return doc;
        }
    }
    doc = __docs->operator[](pointer);
    freq = __freqs->operator[](pointer);

    return doc;
}

double TermScorer::score() {
    BOOST_ASSERT(doc != -1);
    double raw = freq < SCORE_CACHE_SIZE ? scoreCache[freq] : similarity->tf(freq) * weightValue; // compute tf(f) * weight
    return norms ? raw * SIM_NORM_DECODER()[norms[doc] & 0xff] : raw; // normalize for field
}

int32_t TermScorer::advance(int32_t target) {
    // first scan in cache
    for (++pointer; pointer < pointerMax; ++pointer) {
        if (__docs->operator[](pointer) >= target) {
            doc = __docs->operator[](pointer);
            freq = __freqs->operator[](pointer);
            return doc;
        }
    }

    // not found in cache, seek underlying stream
    bool result = __termDocs->skipTo(target);
    if (result) {
        pointerMax = 1;
        pointer = 0;
        doc = __termDocs->doc();
        __docs->operator[](pointer) = doc;
        freq = __termDocs->freq();
        __freqs->operator[](pointer) = freq;
    } else {
        doc = NO_MORE_DOCS;
    }
    return doc;
}

String TermScorer::toString() {
    return L"term scorer(" + weight->toString() + L")";
}

}
