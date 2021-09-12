/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ExactPhraseScorer.h"
#include "PhrasePositions.h"
#include "PhraseQueue.h"

namespace Lucene {

ExactPhraseScorer::ExactPhraseScorer(const WeightPtr& weight, Collection<TermPositionsPtr> tps, Collection<int32_t> offsets, const SimilarityPtr& similarity, ByteArray norms) : PhraseScorer(weight, tps, offsets, similarity, norms) {
}

ExactPhraseScorer::~ExactPhraseScorer() {
}

double ExactPhraseScorer::phraseFreq() {
    // sort list with pq
    pq->clear();
    for (auto* __pp = __first; more && __pp; __pp = __pp->__next) {
        __pp->firstPosition();
        pq->add(__pp);
    }
    pqToList(); // rebuild list from pq

    // For counting how many times the exact phrase is found in current document, just count how many
    // times all PhrasePosition's have exactly the same position.
    int32_t freq = 0;
    do {
        while (__first->position < __last->position) { // scan forward in first
            do {
                if (!__first->nextPosition()) {
                    return freq;
                }
            } while (__first->position < __last->position);
            firstToLast();
        }
        ++freq; // all equal: a match
    } while (__last->nextPosition());

    return freq;
}

}
