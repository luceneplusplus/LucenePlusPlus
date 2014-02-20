/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "WeightedTerm.h"

namespace Lucene {

WeightedTerm::WeightedTerm(double weight, const String& term) {
    this->weight = weight;
    this->term = term;
}

WeightedTerm::~WeightedTerm() {
}

String WeightedTerm::getTerm() {
    return term;
}

double WeightedTerm::getWeight() {
    return weight;
}

void WeightedTerm::setTerm(const String& term) {
    this->term = term;
}

void WeightedTerm::setWeight(double weight) {
    this->weight = weight;
}

}
