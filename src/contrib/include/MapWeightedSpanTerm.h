/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MAPWEIGHTEDSPANTERM_H
#define MAPWEIGHTEDSPANTERM_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Utility class that encapsulates a StringWeightedSpanTerm map that can be overridden.
class LPPCONTRIBAPI MapWeightedSpanTerm : public LuceneObject {
public:
    MapWeightedSpanTerm();
    virtual ~MapWeightedSpanTerm();

    LUCENE_CLASS(MapWeightedSpanTerm);

protected:
    MapStringWeightedSpanTerm map;

public:
    virtual MapStringWeightedSpanTerm::iterator begin();
    virtual MapStringWeightedSpanTerm::iterator end();
    virtual void put(const String& key, const WeightedSpanTermPtr& val);
    virtual WeightedSpanTermPtr get(const String& key) const;
    virtual void clear();
};

}

#endif
