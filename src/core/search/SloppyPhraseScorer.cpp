/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SloppyPhraseScorer.h"
#include "PhrasePositions.h"
#include "PhraseQueue.h"
#include "Similarity.h"

namespace Lucene {

struct __luceneEquals {
    inline bool operator()(const PhrasePositions* __first, const PhrasePositions* __second) const {
        return __first ? (__second && __first == __second)  : (!__first && !__second);
    }
};

typedef HashMap< PhrasePositions*, LuceneObjectPtr, luceneHash<PhrasePositions*>, __luceneEquals > __MapPhrasePositionsLuceneObject;

SloppyPhraseScorer::SloppyPhraseScorer(const WeightPtr& weight, Collection<TermPositionsPtr> tps, Collection<int32_t> offsets, const SimilarityPtr& similarity, int32_t slop, ByteArray norms) : PhraseScorer(weight, tps, offsets, similarity, norms) {
    this->slop = slop;
    this->checkedRepeats = false;
}

SloppyPhraseScorer::~SloppyPhraseScorer() {
}

double SloppyPhraseScorer::phraseFreq() {
    int32_t end = initPhrasePositions();

    double freq = 0.0;
    bool done = (end < 0);
    while (!done) {
        auto* __pp = pq->pop();
        int32_t start = __pp->position;
        int32_t next = pq->top()->position;

        bool tpsDiffer = true;
        for (int32_t pos = start; pos <= next || !tpsDiffer; pos = __pp->position) {
            if (pos<=next && tpsDiffer) {
                start = pos;    // advance pp to min window
            }
            if (!__pp->nextPosition()) {
                done = true; // ran out of a term - done
                break;
            }

            PhrasePositions* __pp2 = nullptr;
            tpsDiffer = (!__pp->repeats || !(__pp2 = termPositionsDiffer(__pp)));
            if (__pp2 && __pp2 != __pp) {
                __pp = flip(__pp, __pp2);    // flip pp to pp2
            }
        }

        int32_t matchLength = end - start;
        if (matchLength <= slop) {
            freq += getSimilarity()->sloppyFreq(matchLength);    // score match
        }

        if (__pp->position > end) {
            end = __pp->position;
        }
        pq->add(__pp); // restore pq
    }

    return freq;
}

PhrasePositions* SloppyPhraseScorer::flip(PhrasePositions* __pp, PhrasePositions* __pp2) {
    int32_t n = 0;
    PhrasePositions* __pp3;
    // pop until finding pp2
    while ((__pp3 = pq->pop()) != __pp2) {
        tmpPos[n++] = __pp3;
    }
    // insert back all but pp2
    for (n--; n >= 0; --n) {
        pq->addOverflow(tmpPos[n]);
    }
    // insert pp back
    pq->add(__pp);
    return __pp2;
}

int32_t SloppyPhraseScorer::initPhrasePositions() {
    int32_t end = 0;

    // no repeats at all (most common case is also the simplest one)
    if (checkedRepeats && !repeats) {
        // build queue from list
        pq->clear();
        for (auto* __pp = __first; __pp; __pp = __pp->__next) {
            __pp->firstPosition();
            if (__pp->position > end) {
                end = __pp->position;
            }
            pq->add(__pp); // build pq from list
        }
        return end;
    }

    // position the pp's
    for (PhrasePositions* __pp = __first; __pp; __pp = __pp->__next) {
        __pp->firstPosition();
    }

    // one time initialization for this scorer
    if (!checkedRepeats) {
        checkedRepeats = true;
        // check for repeats
        __MapPhrasePositionsLuceneObject m;
        for (auto* __pp = __first; __pp; __pp = __pp->__next) {
            int32_t tpPos = __pp->position + __pp->offset;
            for (auto* __pp2 = __pp->__next; __pp2; __pp2 = __pp2->__next) {
                int32_t tpPos2 = __pp2->position + __pp2->offset;
                if (tpPos2 == tpPos) {
                    if (!m) {
                        m = __MapPhrasePositionsLuceneObject::newInstance();
                    }
                    __pp->repeats = true;
                    __pp2->repeats = true;
                    m.put(__pp, LuceneObjectPtr());
                    m.put(__pp2, LuceneObjectPtr());
                }
            }
        }
        if (m) {
            repeats = Collection<PhrasePositions*>::newInstance();
            for (__MapPhrasePositionsLuceneObject::iterator key = m.begin(); key != m.end(); ++key) {
                repeats.add(key->first);
            }
        }
    }

    // with repeats must advance some repeating pp's so they all start with differing tp's
    if (repeats) {
        for (Collection<PhrasePositions*>::iterator pp = repeats.begin(); pp != repeats.end(); ++pp) {
            PhrasePositions* pp2 = nullptr;
            while ((pp2 = termPositionsDiffer(*pp))) {
                if (!pp2->nextPosition()) { // out of pps that do not differ, advance the pp with higher offset
                    return -1;    // ran out of a term - done
                }
            }
        }
    }

    // build queue from list
    pq->clear();
    for (auto* __pp = __first; __pp; __pp = __pp->__next) {
        if (__pp->position > end) {
            end = __pp->position;
        }
        pq->add(__pp); // build pq from list
    }

    if (repeats) {
        tmpPos = Collection<PhrasePositions*>::newInstance(pq->size());
    }

    return end;
}

PhrasePositions* SloppyPhraseScorer::termPositionsDiffer(PhrasePositions* __pp) {
    // Efficiency note: a more efficient implementation could keep a map between repeating pp's, so that if
    // pp1a, pp1b, pp1c are repeats term1, and pp2a, pp2b are repeats of term2, pp2a would only be checked
    // against pp2b but not against pp1a, pp1b, pp1c.  However this would complicate code, for a rather rare
    // case, so choice is to compromise here.
    int32_t tpPos = __pp->position + __pp->offset;
    for (Collection<PhrasePositions*>::iterator pp2 = repeats.begin(); pp2 != repeats.end(); ++pp2) {
        if (*pp2 == __pp) {
            continue;
        }
        int32_t tpPos2 = (*pp2)->position + (*pp2)->offset;
        if (tpPos2 == tpPos) {
            return __pp->offset > (*pp2)->offset ? __pp : *pp2;    // do not differ: return the one with higher offset.
        }
    }
    return nullptr;
}

}
