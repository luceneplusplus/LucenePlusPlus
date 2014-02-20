/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WEIGHTEDSPANTERM_H
#define WEIGHTEDSPANTERM_H

#include "WeightedTerm.h"

namespace Lucene {

/// Lightweight class to hold term, weight, and positions used for scoring this term.
class LPPCONTRIBAPI WeightedSpanTerm : public WeightedTerm {
public:
    WeightedSpanTerm(double weight, const String& term, bool positionSensitive = false);
    virtual ~WeightedSpanTerm();

    LUCENE_CLASS(WeightedSpanTerm);

public:
    bool positionSensitive;

protected:
    Collection<PositionSpanPtr> positionSpans;

public:
    /// Checks to see if this term is valid at position.
    /// @param position To check against valid term positions.
    /// @return true if this term is a hit at this position.
    bool checkPosition(int32_t position);

    void addPositionSpans(Collection<PositionSpanPtr> positionSpans);
    bool isPositionSensitive();
    void setPositionSensitive(bool positionSensitive);
    Collection<PositionSpanPtr> getPositionSpans();
};

/// Utility class to store a Span
class LPPCONTRIBAPI PositionSpan : public LuceneObject {
public:
    PositionSpan(int32_t start, int32_t end);
    virtual ~PositionSpan();

    LUCENE_CLASS(PositionSpan);

public:
    int32_t start;
    int32_t end;
};

}

#endif
