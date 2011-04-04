/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FILTEREDDOCIDSET_H
#define _FILTEREDDOCIDSET_H

#include "FilteredDocIdSetIterator.h"

namespace Lucene
{
    /// Implementation of the contract to build a DocIdSetIterator.
    class DefaultFilteredDocIdSetIterator : public FilteredDocIdSetIterator
    {
    public:
        DefaultFilteredDocIdSetIterator(FilteredDocIdSetPtr filtered, DocIdSetIteratorPtr innerIter);
        virtual ~DefaultFilteredDocIdSetIterator();
    
        LUCENE_CLASS(DefaultFilteredDocIdSetIterator);
    
    protected:
        FilteredDocIdSetPtr filtered;
    
    protected:
        virtual bool match(int32_t docid);
    };
}

#endif
