/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilteredDocIdSet.h"

namespace Lucene
{
    FilteredDocIdSet::FilteredDocIdSet(DocIdSetPtr innerSet)
    {
        this->innerSet = innerSet;
    }
    
    FilteredDocIdSet::~FilteredDocIdSet()
    {
    }
    
    bool FilteredDocIdSet::isCacheable()
    {
        return innerSet->isCacheable();
    }
    
    DocIdSetIteratorPtr FilteredDocIdSet::iterator()
    {
        return newLucene<DefaultFilteredDocIdSetIterator>(shared_from_this(), innerSet->iterator());
    }
    
    DefaultFilteredDocIdSetIterator::DefaultFilteredDocIdSetIterator(FilteredDocIdSetPtr filtered, DocIdSetIteratorPtr innerIter) : FilteredDocIdSetIterator(innerIter)
    {
        this->filtered = filtered;
    }
    
    DefaultFilteredDocIdSetIterator::~DefaultFilteredDocIdSetIterator()
    {
    }
    
    bool DefaultFilteredDocIdSetIterator::match(int32_t docid)
    {
        return filtered->match(docid);
    }
}
