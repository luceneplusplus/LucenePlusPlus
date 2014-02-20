/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MOCKFILTER_H
#define MOCKFILTER_H

#include "test_lucene.h"
#include "Filter.h"

namespace Lucene {

class MockFilter : public Filter {
public:
    MockFilter();
    virtual ~MockFilter();

    LUCENE_CLASS(MockFilter);

protected:
    bool _wasCalled;

public:
    virtual DocIdSetPtr getDocIdSet(const IndexReaderPtr& reader);

    void clear();
    bool wasCalled();
};

}

#endif
