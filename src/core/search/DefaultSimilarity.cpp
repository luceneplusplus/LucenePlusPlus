/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "DefaultSimilarity.h"
#include "FieldInvertState.h"

namespace Lucene {

DefaultSimilarity::DefaultSimilarity() {
    discountOverlaps = false;
}

DefaultSimilarity::~DefaultSimilarity() {
}

double DefaultSimilarity::computeNorm(const String& fieldName, const FieldInvertStatePtr& state) {
    int32_t numTerms;
    if (discountOverlaps) {
        numTerms = state->getLength() - state->getNumOverlap();
    } else {
        numTerms = state->getLength();
    }
    return (state->getBoost() * lengthNorm(fieldName, numTerms));
}

inline double DefaultSimilarity::lengthNorm(const String& fieldName, int32_t numTokens) {
    return (double)(1.0 / std::sqrt((double)numTokens));
}

inline double DefaultSimilarity::queryNorm(double sumOfSquaredWeights) {
    return (double)(1.0 / std::sqrt(sumOfSquaredWeights));
}

inline double DefaultSimilarity::tf(double freq) {
    return (double)std::sqrt(freq);
}

inline double DefaultSimilarity::sloppyFreq(int32_t distance) {
    return (1.0 / (double)(distance + 1));
}

inline double DefaultSimilarity::idf(int32_t docFreq, int32_t numDocs) {
    return (double)(std::log((double)numDocs / (double)(docFreq + 1)) + 1.0);
}

inline double DefaultSimilarity::coord(int32_t overlap, int32_t maxOverlap) {
    return (double)overlap / (double)maxOverlap;
}

inline void DefaultSimilarity::setDiscountOverlaps(bool v) {
    discountOverlaps = v;
}

inline bool DefaultSimilarity::getDiscountOverlaps() {
    return discountOverlaps;
}

}
