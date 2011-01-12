/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanQuery.h"
#include "SpanWeight.h"

namespace Lucene
{
    SpanQuery::~SpanQuery()
    {
    }
    
    WeightPtr SpanQuery::createWeight(SearcherPtr searcher)
    {
        return newLucene<SpanWeight>(shared_from_this(), searcher);
    }
}
