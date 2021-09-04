/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PhraseQueue.h"
#include "PhrasePositions.h"

namespace Lucene {

PhraseQueue::PhraseQueue(int32_t size) : PriorityQueue<PhrasePositionsStar>(size) {
}

PhraseQueue::~PhraseQueue() {
}

inline bool PhraseQueue::lessThan(const PhrasePositionsStar& first, const PhrasePositionsStar& second) {
    if (first && second) {
        if (first->doc == second->doc) {
            if (first->position == second->position) {
                // same doc and pp.position, so decide by actual term positions.
                // rely on: pp.position == tp.position - offset.
                return first->offset < second->offset;
            } else {
                return first->position < second->position;
            }
        } else {
            return first->doc < second->doc;
        }
    }
    return first ? false : true;
}

}
