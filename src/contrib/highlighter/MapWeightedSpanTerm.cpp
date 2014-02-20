/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "MapWeightedSpanTerm.h"

namespace Lucene {

MapWeightedSpanTerm::MapWeightedSpanTerm() {
    map = MapStringWeightedSpanTerm::newInstance();
}

MapWeightedSpanTerm::~MapWeightedSpanTerm() {
}

MapStringWeightedSpanTerm::iterator MapWeightedSpanTerm::begin() {
    return map.begin();
}

MapStringWeightedSpanTerm::iterator MapWeightedSpanTerm::end() {
    return map.end();
}

void MapWeightedSpanTerm::put(const String& key, const WeightedSpanTermPtr& val) {
    return map.put(key, val);
}

WeightedSpanTermPtr MapWeightedSpanTerm::get(const String& key) const {
    return map.get(key);
}

void MapWeightedSpanTerm::clear() {
    map.clear();
}

}
