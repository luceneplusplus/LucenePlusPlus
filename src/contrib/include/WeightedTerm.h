/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WEIGHTEDTERM_H
#define WEIGHTEDTERM_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Lightweight class to hold term and a weight value used for scoring this term
class LPPCONTRIBAPI WeightedTerm : public LuceneObject {
public:
    WeightedTerm(double weight, const String& term);
    virtual ~WeightedTerm();

    LUCENE_CLASS(WeightedTerm);

public:
    double weight; // multiplier
    String term; // stemmed form

public:
    /// @return the term value (stemmed)
    String getTerm();

    /// @return the weight associated with this term
    double getWeight();

    /// @param term the term value (stemmed)
    void setTerm(const String& term);

    /// @param weight the weight associated with this term
    void setWeight(double weight);
};

}

#endif
