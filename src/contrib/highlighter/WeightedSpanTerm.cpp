/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "WeightedSpanTerm.h"

namespace Lucene {

WeightedSpanTerm::WeightedSpanTerm(double weight, const String& term, bool positionSensitive) : WeightedTerm(weight, term) {
    this->positionSensitive = positionSensitive;
    this->positionSpans = Collection<PositionSpanPtr>::newInstance();
}

WeightedSpanTerm::~WeightedSpanTerm() {
}

bool WeightedSpanTerm::checkPosition(int32_t position) {
    // There would probably be a slight speed improvement if PositionSpans where kept in some sort of priority queue -
    // that way this method could bail early without checking each PositionSpan.
    for (Collection<PositionSpanPtr>::iterator posSpan = positionSpans.begin(); posSpan != positionSpans.end(); ++posSpan) {
        if (position >= (*posSpan)->start && position <= (*posSpan)->end) {
            return true;
        }
    }
    return false;
}

void WeightedSpanTerm::addPositionSpans(Collection<PositionSpanPtr> positionSpans) {
    this->positionSpans.addAll(positionSpans.begin(), positionSpans.end());
}

bool WeightedSpanTerm::isPositionSensitive() {
    return positionSensitive;
}

void WeightedSpanTerm::setPositionSensitive(bool positionSensitive) {
    this->positionSensitive = positionSensitive;
}

Collection<PositionSpanPtr> WeightedSpanTerm::getPositionSpans() {
    return positionSpans;
}

PositionSpan::PositionSpan(int32_t start, int32_t end) {
    this->start = start;
    this->end = end;
}

PositionSpan::~PositionSpan() {
}

}
