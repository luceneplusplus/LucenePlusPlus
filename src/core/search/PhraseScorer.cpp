/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PhraseScorer.h"
#include "PhrasePositions.h"
#include "PhraseQueue.h"
#include "Weight.h"
#include "Similarity.h"

namespace Lucene {

PhraseScorer::PhraseScorer(const WeightPtr& weight, Collection<TermPositionsPtr> tps, Collection<int32_t> offsets, const SimilarityPtr& similarity, ByteArray norms) : Scorer(similarity) {
    this->firstTime = true;
    this->more = true;
    this->freq = 0.0;

    this->norms = norms;
    this->weight = weight;
    this->value = weight->getValue();

    // convert tps to a list of phrase positions.
    // Note: phrase-position differs from term-position in that its position reflects the phrase offset: pp.pos = tp.pos - offset.
    // This allows to easily identify a matching (exact) phrase when all PhrasePositions have exactly the same position.
    for (int32_t i = 0; i < tps.size(); ++i) {
        PhrasePositionsPtr pp(newLucene<PhrasePositions>(tps[i], offsets[i]));
        auto* __pp = pp.get();
        if (__last) { // add next to end of list
            __last->__next = __pp;
        } else {
            __first = __pp;
        }
        __last = __pp;
        _holds.emplace_back(pp);
    }

    pq = newLucene<PhraseQueue>(tps.size()); // construct empty pq
    __first->doc = -1;
}

PhraseScorer::~PhraseScorer() {
}

int32_t PhraseScorer::docID() {
    return __first->doc;
}

int32_t PhraseScorer::nextDoc() {
    if (firstTime) {
        init();
        firstTime = false;
    } else if (more) {
        more = __last->next();    // trigger further scanning
    }
    if (!doNext()) {
        __first->doc = NO_MORE_DOCS;
    }
    return __first->doc;
}

bool PhraseScorer::doNext() {
    while (more) {
        while (more && __first->doc < __last->doc) { // find doc with all the terms
            more = __first->skipTo(__last->doc); // skip first upto last and move it to the end
            firstToLast();
        }

        if (more) {
            // found a doc with all of the terms
            freq = phraseFreq(); // check for phrase
            if (freq == 0.0) { // no match
                more = __last->next();    // trigger further scanning
            } else {
                return true;
            }
        }
    }
    return false; // no more matches
}

double PhraseScorer::score() {
    double raw = getSimilarity()->tf(freq) * value; // raw score
    return !norms ? raw : raw * Similarity::decodeNorm(norms[__first->doc]); // normalize
}

int32_t PhraseScorer::advance(int32_t target) {
    firstTime = false;
    for (auto* __pp = __first; more && __pp; __pp = __pp->__next) {
        more = __pp->skipTo(target);
    }
    if (more) {
        sort();    // re-sort
    }
    if (!doNext()) {
        __first->doc = NO_MORE_DOCS;
    }
    return __first->doc;
}

double PhraseScorer::currentFreq() {
    return freq;
}

void PhraseScorer::init() {
    for (auto* __pp = __first; more && __pp; __pp = __pp->__next) {
        more = __pp->next();
    }
    if (more) {
        sort();
    }
}

void PhraseScorer::sort() {
    pq->clear();
    for (auto* __pp = __first; more && __pp; __pp = __pp->__next) {
        pq->add(__pp);
    }
    pqToList();
}

void PhraseScorer::pqToList() {
    __last = nullptr;
    __first = nullptr;
    while (pq->top()) {
        auto* __pp = pq->pop();
        if (__last) { // add next to end of list
            __last->__next = __pp;
        } else {
            __first = __pp;
        }
        __last = __pp;
        __pp->__next = nullptr;
    }
}

void PhraseScorer::firstToLast() {
    __last->__next = __first; // move first to end of list
    __last = __first;
    __first = __first->__next;
    __last->__next = nullptr;
}

String PhraseScorer::toString() {
    return L"scorer(" + weight->toString() + L")";
}

}
